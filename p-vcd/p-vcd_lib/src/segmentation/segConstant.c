/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "seg.h"

struct Estado_CTE {
	bool isByFrames;
	double sizeInFrames, sizeInSeconds;
	double stepSize;
};

static void seg_config_cte(const char *segCode, const char *segParameters,
		void **out_state) {
	struct Estado_CTE *es = MY_MALLOC(1, struct Estado_CTE);
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	double value = my_tokenizer_nextFraction(tk);
	if (my_tokenizer_isNext(tk, "FRAMES")) {
		es->sizeInFrames = value;
		es->isByFrames = true;
	} else {
		es->sizeInSeconds = value;
	}
	if (my_tokenizer_isNext(tk, "STEP")) {
		es->stepSize = my_tokenizer_nextFraction(tk);
	}
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}
static struct Segmentation* partition_without_overlap(FileDB *fdb,
		struct Estado_CTE *es) {
	int64_t totalFrames = my_math_round_int(
			fdb->lengthSec * fdb->numObjsPerSecond);
	int64_t video_frames_offset = my_math_round_int(
			fdb->secStartTime * fdb->numObjsPerSecond);
	double framesPerSegment =
			es->isByFrames ?
					es->sizeInFrames :
					(es->sizeInSeconds * fdb->numObjsPerSecond);
	MyVectorInt *limites = my_math_computeBinSizes(framesPerSegment,
			totalFrames);
	struct Segmentation *seg = createNewSegmentation(
			my_vectorInt_size(limites) - 1, fdb->numObjsPerSecond);
	for (int64_t i = 0; i < my_vectorInt_size(limites) - 1; ++i) {
		int64_t start = my_vectorInt_get(limites, i);
		int64_t end = my_vectorInt_get(limites, i + 1);
		struct VideoSegment *sel = seg->segments + i;
		sel->start_frame = start;
		sel->end_frame = MIN(end, totalFrames) - 1;
		sel->selected_frame = (sel->start_frame + sel->end_frame) / 2;
		sel->start_second = start / seg->video_fps;
		sel->end_second = MIN(end, totalFrames) / seg->video_fps;
		sel->selected_second = (sel->start_second + sel->end_second) / 2;
		if (video_frames_offset > 0) {
			sel->start_frame += video_frames_offset;
			sel->end_frame += video_frames_offset;
			sel->selected_frame += video_frames_offset;
			sel->start_second += fdb->secStartTime;
			sel->end_second += fdb->secStartTime;
			sel->selected_second += fdb->secStartTime;
		}
	}
	my_vectorInt_release(limites);
	return seg;
}
static struct Segmentation* segment_with_overlap(FileDB *fdb,
		struct Estado_CTE *es) {
	my_assert_isFalse("FRAMES", es->isByFrames);
	my_assert_greaterDouble("size", es->sizeInSeconds, 0);
	int64_t cont_segments = 0;
	double start = fdb->secStartTime;
	for (;;) {
		double middle = start + es->sizeInSeconds / 2;
		if (middle >= fdb->secEndTime)
			break;
		cont_segments++;
		start += es->stepSize;
	}
	if (cont_segments == 0)
		cont_segments = 1;
	struct Segmentation *seg = createNewSegmentation(cont_segments,
			fdb->numObjsPerSecond);
	start = fdb->secStartTime;
	for (int64_t i = 0; i < cont_segments; ++i) {
		struct VideoSegment *sel = seg->segments + i;
		sel->start_second = start;
		sel->end_second = MIN(sel->start_second + es->sizeInSeconds,
				fdb->secEndTime);
		sel->selected_second = (sel->start_second + sel->end_second) / 2;
		sel->start_frame = my_math_round_int(
				sel->start_second * seg->video_fps);
		sel->end_frame = my_math_round_int(sel->end_second * seg->video_fps);
		sel->selected_frame = (sel->start_frame + sel->end_frame) / 2;
		start += es->stepSize;
	}
	return seg;
}
static struct Segmentation* seg_segmentar_cte(FileDB *fdb, void* state) {
	struct Estado_CTE *es = state;
	if (!fdb->isVideo && !fdb->isAudio)
		my_log_error("only audio or video can be segmented\n");
	my_assert_greaterDouble("lengthSec", fdb->lengthSec, 0);
	my_assert_greaterDouble("numObjsPerSecond", fdb->numObjsPerSecond, 0);
	if (es->stepSize > 0)
		return segment_with_overlap(fdb, es);
	else
		return partition_without_overlap(fdb, es);
}

static void seg_release_cte(void *state) {
	struct Estado_CTE *es = state;
	MY_FREE(es);
}

void seg_reg_cte() {
	addSegmentatorDef("SEGCTE", "size_[FRAMES]_[STEP_stepSize]", seg_config_cte,
			seg_segmentar_cte, seg_release_cte);
}

