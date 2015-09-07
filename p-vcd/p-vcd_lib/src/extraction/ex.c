/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

void ext_reg_canny();
void ext_reg_histGray();
void ext_reg_histByChannel();
void ext_reg_histColFixed();
void ext_reg_kf();
void ext_reg_le();
void ext_reg_gdh();
void ext_reg_edg();
void ext_reg_ehd();
void ext_reg_omd();

void reg_local_local();

void ext_reg_average();
void ext_reg_bow();
void ext_reg_text();

void pvcd_register_default_extractors() {
#ifndef NO_OPENCV
	//global
	ext_reg_canny();
	ext_reg_histGray();
	ext_reg_histByChannel();
	ext_reg_histColFixed();
	ext_reg_kf();
	ext_reg_le();
	ext_reg_gdh();
	ext_reg_edg();
	ext_reg_ehd();
	ext_reg_omd();
	//local
	reg_local_local();
	//segment
	ext_reg_average();
	ext_reg_bow();
	ext_reg_text();
#endif
}

#ifndef NO_OPENCV

struct Extractor_Def2 {
	char *code, *help;
	bool isGlobal, isLocal, isSegment;
	extract_func_new func_new;
	extract_func_extract_image func_extract_image;
	extract_func_init_video func_init_video;
	extract_func_extract_segment func_extract_segment;
	extract_func_end_video func_end_video;
	extract_func_release func_release;
};

static MyVectorObj *defs_ext = NULL;

static struct Extractor_Def2 *addExtractorDef(const char *code,
		const char *help) {
	struct Extractor_Def2 *def = MY_MALLOC(1, struct Extractor_Def2);
	def->code = my_newString_string(code);
	def->help = my_newString_string(help);
	if (defs_ext == NULL) {
		defs_ext = my_vectorObj_new();
	} else {
		int64_t i;
		for (i = 0; i < my_vectorObj_size(defs_ext); ++i) {
			struct Extractor_Def2 *d = my_vectorObj_get(defs_ext, i);
			if (my_string_equals(d->code, code))
				my_log_error("extractor %s already defined\n", code);
		}
	}
	my_vectorObj_add(defs_ext, def);
	return def;
}
void addExtractorGlobalDef(bool isLocal, const char *code, const char *help,
		extract_func_new func_new,
		extract_func_extract_image func_extract_image,
		extract_func_release func_release) {
	struct Extractor_Def2 *def = addExtractorDef(code, help);
	def->func_new = func_new;
	def->func_extract_image = func_extract_image;
	def->func_release = func_release;
	def->isGlobal = isLocal ? 0 : 1;
	def->isLocal = isLocal ? 1 : 0;
}
void addExtractorSegmentDef(const char *code, const char *help,
		extract_func_new func_new, extract_func_init_video func_init_video,
		extract_func_extract_segment func_extract_segment,
		extract_func_end_video func_end_video,
		extract_func_release func_release) {
	struct Extractor_Def2 *def = addExtractorDef(code, help);
	def->func_new = func_new;
	def->func_init_video = func_init_video;
	def->func_extract_segment = func_extract_segment;
	def->func_end_video = func_end_video;
	def->func_release = func_release;
	def->isSegment = true;
}

static void print_extractors_type(uchar isGlobal, uchar isLocal,
		uchar isSegment) {
	if (defs_ext == NULL)
		return;
	for (int64_t i = 0; i < my_vectorObj_size(defs_ext); ++i) {
		struct Extractor_Def2 *def = my_vectorObj_get(defs_ext, i);
		if ((def->isGlobal && isGlobal) || (def->isLocal && isLocal)
				|| (def->isSegment && isSegment))
			my_log_info("  %s%s%s\n", def->code, (def->help == NULL) ? "" : "_",
					(def->help == NULL) ? "" : def->help);
	}
}
#endif

void print_extractors() {
#ifndef NO_OPENCV
	my_log_info("\nAvailable Global Extractors:\n");
	print_extractors_type(1, 0, 0);
	my_log_info("\nAvailable Local Extractors:\n");
	print_extractors_type(0, 1, 0);
	my_log_info("\nAvailable Segment Extractors:\n");
	print_extractors_type(0, 0, 1);
	my_log_info("\nResize Options:\n  %s\n", my_imageResizer_getTextOptions());
#endif
}

void print_extractors_local() {
#ifndef NO_OPENCV
	my_log_info("\nAvailable Local Extractors:\n");
	print_extractors_type(0, 1, 0);
	my_log_info("\nResize Options:\n  %s\n", my_imageResizer_getTextOptions());
#endif
}
#ifndef NO_OPENCV
struct Extractor_Def2 *searchExtractorDef(const char *code) {
	if (defs_ext != NULL) {
		for (int64_t i = 0; i < my_vectorObj_size(defs_ext); ++i) {
			struct Extractor_Def2 *d = my_vectorObj_get(defs_ext, i);
			if (my_string_equals(d->code, code))
				return d;
		}
	}
	my_log_error("unknown extractor %s\n", code);
	return NULL;
}
struct Extractor {
	char *codeAndParameters;
	struct Extractor_Def2 *def;
	void *state;
	DescriptorType td;
	bool useImgGray;
	MyImageColor *converter;
};

Extractor *getExtractor2(const char *code, const char *parameters) {
	Extractor *ex = MY_MALLOC(1, Extractor);
	ex->def = searchExtractorDef(code);
	ex->def->func_new(ex->def->code, parameters, &ex->state, &ex->td,
			&ex->useImgGray);
	my_assert_notNull("state", ex->state);
	my_assert_greaterInt("dtype", ex->td.dtype, 0);
	if (ex->useImgGray)
		ex->converter = my_imageColor_newConverterToGray();
	ex->codeAndParameters =
			(parameters == NULL || strlen(parameters) == 0) ?
					my_newString_string(ex->def->code) :
					my_newString_format("%s_%s", ex->def->code, parameters);
	return ex;
}
Extractor *getExtractor(const char *codeAndParameters) {
	MyTokenizer *tk = my_tokenizer_new(codeAndParameters, '_');
	const char *code = my_tokenizer_nextToken(tk);
	Extractor *ex = getExtractor2(code, my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	return ex;
}
const char *getExtractorHelp(const char *extCode) {
	struct Extractor_Def2 *def = searchExtractorDef(extCode);
	return def->help;
}
DescriptorType getDescriptorType(Extractor *ex) {
	return ex->td;
}
void* getExtractorState(Extractor *ex) {
	return ex->state;
}
const char *getExtractorCodeAndParameters(Extractor *ex) {
	return ex->codeAndParameters;
}
//do not MY_FREE returned descriptor (will be released at releaseExtractor)
void *extractVolatileDescriptor(Extractor *ex, IplImage *image) {
	//if (ex->def->isSegment)
	//my_log_error("extractor %s is not for single images\n", ex->def->code);
	if (ex->useImgGray && image->nChannels != 1)
		image = my_imageColor_convertFromBGR(image, ex->converter);
	return ex->def->func_extract_image(image, ex->state);
}
void releaseExtractor(Extractor *ex) {
	if (ex == NULL)
		return;
	if (ex->def->func_release != NULL)
		ex->def->func_release(ex->state);
	if (ex->useImgGray)
		my_imageColor_release(ex->converter);
	free(ex->codeAndParameters);
	free(ex);
}
static void priv_extractPersistentDescriptors_segmented(Extractor *ex,
		FileDB *fdb, const struct Segmentation *seg, void **descriptors_first,
		int64_t firstSegment, int64_t lastSegmentNotIncluded, MyProgress *lt) {
	my_assert_notNull("seg", seg);
	VideoFrame *video_frame = openFileDB(fdb, 1);
	if (ex->def->isSegment) {
		struct Extractor_InitParams ip;
		ip.video_frame = video_frame;
		ip.segmentation = seg;
		ip.fileDB = fdb;
		if (ex->def->func_init_video != NULL)
			ex->def->func_init_video(&ip, ex->state);
		for (int64_t j = firstSegment; j < lastSegmentNotIncluded; ++j) {
			void *des = ex->def->func_extract_segment(&ip, j, ex->state);
			descriptors_first[j - firstSegment] = cloneDescriptor(ex->td, des);
			if (lt != NULL)
				my_progress_add1(lt);
		}
		if (ex->def->func_end_video != NULL)
			ex->def->func_end_video(&ip, ex->state);
	} else {
		for (int64_t j = firstSegment; j < lastSegmentNotIncluded; ++j) {
			struct VideoSegment s = seg->segments[j];
			if (!seekVideoToFrame(video_frame, s.selected_frame))
				my_log_info(
						"extract: can't jump to segment %"PRIi64" (frame #%"PRIi64")\n",
						j, s.selected_frame);
			IplImage *image = getCurrentFrameOrig(video_frame);
			void *des = extractVolatileDescriptor(ex, image);
			descriptors_first[j - firstSegment] = cloneDescriptor(ex->td, des);
			if (lt != NULL)
				my_progress_add1(lt);
		}
	}
	closeVideo(video_frame);
}
static void priv_printDataStats(FileDB *fdb, int64_t numDescriptors,
		DescriptorType td, void **descriptors) {
	int64_t totalBytes = 0, totalVectors = 0;
	for (int64_t i = 0; i < numDescriptors; ++i) {
		if (descriptors[i] == NULL)
			continue;
		if (td.dtype == DTYPE_LOCAL_VECTORS) {
			totalVectors += my_localDescriptors_getNumDescriptors(
					descriptors[i]);
			totalBytes += my_localDescriptors_serializePredictSizeBytes(
					descriptors[i]);
		} else if (td.dtype == DTYPE_SPARSE_ARRAY) {
			totalBytes += my_sparseArray_serializePredictSizeBytes(
					descriptors[i]);
		} else {
			totalBytes += getLengthBytesDescriptor(td);
		}
	}
	char *stSize = my_newString_diskSpace(totalBytes);
	if (td.dtype == DTYPE_LOCAL_VECTORS) {
		char *stAvg = my_newString_doubleDec(
				totalVectors / ((double) numDescriptors), 1);
		my_log_info_time(
				"%s: %"PRIi64" local vectors in %"PRIi64" frames, %s vec/img, %s.\n",
				fdb->id, totalVectors, numDescriptors, stAvg, stSize);
		free(stAvg);
	} else {
		my_log_info_time("%s: %"PRIi64" descriptors, %s.\n", fdb->id,
				numDescriptors, stSize);
	}
	free(stSize);
}
void extractPersistentDescriptors_seg(Extractor *ex, FileDB *fdb,
		const struct Segmentation *seg, int64_t first_segment,
		int64_t last_segmentNotIncluded, void **persistent_descriptors) {
	int64_t num_segments = last_segmentNotIncluded - first_segment;
	my_assert_greaterInt("num_segments", num_segments, 0);
	MyProgress *lt = NULL;
	if (num_segments > 1)
		lt = my_progress_new(fdb->id, num_segments, 1);
	priv_extractPersistentDescriptors_segmented(ex, fdb, seg,
			persistent_descriptors, first_segment, last_segmentNotIncluded, lt);
	priv_printDataStats(fdb, num_segments, ex->td, persistent_descriptors);
	my_progress_release(lt);
}
struct ParallelSegmented {
	Extractor **exs;
	FileDB *fdb;
	const struct Segmentation *seg;
	int64_t first_segment;
	MyProgress *lt;
	void **descriptors;
};
static void priv_extractPersistentDescriptors_segmented_thread(
		int64_t start_process, int64_t end_process_notIncluded,
		void *state_object, MyProgress *lt, int64_t current_thread) {
	struct ParallelSegmented *state = state_object;
	Extractor *ex = state->exs[current_thread];
	priv_extractPersistentDescriptors_segmented(ex, state->fdb, state->seg,
			state->descriptors + start_process,
			state->first_segment + start_process,
			state->first_segment + end_process_notIncluded, state->lt);
}
void extractPersistentDescriptors_threadedSegments(Extractor **exs,
		int64_t numExtractors, FileDB *fdb, const struct Segmentation *seg,
		int64_t first_segment, int64_t last_segmentNotIncluded,
		void **persistent_descriptors) {
	int64_t num_segments = last_segmentNotIncluded - first_segment;
	my_assert_greaterInt("num_segments", num_segments, 0);
	struct ParallelSegmented state = { 0 };
	state.exs = exs;
	state.fdb = fdb;
	state.seg = seg;
	state.first_segment = first_segment;
	state.descriptors = persistent_descriptors;
	state.lt = my_progress_new(fdb->id, num_segments, 1);
	int64_t segment_size = my_math_ceil_int(
			num_segments / (double) numExtractors);
	my_parallel_buffered(num_segments, &state,
			priv_extractPersistentDescriptors_segmented_thread,
			NULL, numExtractors, segment_size);
	my_progress_release(state.lt);
	priv_printDataStats(fdb, num_segments, getDescriptorType(exs[0]),
			persistent_descriptors);
}
void releasePersistentDescriptors(void **descriptors, int64_t numDescriptors,
		DescriptorType td) {
	if (td.dtype == DTYPE_LOCAL_VECTORS) {
		for (int64_t i = 0; i < numDescriptors; ++i)
			my_localDescriptors_release(descriptors[i]);
	} else if (td.dtype == DTYPE_SPARSE_ARRAY) {
		for (int64_t i = 0; i < numDescriptors; ++i)
			my_sparseArray_release(descriptors[i]);
	} else {
		for (int64_t i = 0; i < numDescriptors; ++i)
			free(descriptors[i]);
	}
}
#endif
