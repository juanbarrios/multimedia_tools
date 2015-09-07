/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef GROUND_TRUTH_SEGMENTS_H
#define GROUND_TRUTH_SEGMENTS_H

#include "evaluation.h"

struct GroundTruthSegments *newGroundTruthSegments(const char *file_groundTruth,
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference);

void releaseGroundTruthSegments(struct GroundTruthSegments *gts);

int64_t groundtruth_correctSegment(struct GroundTruthSegments *gts,
		struct SearchFile *arcQuery, double seg_ini_q, double seg_end_q,
		struct SearchFile *arcRef, double seg_ini_r, double seg_end_r);

struct GroundTruthFrames* newGroundTruthFrames_segms(
		struct GroundTruthSegments *gts);

#endif
