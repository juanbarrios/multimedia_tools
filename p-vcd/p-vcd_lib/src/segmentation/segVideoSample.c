/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "seg.h"

struct State_VideoSample {
	int64_t numSamplesByVideo;
};

static void seg_config_sample(const char *segCode, const char *segParameters,
		void **out_state) {
	struct State_VideoSample *es = MY_MALLOC(1, struct State_VideoSample);
	es->numSamplesByVideo = my_parse_int(segParameters);
	*out_state = es;
}
static struct Segmentation* seg_segmentar_sample(FileDB *fdb, void* state) {
	struct State_VideoSample *es = state;
	my_assert_isTrue("isVideo", fdb->isVideo);
	my_assert_greaterDouble("fps", fdb->numObjsPerSecond, 0);
	int64_t totalFrames = my_math_round_int(
			fdb->lengthSec * fdb->numObjsPerSecond);
	int64_t numSelectedFrames = es->numSamplesByVideo;
	if (numSelectedFrames > totalFrames) {
		my_log_info(
				"warning: %s contains less than %"PRIi64" frames, selecting only %"PRIi64" frames\n",
				fdb->id, es->numSamplesByVideo, totalFrames);
		numSelectedFrames = totalFrames;
	}
	int64_t *limits = my_math_partitionIntUB(numSelectedFrames, totalFrames);
	struct Segmentation *seg = createNewSegmentation(numSelectedFrames,
			fdb->numObjsPerSecond);
	for (int64_t i = 0; i < numSelectedFrames; ++i) {
		int64_t start = (i == 0) ? 0 : limits[i - 1];
		int64_t end = limits[i];
		struct VideoSegment *sel = seg->segments + i;
		sel->start_frame = start;
		sel->end_frame = MIN(end, totalFrames) - 1;
		sel->start_second = sel->start_frame / fdb->numObjsPerSecond;
		sel->end_second = (sel->end_frame + 1) / fdb->numObjsPerSecond;
		sel->selected_frame = (sel->start_frame + sel->end_frame) / 2;
		sel->selected_second = (sel->start_second + sel->end_second) / 2;
	}
	MY_FREE(limits);
	return seg;
}

static void seg_release_sample(void *state) {
	struct State_VideoSample *es = state;
	MY_FREE(es);
}

void seg_reg_sample() {
	addSegmentatorDef("VIDEOSAMPLE", "numSamples", seg_config_sample,
			seg_segmentar_sample, seg_release_sample);
}

