/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef LOADSEGMENTATION_H
#define LOADSEGMENTATION_H

#include "../pvcd.h"

LoadSegmentation *newLoadSegmentation(DB *db, const char *segAlias);
DB *loadSegmentation_getDb(LoadSegmentation *loader);
void releaseLoadSegmentation(LoadSegmentation *loader);

struct VideoSegment {
	int64_t num_segment, selected_frame, start_frame, end_frame;
	double selected_second, start_second, end_second;
};

struct Segmentation {
	int64_t num_segments;
	double video_fps;
	struct VideoSegment *segments;
};

const struct Segmentation* loadSegmentationFileId(LoadSegmentation *loader,
		const char *file_id);
const struct Segmentation* loadSegmentationFileDB(LoadSegmentation *loader,
		FileDB *fdb);

void validateSegmentation(const struct Segmentation *seg);
void contFramesAndSecondsInSegmentation(const struct Segmentation *seg,
		int64_t *out_cont_frames, double *out_cont_seconds);

struct Segmentation *createNewSegmentation(int64_t num_segments,
		double video_fps);
void releaseSegmentation(struct Segmentation *seg);

#endif
