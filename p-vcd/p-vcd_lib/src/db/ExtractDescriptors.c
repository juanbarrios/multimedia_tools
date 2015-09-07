/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

#ifndef NO_OPENCV
struct Params {
	DB *db;
	bool useSingleFile;
	bool withTxt, existingOverwrite, existingSkip;
	int64_t numThreadsByFile;
	MyVectorObj *processes;
};
struct Process {
	const char *extractor, *alias, *name_segmentation;
	char *newDirname, *path_descriptorOut;
	struct Params *params;
	Extractor **exs;
	SaveDescriptors *saver;
	LoadSegmentation *loader_seg;
};
struct FileDescriptors {
	int64_t numDescriptors;
	void **descriptors;
	const struct Segmentation *seg;
	DescriptorType td;
};

static struct FileDescriptors extractDescriptors(FileDB *fdb,
		struct Process *proc, int64_t numThread) {
	const struct Segmentation *seg = loadSegmentationFileDB(proc->loader_seg,
			fdb);
	my_assert_notNull("seg", seg);
	void **descriptors = MY_MALLOC(seg->num_segments, void*);
	if (proc->params->numThreadsByFile > 1) {
		extractPersistentDescriptors_threadedSegments(proc->exs,
				proc->params->numThreadsByFile, fdb, seg, 0, seg->num_segments,
				descriptors);
	} else {
		extractPersistentDescriptors_seg(proc->exs[numThread], fdb, seg, 0,
				seg->num_segments, descriptors);
	}
	struct FileDescriptors fd = { 0 };
	fd.numDescriptors = seg->num_segments;
	fd.descriptors = descriptors;
	fd.seg = seg;
	fd.td = getDescriptorType(proc->exs[0]);
	return fd;
}
static void ext_process_file(FileDB *fdb, struct Process *proc,
		int64_t numThread) {
	bool alreadyExists = saveDescriptors_existsFile(proc->saver, fdb->id);
	if (alreadyExists && proc->params->existingSkip)
		return;
	if (!my_io_existsDir(proc->path_descriptorOut)) {
		my_io_createDir(proc->params->db->pathDescriptors, false);
		my_io_createDir(proc->path_descriptorOut, false);
	}
	struct FileDescriptors fd = extractDescriptors(fdb, proc, numThread);
	saveDescriptors(proc->saver, fdb->id, fd.numDescriptors, fd.descriptors,
			fd.seg);
	releasePersistentDescriptors(fd.descriptors, fd.numDescriptors, fd.td);
	MY_FREE(fd.descriptors);
}
static void ext_processFile(int64_t currentProcess, void* state,
		int64_t numThread) {
	struct Params *params = (struct Params *) state;
	FileDB *fdb = params->db->filesDb[currentProcess];
	for (int64_t i = 0; i < my_vectorObj_size(params->processes); ++i) {
		struct Process *proc = my_vectorObj_get(params->processes, i);
		MyTimer *timer = my_timer_new();
		ext_process_file(fdb, proc, numThread);
		double secs = my_timer_getSeconds(timer);
		if (secs >= 5) {
			my_log_info("extractTime\t%s\t%s\t%1.1lf\tseconds\n", fdb->id,
					proc->newDirname, secs);
		}
		my_timer_release(timer);
	}
}
static void initialize_processes(struct Params *params, int64_t numThreads) {
	for (int64_t i = 0; i < my_vectorObj_size(params->processes); ++i) {
		struct Process *proc = my_vectorObj_get(params->processes, i);
		proc->loader_seg = newLoadSegmentation(params->db,
					proc->name_segmentation);
		if (proc->alias != NULL)
			proc->newDirname = my_newString_string(proc->alias);
		else if (proc->name_segmentation != NULL)
			proc->newDirname = my_newString_format("%s-%s", proc->extractor,
					proc->name_segmentation);
		else
			proc->newDirname = my_newString_string(proc->extractor);
		my_io_removeInvalidChars(proc->newDirname);
		proc->path_descriptorOut = my_newString_format("%s/%s",
				params->db->pathDescriptors, proc->newDirname);
		if (my_io_existsDir(proc->path_descriptorOut)
				&& !params->existingOverwrite && !params->existingSkip)
			my_log_error(
					"directory %s already exists (use -continue or -overwrite)\n",
					proc->path_descriptorOut);
		proc->exs = MY_MALLOC_NOINIT(numThreads, Extractor*);
		for (int64_t j = 0; j < numThreads; ++j)
			proc->exs[j] = getExtractor(proc->extractor);
		proc->params = params;
		DescriptorType td = getDescriptorType(proc->exs[0]);
		proc->saver = newSaveDescriptors(proc->path_descriptorOut, td, true,
				params->withTxt, params->useSingleFile, proc->extractor,
				proc->name_segmentation);
	}
}
static void finalize_processes(struct Params *params, int64_t numThreads) {
	for (int64_t i = 0; i < my_vectorObj_size(params->processes); ++i) {
		struct Process *proc = my_vectorObj_get(params->processes, i);
		releaseSaveDescriptors(proc->saver);
	}
	for (int64_t i = 0; i < my_vectorObj_size(params->processes); ++i) {
		struct Process *proc = my_vectorObj_get(params->processes, i);
		for (int64_t j = 0; j < numThreads; ++j)
			releaseExtractor(proc->exs[j]);
		MY_FREE(proc->exs);
		MY_FREE(proc->path_descriptorOut);
		MY_FREE(proc);
	}
}
#endif
void db_compute_descriptors(CmdParams *cmd_params, char *argOption) {
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"    -db path_DB                      Mandatory. Options: %s\n",
				getDbLoadOptions());
		my_log_info(
				"    [-seg segmentation]              Optional for images. Mandatory for videos. One or more.\n");
		my_log_info(
				"    -desc extractor  [-alias txt]    Mandatory. One or more. Alias is optional.\n");
		my_log_info(
				"    [-singleFile | -multiFile]       Optional. Default=Auto. -singleFile when db contains only images, -multiFile otherwise.\n");
		my_log_info("    [-withTxt]                       Optional.\n");
		my_log_info("    [-continue | -overwrite]         Optional.\n");
		my_log_info(
				"    [-useThreadsInVideo | noThreadsInVideo]  Optional. use or not multi-threading on the same video.\n");
		my_log_info(
				"    -show                            Shows defined extractors\n");
		return;
	}
	const char *dbName = NULL, *last_segmentation = NULL;
	struct Params params = { 0 };
	params.processes = my_vectorObj_new();
	bool autoSingleFile = true;
	bool autoThreadsInVideo = true, useThreadsInVideo = false;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-db")) {
			dbName = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-seg")) {
			last_segmentation = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-desc")) {
			struct Process *proc = MY_MALLOC(1, struct Process);
			proc->extractor = nextParam(cmd_params);
			proc->name_segmentation = last_segmentation;
			if (isNextParam(cmd_params, "-alias"))
				proc->alias = nextParam(cmd_params);
			my_vectorObj_add(params.processes, proc);
		} else if (isNextParam(cmd_params, "-withTxt")) {
			params.withTxt = 1;
		} else if (isNextParam(cmd_params, "-multiFile")) {
			autoSingleFile = false;
			params.useSingleFile = false;
		} else if (isNextParam(cmd_params, "-singleFile")) {
			autoSingleFile = false;
			params.useSingleFile = true;
		} else if (isNextParam(cmd_params, "-continue")) {
			params.existingSkip = true;
		} else if (isNextParam(cmd_params, "-overwrite")) {
			params.existingOverwrite = true;
		} else if (isNextParam(cmd_params, "-useThreadsInVideo")) {
			autoThreadsInVideo = false;
			useThreadsInVideo = true;
		} else if (isNextParam(cmd_params, "-noThreadsInVideo")) {
			autoThreadsInVideo = false;
			useThreadsInVideo = false;
		} else if (isNextParam(cmd_params, "-show")) {
			print_extractors();
			return;
		} else
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
	}
	my_assert_notNull("-db", dbName);
	my_assert_greaterEqual_int("desc", my_vectorObj_size(params.processes), 1);
	DB *db = loadDB(dbName, false, true);
	if (autoSingleFile)
		params.useSingleFile = (db->isImageDb && !db->isVideoDb
				&& !db->isAudioDb);
	params.db = db;
	int64_t numParallelFiles = NUM_CORES;
	params.numThreadsByFile = 1;
	if (db->isVideoDb && !db->isAudioDb && !db->isImageDb
			&& ((autoThreadsInVideo && db->numFilesDb < 2 * NUM_CORES)
					|| useThreadsInVideo)) {
		numParallelFiles = 1;
		params.numThreadsByFile = NUM_CORES;
	}
	initialize_processes(&params,
			MAX(numParallelFiles, params.numThreadsByFile));
	set_logfile(db->pathLogs, cmd_params, "");
	if (db->numFilesDb > 10000 && !params.useSingleFile)
		my_log_info("too many files, you should use -singleFile\n");
	my_parallel_incremental(db->numFilesDb, &params, ext_processFile, "extract",
			numParallelFiles);
	finalize_processes(&params, numParallelFiles);
	close_logfile();
#endif
}
