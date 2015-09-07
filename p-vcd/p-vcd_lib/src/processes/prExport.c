/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

#ifndef NO_OPENCV
struct State_Export {
	bool isFormatRaw, isFormatText, isFormatDataset;
	double sampleFractionGlobal, sampleFractionFiles, sampleFractionSegments,
			sampleFractionPerFrame;
	char *outFilename, *nameExtractor, *fileFormat;
	int64_t numThreads;
	Extractor **exs;
};

static void initState(struct State_Export *es) {
	MyTokenizer *tk = my_tokenizer_new(es->fileFormat, '+');
	const char *s;
	while ((s = my_tokenizer_nextToken(tk)) != NULL) {
		if (my_string_equals_ignorecase(s, "RAW")) {
			es->isFormatRaw = true;
		} else if (my_string_equals_ignorecase(s, "TEXT")) {
			es->isFormatText = true;
		} else if (my_string_equals_ignorecase(s, "DATASET")) {
			es->isFormatDataset = true;
		} else {
			my_log_error("unknown format '%s'\n", s);
		}
	}
	if (!es->isFormatRaw && !es->isFormatText && !es->isFormatDataset)
		my_log_error("no output format\n");
	if (es->nameExtractor != NULL) {
		es->numThreads = NUM_CORES;
		es->exs = MY_MALLOC(es->numThreads, Extractor*);
		for (int64_t i = 0; i < es->numThreads; ++i)
			es->exs[i] = getExtractor(es->nameExtractor);
	}
}

static void export_new(const char *procCode, const char *segParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct State_Export *es = MY_MALLOC(1, struct State_Export);
	es->sampleFractionGlobal = my_tokenizer_nextFraction(tk);
	es->sampleFractionFiles = my_tokenizer_nextFraction(tk);
	es->sampleFractionSegments = my_tokenizer_nextFraction(tk);
	es->sampleFractionPerFrame = my_tokenizer_nextFraction(tk);
	es->outFilename = my_tokenizer_nextToken_newString(tk);
	es->fileFormat = my_tokenizer_nextToken_newString(tk);
	if (my_string_equals(procCode, "EXTRACTEXPORT"))
		es->nameExtractor = my_tokenizer_nextToken_newString(tk);
	my_tokenizer_release(tk);
	initState(es);
	*out_state = es;
}
static void exportFile_new(const char *procCode, const char *procParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(procParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct State_Export *es = MY_MALLOC(1, struct State_Export);
	es->sampleFractionSegments = my_tokenizer_nextFraction(tk);
	es->sampleFractionPerFrame = my_tokenizer_nextFraction(tk);
	es->outFilename = my_tokenizer_nextToken_newString(tk);
	es->fileFormat = my_tokenizer_nextToken_newString(tk);
	if (my_string_equals(procCode, "EXTRACTEXPORTFILES"))
		es->nameExtractor = my_tokenizer_nextToken_newString(tk);
	my_tokenizer_release(tk);
	initState(es);
	*out_state = es;
}
static char *get_filename_raw(FileDB *fdb, MknnDataset *dataset,
		struct State_Export *es) {
	MknnDomain *dom = mknn_dataset_getDomain(dataset);
	int64_t cols = mknn_domain_vector_getNumDimensions(dom);
	int64_t rows = mknn_dataset_getNumObjects(dataset);
	const char *dtype = mknn_datatype_toString(
			mknn_domain_vector_getDimensionDataType(dom));
	if (fdb == NULL) {
		if (my_string_endsWith_ignorecase(es->outFilename, ".raw"))
			return my_newString_string(es->outFilename);
		else
			return my_newString_format("%s.%"PRIi64"x%"PRIi64"x%s.raw",
					es->outFilename, rows, cols, dtype);
	} else {
		return my_newString_format("%s/%s.%"PRIi64"x%"PRIi64"x%s.raw",
				es->outFilename, fdb->id, rows, cols, dtype);
	}
}
static char *get_filename_txt(FileDB *fdb, struct State_Export *es) {
	if (fdb == NULL) {
		if (my_string_endsWith_ignorecase(es->outFilename, ".txt"))
			return my_newString_string(es->outFilename);
		else
			return my_newString_format("%s.txt", es->outFilename);
	} else {
		return my_newString_format("%s/%s.txt", es->outFilename, fdb->id);
	}
}
static char *get_filename_dataset(FileDB *fdb, struct State_Export *es) {
	if (fdb == NULL) {
		if (my_string_endsWith_ignorecase(es->outFilename, ".dataset"))
			return my_newString_string(es->outFilename);
		else
			return my_newString_format("%s.dataset", es->outFilename);
	} else {
		return my_newString_format("%s/%s.dataset", es->outFilename, fdb->id);
	}
}
static void savePrintDataset(MknnDataset *dataset, FileDB *fdb,
		struct State_Export *es) {
	if (fdb != NULL)
		my_io_createDir(es->outFilename, false);
	if (es->isFormatRaw) {
		char *fname = get_filename_raw(fdb, dataset, es);
		mknn_dataset_printObjectsRawFile(dataset, fname);
		free(fname);
	}
	if (es->isFormatDataset) {
		char *fname = get_filename_dataset(fdb, es);
		mknn_dataset_save(dataset, fname);
		free(fname);
	}
	if (es->isFormatText) {
		char *fname = get_filename_txt(fdb, es);
		mknn_dataset_printObjectsTextFile(dataset, fname);
		free(fname);
	}
}
static void export_process_desc(LoadDescriptors *desloader, void *state) {
	struct State_Export *es = state;
	MknnDataset *dataset = getAlreadyComputedDescriptorsSample(desloader,
			es->sampleFractionGlobal, es->sampleFractionFiles,
			es->sampleFractionSegments, es->sampleFractionPerFrame);
	savePrintDataset(dataset, NULL, es);
	mknn_dataset_release(dataset);
}
static void extractExport_process_segment(LoadSegmentation *segloader,
		void *state) {
	struct State_Export *es = state;
	MknnDataset *dataset = computeDescriptorsSample(segloader, es->exs,
			es->numThreads, es->sampleFractionGlobal, es->sampleFractionFiles,
			es->sampleFractionSegments, es->sampleFractionPerFrame);
	savePrintDataset(dataset, NULL, es);
	mknn_dataset_release(dataset);
}
struct Param {
	LoadSegmentation *segloader;
	struct State_Export *es;
};
static void extractExportFile_incremental(int64_t current_process,
		void *state_object, int64_t current_thread) {
	struct Param *p = state_object;
	struct State_Export *es = p->es;
	DB *db = loadSegmentation_getDb(p->segloader);
	FileDB *fdb = db->filesDb[current_process];
	char *fname = get_filename_dataset(fdb, es);
	if (!my_io_existsFile(fname)) {
		MknnDataset *dataset = computeDescriptorsSample_singleFile(p->segloader,
				es->exs[current_thread], fdb, es->sampleFractionSegments,
				es->sampleFractionPerFrame);
		savePrintDataset(dataset, fdb, es);
		mknn_dataset_release(dataset);
	}
	free(fname);
	return;
}

static void extractExportFile_process_segment(LoadSegmentation *segloader,
		void *state) {
	struct State_Export *es = state;
	int64_t num_files = loadSegmentation_getDb(segloader)->numFilesDb;
	struct Param p = { 0 };
	p.segloader = segloader;
	p.es = es;
	my_parallel_incremental(num_files, &p, extractExportFile_incremental,
			"EXTRACTEXPORTBYFILE", es->numThreads);
}
static void export_release(void *state) {
	struct State_Export *es = state;
	MY_FREE(es->outFilename);
	MY_FREE(es->fileFormat);
	MY_FREE(es->nameExtractor);
	for (int64_t i = 0; i < es->numThreads; ++i)
		releaseExtractor(es->exs[i]);
	MY_FREE(es->exs);
	MY_FREE(es);
}
#endif

void proc_reg_export() {
#ifndef NO_OPENCV
	addProcessorDefDescriptor("EXPORT",
			"sampleFractionGlobal_sampleFractionFiles_sampleFractionSegments_sampleFractionPerFrame_outFilename_(RAW+TEXT+DATASET)",
			export_new, export_process_desc, export_release);
	addProcessorDefSegmentation("EXTRACTEXPORT",
			"sampleFractionGlobal_sampleFractionFiles_sampleFractionSegments_sampleFractionPerFrame_outFilename_(RAW+TEXT+DATASET)_[extractor]",
			export_new, extractExport_process_segment, export_release);
	addProcessorDefSegmentation("EXTRACTEXPORTBYFILE",
			"sampleFractionSegments_sampleFractionPerFrame_outDir_(RAW+TEXT+DATASET)_[extractor]",
			exportFile_new, extractExportFile_process_segment, export_release);
#endif
}
