/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef DETECTION_H
#define DETECTION_H

#include "../pvcd.h"
#include <metricknn/metricknn_c/metricknn_impl.h>

struct ParamsVoting {
	MknnHistogram *dist_hist;
	//voting
	double matchVote, missCost, rankWeight, stepSize;
	int64_t matchType;
	//others
	double minLengthSecondsQ, minLengthSecondsR, minLengthFractionVideoQ,
			minLengthFractionVideoR;
	int64_t maxNNLoad, maxDetections;
	double maxDistLoad;
	int singleDetection;
	bool useDistWeight;
	//to print
	const char *matchVote_st, *missCost_st, *rankWeight_st, *stepSize_st,
			*matchType_st, *minimumLength_st, *singleDetection_st,
			*useDistWeight_st, *maxNNLoad_st, *maxDistLoad_st,
			*maxDetections_st;
};

void det_reportDetection(struct SearchSegment *desde_q,
		struct SearchSegment *hasta_q, struct SearchSegment *desde_r,
		struct SearchSegment *hasta_r, double score, int64_t numVotantes,
		int64_t numSinVotos, void *stateVot);
void det_detectByVoting(void *qqueryVideo, struct ParamsVoting *pv,
		int64_t numThreads, void *stateVot);

struct Detection {
	struct SearchFile *videoQ, *videoR;
	double secStartQ, secEndQ, secStartR, secEndR, score;
	int64_t numVoters, numMisses, scoreRankInQuery;
	//space to attach internal info
	void *extraInfo;
};
struct DetectionsFile {
	//array de struct Detection*
	MyVectorObj *allDetections;
	//videoName => Array_obj* of struct Detection*
	MyMapStringObj *detectionsPerQuery;
};
struct DetectionsFile *loadDetectionsFile(const char *filename,
		struct SearchProfile *profile);
void releaseDetectionsFile(struct DetectionsFile *df);
FILE *createDetectionsFile(const char *filename, CmdParams *cmd_params,
		const char *comments);
void sortDetectionsByScore(MyVectorObj *detections);
void printDetectionsToFile(FILE *out, MyVectorObj *detections,
		int64_t maxDetections);
void forceSameDetectionLength(struct Detection *det);

struct DetPrinterOptions {
	bool useWindowsFormat;
	bool printOnlyQ, printOnlyR, printBothQR, overwrite;
	const char *newline;
};

typedef FILE *(*func_detPrinter_openFile)(const char *filename,
		struct DetPrinterOptions *opt);

typedef void (*func_detPrinter_printDetection)(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt);

typedef void (*func_detPrinter_closeFile)(FILE *out,
		struct DetPrinterOptions *opt);

void register_detectionsPrinter(const char *code, const char *description,
		func_detPrinter_openFile func_openFile,
		func_detPrinter_printDetection func_printDetection,
		func_detPrinter_closeFile func_closeFile);

void register_default_detectionsPrinters();

#endif
