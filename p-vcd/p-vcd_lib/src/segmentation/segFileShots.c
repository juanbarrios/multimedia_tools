/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "seg.h"

struct VideoShotsData {
	int64_t num_shots;
	double *seconds_start, *seconds_end, *seconds_selected;
};
static void loadShots(MyMapStringObj *videoName2segmentation,
		const char *fileShots) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(fileShots, 1), "PVCD", "Shots", 1, 1);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *videoName = my_tokenizer_nextToken_newString(tk);
		struct VideoShotsData *vsd = MY_MALLOC(1, struct VideoShotsData);
		vsd->num_shots = my_tokenizer_nextInt(tk);
		my_tokenizer_nextDouble(tk); //fps
		my_tokenizer_nextDouble(tk); //video length
		my_tokenizer_releaseValidateEnd(tk);
		vsd->seconds_start = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_end = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_selected = MY_MALLOC(vsd->num_shots, double);
		for (int64_t i = 0; i < vsd->num_shots; ++i) {
			line = my_lreader_readLine(reader);
			my_assert_notNull("line", line);
			MyTokenizer *tk2 = my_tokenizer_new(line, '\t');
			my_tokenizer_nextToken(tk2); //shot name
			vsd->seconds_start[i] = my_tokenizer_nextDouble(tk2);
			vsd->seconds_end[i] = my_tokenizer_nextDouble(tk2);
			my_tokenizer_releaseValidateEnd(tk2);
			vsd->seconds_selected[i] = (vsd->seconds_start[i]
					+ vsd->seconds_end[i]) / 2;
		}
		my_mapStringObj_add(videoName2segmentation, videoName, vsd);
	}
	my_lreader_close(reader, true);
}
static void loadShotsV2(MyMapStringObj *videoName2segmentation,
		const char *fileShots) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(fileShots, 1), "PVCD", "ShotsV2", 1, 0);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		char *videoName = my_newString_string(line);
		struct VideoShotsData *vsd = MY_MALLOC(1, struct VideoShotsData);
		int64_t total_frames_video = my_parse_int(my_lreader_readLine(reader));
		my_lreader_readLine(reader); //seconds length video
		double video_fps = my_parse_double(my_lreader_readLine(reader));
		video_fps = 25; //correct
		int64_t size = my_parse_int(my_lreader_readLine(reader));
		MyVectorInt *scenecuts = my_tokenizer_splitLineInt(
				my_lreader_readLine(reader), ' ');
		my_assert_equalInt("num values", my_vectorInt_size(scenecuts), size);
		my_vectorInt_qsort(scenecuts);
		vsd->num_shots = size + 1;
		vsd->seconds_start = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_end = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_selected = MY_MALLOC(vsd->num_shots, double);
		for (int64_t i = 0; i < vsd->num_shots; ++i) {
			if (i == 0) {
				vsd->seconds_start[i] = 0;
				vsd->seconds_end[i] = my_vectorInt_get(scenecuts, i)
						/ video_fps;
			} else if (i == vsd->num_shots - 1) {
				vsd->seconds_start[i] = my_vectorInt_get(scenecuts, i - 1)
						/ video_fps;
				vsd->seconds_end[i] = total_frames_video / video_fps;
			} else {
				vsd->seconds_start[i] = my_vectorInt_get(scenecuts, i - 1)
						/ video_fps;
				vsd->seconds_end[i] = my_vectorInt_get(scenecuts, i)
						/ video_fps;
			}
			vsd->seconds_selected[i] = (vsd->seconds_start[i]
					+ vsd->seconds_end[i]) / 2;
		}
		my_vectorInt_release(scenecuts);
		my_mapStringObj_add(videoName2segmentation, videoName, vsd);
	}
	my_lreader_close(reader, true);
}
static void loadShotsV3(MyMapStringObj *videoName2segmentation,
		const char *fileShots) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(fileShots, 1), "PVCD", "ShotsV3", 1, 0);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		char *videoName = my_newString_string(line);
		struct VideoShotsData *vsd = MY_MALLOC(1, struct VideoShotsData);
		int64_t total_frames_video = my_parse_int(my_lreader_readLine(reader));
		my_lreader_readLine(reader); //seconds length video
		double video_fps = my_parse_double(my_lreader_readLine(reader));
		video_fps = 25; //correct
		int64_t size = my_parse_int(my_lreader_readLine(reader));
		MyVectorInt *samples = my_tokenizer_splitLineInt(
				my_lreader_readLine(reader), ' ');
		my_assert_equalInt("num values", my_vectorInt_size(samples), size);
		my_vectorInt_qsort(samples);
		vsd->num_shots = size;
		vsd->seconds_start = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_end = MY_MALLOC(vsd->num_shots, double);
		vsd->seconds_selected = MY_MALLOC(vsd->num_shots, double);
		for (int64_t i = 0; i < vsd->num_shots; ++i) {
			vsd->seconds_selected[i] = my_vectorInt_get(samples, i) / video_fps;
			if (i == 0) {
				vsd->seconds_start[i] = 0;
			} else {
				vsd->seconds_start[i] =
						vsd->seconds_end[i - 1] = (vsd->seconds_selected[i - 1]
								+ vsd->seconds_selected[i]) / 2;
			}
			if (i == vsd->num_shots - 1) {
				vsd->seconds_end[i] = total_frames_video / video_fps;
			}
		}
		my_vectorInt_release(samples);
		my_mapStringObj_add(videoName2segmentation, videoName, vsd);
	}
	my_lreader_close(reader, true);
}

struct State_FileShots {
	char *filenameShots;
	MyMapStringObj *videoName2segmentation;
	int64_t numSamplesByShot;
	double removeStart, removeEnd;
};
static void seg_new(const char *segCode, const char *segParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	struct State_FileShots *es = MY_MALLOC(1, struct State_FileShots);
	es->numSamplesByShot = my_tokenizer_nextInt(tk);
	if (my_tokenizer_isNext(tk, "REMOVESTART"))
		es->removeStart = my_tokenizer_nextDouble(tk);
	if (my_tokenizer_isNext(tk, "REMOVEEND"))
		es->removeEnd = my_tokenizer_nextDouble(tk);
	es->filenameShots = my_newString_string(my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	es->videoName2segmentation = my_mapStringObj_newCaseSensitive();
	char *format = my_io_detectFileConfig(es->filenameShots, "PVCD");
	if (format != NULL && my_string_equals(format, "Shots")) {
		loadShots(es->videoName2segmentation, es->filenameShots);
	} else if (format != NULL && my_string_equals(format, "ShotsV2")) {
		loadShotsV2(es->videoName2segmentation, es->filenameShots);
	} else if (format != NULL && my_string_equals(format, "ShotsV3")) {
		loadShotsV3(es->videoName2segmentation, es->filenameShots);
	} else {
		my_log_error("unknown file format %s\n", format);
	}
	*out_state = es;
}
static bool isValidSegment(double seconds_start, double seconds_end,
		FileDB *fdb, struct State_FileShots *es) {
	if (!MY_INTERSECT(seconds_start, seconds_end, fdb->secStartTime,
			fdb->secEndTime))
		return false;
	if (es->removeStart
			> 0&& MY_INTERSECT(seconds_start, seconds_end, fdb->secStartTime,
					fdb->secStartTime + es->removeStart))
		return false;
	if (es->removeEnd > 0&&MY_INTERSECT(seconds_start, seconds_end,
			fdb->secEndTime - es->removeEnd, fdb->secEndTime))
		return false;
	return true;
}
static struct Segmentation* seg_segment(FileDB *fdb, void* state) {
	struct State_FileShots *es = state;
	struct VideoShotsData *vsd = my_mapStringObj_get(es->videoName2segmentation,
			fdb->id);
	if (vsd == NULL) {
		char *name = my_io_getFilenameWithoutExtension(fdb->filenameReal);
		vsd = my_mapStringObj_get(es->videoName2segmentation, name);
		free(name);
	}
	if (vsd == NULL)
		my_log_error("can't find segmentation for %s\n", fdb->id);
	int64_t contSelected = 0;
	for (int64_t i = 0; i < vsd->num_shots; ++i) {
		if (isValidSegment(vsd->seconds_start[i], vsd->seconds_end[i], fdb, es))
			contSelected++;
	}
	struct Segmentation *seg = createNewSegmentation(
			contSelected * es->numSamplesByShot, fdb->numObjsPerSecond);
	int64_t cont = 0;
	for (int64_t i = 0; i < vsd->num_shots; ++i) {
		if (!isValidSegment(vsd->seconds_start[i], vsd->seconds_end[i], fdb,
				es))
			continue;
		double corrected_start = MAX(vsd->seconds_start[i], fdb->secStartTime);
		double corrected_end = MIN(vsd->seconds_end[i], fdb->secEndTime);
		if (es->numSamplesByShot > 1) {
			double offset = (corrected_end - corrected_start)
					/ es->numSamplesByShot;
			for (int64_t j = 0; j < es->numSamplesByShot; ++j) {
				struct VideoSegment *sel = seg->segments + cont;
				sel->start_second = corrected_start + offset * j;
				sel->end_second = corrected_start + offset * (j + 1);
				sel->selected_second = (sel->start_second + sel->end_second)
						/ 2;
				cont++;
			}
		} else {
			struct VideoSegment *sel = seg->segments + cont;
			sel->start_second = corrected_start;
			sel->end_second = corrected_end;
			sel->selected_second = MAX(corrected_start,
					MIN(corrected_end,vsd->seconds_selected[i] ));
			cont++;
		}
	}
	//frame numbers
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		struct VideoSegment *sel = seg->segments + i;
		sel->start_frame = my_math_round_int(
				sel->start_second * seg->video_fps);
		sel->end_frame = my_math_round_int(sel->end_second * seg->video_fps);
		int64_t s = my_math_round_int(sel->selected_second * seg->video_fps);
		sel->selected_frame = MAX(sel->start_frame, MIN(sel->end_frame, s));
	}
	return seg;
}
static void seg_release(void *state) {
	struct State_FileShots *es = state;
	free(es->filenameShots);
	free(es);
}

void seg_reg_fileshots() {
	addSegmentatorDef("FILESHOTS",
			"numSamplesByShot_[REMOVESTART_seconds]_[REMOVEEND_seconds]_filenameShots",
			seg_new, seg_segment, seg_release);
}
