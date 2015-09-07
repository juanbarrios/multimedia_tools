/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

#ifndef NO_OPENCV
struct ParamsSeg {
	DB *db;
	Segmentator **sp;
	SaveSegmentation *saver_seg;
	const char *segAlias;
};
static void printSegmentationStats(const char *nameFile, const char *segAlias,
		struct Segmentation *seg) {
	int64_t cont_frames = 0;
	double cont_seconds = 0;
	contFramesAndSecondsInSegmentation(seg, &cont_frames, &cont_seconds);
	char *stNumSegments = my_newString_int(seg->num_segments);
	char *stNumFrames = my_newString_int(cont_frames);
	char *stNumSeconds = my_newString_hhmmss(cont_seconds);
	char *stAvgFrames = my_newString_doubleDec(
			cont_frames / (double) seg->num_segments, 1);
	char *stAvgSeconds = my_newString_doubleDec(
			cont_seconds / (double) seg->num_segments, 2);
	my_log_info(
			"%s %s: length=%s frms=%s segments=%s avg.frms/segment=%s avg.secs/segment=%s\n",
			nameFile, segAlias, stNumSeconds, stNumFrames, stNumSegments,
			stAvgFrames, stAvgSeconds);
	MY_FREE_MULTI(stNumSegments, stNumFrames, stNumSeconds, stAvgFrames,
			stAvgSeconds);
}
static void seg_segmentVideo(int64_t currentProcess, void* state,
		int64_t numThread) {
	struct ParamsSeg *params = state;
	FileDB *fdb = params->db->filesDb[currentProcess];
	if (!fdb->isVideo && !fdb->isAudio)
		return;
	Segmentator *sp = params->sp[numThread];
	struct Segmentation *seg = segmentVideoFile(sp, fdb);
	printSegmentationStats(fdb->id, params->segAlias, seg);
	saveSegmentation(params->saver_seg, fdb->id, seg);
	releaseSegmentation(seg);
}
static void process_db(DB *db, const char *segmentation, const char *segAlias) {
	my_io_createDir(db->pathSegmentations, false);
	char *path = my_newString_format("%s/%s", db->pathSegmentations, segAlias);
	struct ParamsSeg params = { 0 };
	params.db = db;
	params.saver_seg = newSaveSegmentation(path, segmentation);
	params.segAlias = segAlias;
	params.sp = MY_MALLOC(NUM_CORES, Segmentator*);
	for (int64_t i = 0; i < NUM_CORES; ++i)
		params.sp[i] = findSegmentator(segmentation);
	free(path);
	my_parallel_incremental(db->numFilesDb, &params, seg_segmentVideo, segAlias,
			NUM_CORES);
	releaseSaveSegmentation(params.saver_seg);
	for (int64_t i = 0; i < NUM_CORES; ++i)
		releaseSegmentator(params.sp[i]);
	free(params.sp);
}
#endif

void db_compute_segmentation(CmdParams *cmd_params, char *argOption) {
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"    -db path_DB                       Mandatory. One or More. Options: %s\n",
				getDbLoadOptions());
		my_log_info(
				"    [-seg segmentator [-alias txt]]+  Mandatory. One or More.\n");
		my_log_info(
				"    -show                                  Shows defined segmentations.\n");
		return;
	}
	MyVectorString *dbs = my_vectorString_new();
	MyVectorString *segmentations = my_vectorString_new();
	MyVectorString *aliases = my_vectorString_new();
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-db")) {
			const char *db = nextParam(cmd_params);
			my_vectorStringConst_add(dbs, db);
		} else if (isNextParam(cmd_params, "-seg")) {
			const char *seg = nextParam(cmd_params);
			my_vectorStringConst_add(segmentations, seg);
			if (isNextParam(cmd_params, "-alias")) {
				my_vectorStringConst_add(aliases, nextParam(cmd_params));
			} else {
				my_vectorStringConst_add(aliases, seg);
			}
		} else if (isNextParam(cmd_params, "-show")) {
			print_segmentators();
			return;
		} else {
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
		}
	}
	if (my_vectorString_size(dbs) == 0)
		my_log_error("-db is mandatory\n");
	if (my_vectorString_size(segmentations) == 0)
		my_log_error("-seg is mandatory\n");
	for (int64_t i = 0; i < my_vectorString_size(dbs); ++i) {
		const char *nameDB = my_vectorString_get(dbs, i);
		DB *db = loadDB(nameDB, true, true);
		set_logfile(db->pathLogs, cmd_params, "");
		for (int64_t j = 0; j < my_vectorString_size(segmentations); ++j) {
			const char *segmentation = my_vectorString_get(segmentations, j);
			const char *alias = my_vectorString_get(aliases, j);
			char *dirname = my_newString_string(alias);
			my_io_removeInvalidChars(dirname);
			process_db(db, segmentation, dirname);
			free(dirname);
		}
		close_logfile();
		releaseDB(db);
	}
#endif
}

