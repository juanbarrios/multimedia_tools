/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef GROUND_TRUTH_FRAMES_H
#define GROUND_TRUTH_FRAMES_H

#include "evaluation.h"

struct GroundTruthFrames *newGroundTruthFramesEmpty(
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference);

void GroundTruthFrames_add(struct GroundTruthFrames *gtf,
		struct SearchSegment *kfq, struct SearchSegment *kfr);

struct GroundTruthFrames *newGroundTruthFileSegments(
		const char *file_groundTruthSegments, struct SearchCollection *colQuery,
		struct SearchCollection *colReference);

struct GroundTruthFrames *newGroundTruthFileFrames(
		const char *file_groundTruthFrames, struct SearchCollection *colQuery,
		struct SearchCollection *colReference);

struct GroundTruthFrames *newGroundTruthFilesFramesCategories(
		const char *file_groundTruthFramesCategories,
		const char *file_groundTruthCategoriesFrames,
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference);

void releaseGroundTruthFrames(struct GroundTruthFrames *gtf);

MknnEvaluation_s *GroundTruthFrames_evaluateFile(struct GroundTruthFrames *gtf,
		struct ArchivoFrames *af);

#endif
