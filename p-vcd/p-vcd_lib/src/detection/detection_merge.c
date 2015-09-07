/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

struct DetectionExtraInfo {
	int64_t contOccurences;
};
static struct Detection *searchDuplicatedCopy(MyVectorObj *detections,
		struct Detection *detNew) {
	int64_t i;
	for (i = 0; i < my_vectorObj_size(detections); ++i) {
		struct Detection *c = my_vectorObj_get(detections, i);
		if ((c->videoR == detNew->videoR)
				&& (my_math_pctIntersection(c->secStartQ, c->secEndQ,
						detNew->secStartQ, detNew->secEndQ) >= 0.5)
				&& (my_math_pctIntersection(c->secStartR, c->secEndR,
						detNew->secStartR, detNew->secEndR) >= 0.5))
			return c;
	}
	return NULL;
}
static void mergeDetectionsSameQuery(MyVectorObj *detsBase,
		MyVectorObj *detsInFile) {
	int64_t i;
	for (i = 0; i < my_vectorObj_size(detsInFile); ++i) {
		struct Detection *detNew = my_vectorObj_get(detsInFile, i);
		struct Detection *detOld = searchDuplicatedCopy(detsBase, detNew);
		if (detOld == NULL) {
			my_vectorObj_add(detsBase, detNew);
		} else {
			detOld->secStartQ = MIN(detOld->secStartQ, detNew->secStartQ);
			detOld->secEndQ = MAX(detOld->secEndQ, detNew->secEndQ);
			detOld->secStartR = MIN(detOld->secStartR, detNew->secStartR);
			detOld->secEndR = MAX(detOld->secEndR, detNew->secEndR);
			detOld->numVoters += detNew->numVoters;
			detOld->numMisses += detNew->numMisses;
			detOld->score += detNew->score;
			struct DetectionExtraInfo *extra = detOld->extraInfo;
			if (extra == NULL) {
				extra = MY_MALLOC(1, struct DetectionExtraInfo);
				extra->contOccurences = 1;
				detOld->extraInfo = extra;
			}
			extra->contOccurences++;
		}
	}
}
static void forceFitSegmentation(double *out_startTime, double *out_endTime,
		const struct Segmentation *seg) {
	double start = *out_startTime;
	double end = *out_endTime;
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		struct VideoSegment s = seg->segments[i];
		if (MY_BETWEEN(start, s.start_second, s.end_second)) {
			double err1 = fabs(start - s.start_second);
			double err2 = fabs(start - s.end_second);
			if (err1 <= err2) {
				start -= err1;
				end -= err1;
			} else {
				start += err2;
				end += err2;
			}
		}
	}
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		struct VideoSegment s = seg->segments[i];
		if (MY_BETWEEN(end, s.start_second, s.end_second)) {
			double err1 = fabs(end - s.start_second);
			double err2 = fabs(end - s.end_second);
			if (err1 <= err2 && (end - err1) > start) {
				end -= err1;
			} else {
				end += err2;
			}
		}
	}
	*out_startTime = start;
	*out_endTime = end;
}

struct Params {
	const char *outFilename;
	int64_t maxDetections;
	bool onlyDuplicates;
	const char *nameSegmentationQ, *nameSegmentationR;
	double minScore;
	CmdParams *cmd_params;
	MyVectorObj *detectionFiles;
	FILE *out;
};
static void mergeQuery(struct SearchFile *videoQ, struct Params *param) {
	MyVectorObj *detsBase = my_vectorObj_new();
	for (int64_t i = 0; i < my_vectorObj_size(param->detectionFiles); ++i) {
		struct DetectionsFile *df = my_vectorObj_get(param->detectionFiles, i);
		MyVectorObj *detsInFile = my_mapStringObj_get(df->detectionsPerQuery,
				videoQ->name);
		if (detsInFile != NULL)
			mergeDetectionsSameQuery(detsBase, detsInFile);
	}
	for (int64_t i = 0; i < my_vectorObj_size(detsBase); ++i) {
		struct Detection *det = my_vectorObj_get(detsBase, i);
		forceSameDetectionLength(det);
		if (param->nameSegmentationQ != NULL) {
			forceFitSegmentation(&det->secStartQ, &det->secEndQ,
					det->videoQ->seg);
		}
		if (param->nameSegmentationR != NULL) {
			forceFitSegmentation(&det->secStartR, &det->secEndR,
					det->videoR->seg);
		}
	}
	for (int64_t i = 0; i < my_vectorObj_size(detsBase); ++i) {
		struct Detection *det = my_vectorObj_get(detsBase, i);
		if (param->onlyDuplicates) {
			struct DetectionExtraInfo *extra = det->extraInfo;
			if (extra == NULL || extra->contOccurences <= 1)
				det->score = 0;
		}
		if (param->minScore > 0 && det->score < param->minScore)
			det->score = 0;
	}
	if (my_vectorObj_size(detsBase) > 0) {
		if (param->out == NULL)
			param->out = createDetectionsFile(param->outFilename,
					param->cmd_params,
					NULL);
		printDetectionsToFile(param->out, detsBase, param->maxDetections);
	}
	my_vectorObj_release(detsBase, 0);
}
void loc_merge_copyDetection(CmdParams *cmd_params, char *argOption) {
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info("  detFile1.txt  detFile2.txt... \n");
		my_log_info(
				"  -out filename                Mandatory. File to be created.\n");
		my_log_info(
				"  -maxDetections num           Optional. Restricts max detections per query.\n");
		my_log_info(
				"  -overwrite                   Optional. Output file can be overwritten.\n");
		my_log_info(
				"  -fitQ segmentationQ          Optional. Modifies boundaries to fit a given segmentation.\n");
		my_log_info(
				"  -fitR segmentationR          Optional. Modifies boundaries to fit a given segmentation.\n");
		my_log_info(
				"  -onlyDuplicates              Optional. Copies in only one file are discarded.\n");
		my_log_info(
				"  -minScore double            Optional. minimum detection score.\n");
		return;
	}
	MyVectorString *filenames = my_vectorString_new();
	struct Params *param = MY_MALLOC(1, struct Params);
	param->cmd_params = cmd_params;
	bool overwrite = false;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-out")) {
			param->outFilename = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-maxDetections")) {
			param->maxDetections = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-overwrite")) {
			overwrite = true;
		} else if (isNextParam(cmd_params, "-onlyDuplicates")) {
			param->onlyDuplicates = true;
		} else if (isNextParam(cmd_params, "-fitQ")) {
			param->nameSegmentationQ = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-fitR")) {
			param->nameSegmentationR = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-minScore")) {
			param->minScore = nextParamDouble(cmd_params);
		} else {
			const char *file = nextParam(cmd_params);
			my_assert_fileExists(file);
			my_vectorStringConst_add(filenames, file);
		}
	}
	my_assert_greaterInt("num files", my_vectorString_size(filenames), 0);
	my_assert_notNull("-out", param->outFilename);
	if (my_io_existsFile(param->outFilename) && !overwrite)
		my_log_error("file %s already exists. Overwrite it with -overwrite\n",
				param->outFilename);
	struct SearchProfile *profile = loadProfileFromFile(
			my_vectorString_get(filenames, 0));
	param->detectionFiles = my_vectorObj_new();
	for (int64_t i = 0; i < my_vectorString_size(filenames); ++i) {
		char *filename = my_vectorString_get(filenames, i);
		struct DetectionsFile *df = loadDetectionsFile(filename, profile);
		my_vectorObj_add(param->detectionFiles, df);
	}
	for (int64_t j = 0; j < profile->colQuery->numFiles; ++j) {
		struct SearchFile *videoQ = profile->colQuery->sfiles[j];
		mergeQuery(videoQ, param);
	}
	if (param->out != NULL)
		fclose(param->out);
	for (int64_t i = 0; i < my_vectorObj_size(param->detectionFiles); ++i) {
		struct DetectionsFile *df = my_vectorObj_get(param->detectionFiles, i);
		releaseDetectionsFile(df);
	}
	releaseProfile(profile);
}
