/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

static void parseDetectionsFile(const char *filename,
		struct SearchProfile *profile, MyVectorObj *dets) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filename, 1), "PVCD", "Detections", 1, 0);
	my_log_info("reading %s\n", filename);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		struct Detection *d = MY_MALLOC(1, struct Detection);
		d->score = my_tokenizer_nextDouble(tk);
		d->videoQ = findSearchFileInCollection(profile->colQuery,
				my_tokenizer_nextToken(tk), 0);
		d->secStartQ = my_tokenizer_nextSeconds(tk);
		d->secEndQ = my_tokenizer_nextSeconds(tk);
		d->videoR = findSearchFileInCollection(profile->colReference,
				my_tokenizer_nextToken(tk), 0);
		d->secStartR = my_tokenizer_nextSeconds(tk);
		d->secEndR = my_tokenizer_nextSeconds(tk);
		d->numVoters = my_tokenizer_nextInt(tk);
		d->numMisses = my_tokenizer_nextInt(tk);
		my_tokenizer_releaseValidateEnd(tk);
		if (d->videoQ == NULL || d->videoR == NULL) {
			free(d);
			my_log_info("could not load detection %s\n", line);
			continue;
		}
		my_vectorObj_add(dets, d);
	}
	my_lreader_close(reader, true);
}

struct DetectionsFile *loadDetectionsFile(const char *filename,
		struct SearchProfile *profile) {
	struct DetectionsFile *df = MY_MALLOC(1, struct DetectionsFile);
	df->allDetections = my_vectorObj_new();
	parseDetectionsFile(filename, profile, df->allDetections);
	df->detectionsPerQuery = my_mapStringObj_newCaseSensitive();
	for (int64_t i = 0; i < my_vectorObj_size(df->allDetections); ++i) {
		struct Detection *d = my_vectorObj_get(df->allDetections, i);
		MyVectorObj *listQ = my_mapStringObj_get(df->detectionsPerQuery,
				d->videoQ->name);
		if (listQ == NULL) {
			listQ = my_vectorObj_new();
			my_mapStringObj_add(df->detectionsPerQuery, d->videoQ->name, listQ);
		}
		my_vectorObj_add(listQ, d);
	}
	for (int64_t i = 0; i < my_mapStringObj_size(df->detectionsPerQuery); ++i) {
		MyVectorObj *listQ = my_mapStringObj_getObjAt(df->detectionsPerQuery,
				i);
		sortDetectionsByScore(listQ);
		for (int64_t j = 0; j < my_vectorObj_size(listQ); ++j) {
			struct Detection *d = my_vectorObj_get(listQ, j);
			d->scoreRankInQuery = j + 1;
		}
	}
	return df;
}
void releaseDetectionsFile(struct DetectionsFile *df) {
	my_vectorObj_release(df->allDetections, true);
	my_mapStringObj_release(df->detectionsPerQuery, false, false);
	MY_FREE(df);
}

FILE *createDetectionsFile(const char *filename, CmdParams *cmd_params,
		const char *comments) {
	FILE *out = my_io_openFileWrite1Config(filename, "PVCD", "Detections", 1,
			0);
	if (cmd_params != NULL) {
		char *stCmd = get_command_line(cmd_params);
		fprintf(out, "#%s\n", stCmd);
		MY_FREE(stCmd);
	}
	if (comments != NULL)
		fprintf(out, "#%s\n", comments);
	fprintf(out,
			"#score\tqueryVideo\tsec_start_q\tsec_end_q\treferenceVideo\tsec_start_r\tsec_end_r\tnumVoters\tnumMisses\n");
	return out;
}

static int det_compare_score_may_men(const void *a, const void *b) {
	double sa = (*(struct Detection **) a)->score;
	double sb = (*(struct Detection **) b)->score;
	return (sa == sb) ? 0 : (sa > sb ? -1 : 1);
}
void sortDetectionsByScore(MyVectorObj *detections) {
	my_vectorObj_qsort(detections, det_compare_score_may_men);
}
void printDetectionsToFile(FILE *out, MyVectorObj *detections,
		int64_t maxDetections) {
	sortDetectionsByScore(detections);
	int64_t contPrint = 0;
	for (int64_t i = 0; i < my_vectorObj_size(detections); ++i) {
		struct Detection *c = my_vectorObj_get(detections, i);
		if (c->score <= 0)
			continue;
		char *nombreQuery = toString_SearchFile(c->videoQ);
		char *nombreReferencia = toString_SearchFile(c->videoR);
		char *sscore = my_newString_double(c->score);
		char *sdq = my_newString_hhmmssfff(c->secStartQ);
		char *shq = my_newString_hhmmssfff(c->secEndQ);
		char *sdr = my_newString_hhmmssfff(c->secStartR);
		char *shr = my_newString_hhmmssfff(c->secEndR);
		fprintf(out, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%"PRIi64"\t%"PRIi64"\n",
				sscore, nombreQuery, sdq, shq, nombreReferencia, sdr, shr,
				c->numVoters, c->numMisses);
		MY_FREE_MULTI(nombreQuery, nombreReferencia, sscore, sdq, shq, sdr,
				shr);
		contPrint++;
		if (maxDetections > 0 && contPrint >= maxDetections)
			break;
	}
	fflush(out);
}
static void verifyBoundaries(double *startTime, double *endTime,
		double maxLimit) {
	if (*endTime > maxLimit) {
		*startTime -= *endTime - maxLimit;
		*endTime = maxLimit;
	}
	if (*startTime < 0) {
		*endTime -= *startTime;
		*startTime = 0;
	}
}
void forceSameDetectionLength(struct Detection *det) {
	double lengthQ = det->secEndQ - det->secStartQ;
	double lengthR = det->secEndR - det->secStartR;
	double diff = lengthQ - lengthR;
	double delta = fabs(diff / 4.0);
	if (diff > 0) {
		//Q larger than R
		det->secStartQ += delta;
		det->secEndQ -= delta;
		det->secStartR -= delta;
		det->secEndR += delta;
	} else if (diff < 0) {
		//R larger than Q
		det->secStartQ -= delta;
		det->secEndQ += delta;
		det->secStartR += delta;
		det->secEndR -= delta;
	}
	verifyBoundaries(&det->secStartQ, &det->secEndQ,
			getSecondsSearchFile(det->videoQ));
	verifyBoundaries(&det->secStartR, &det->secEndR,
			getSecondsSearchFile(det->videoR));
}
void loc_new_copyDetection(CmdParams *cmd_params, char *argOption);
void loc_print_detections(CmdParams *cmd_params, char *argOption);
void loc_merge_copyDetection(CmdParams *cmd_params, char *argOption);

int pvcd_copyDetection(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s -detect ...\n", getBinaryName(cmd_params));
		my_log_info("%s -print ...\n", getBinaryName(cmd_params));
		my_log_info("%s -merge ...\n", getBinaryName(cmd_params));
		return pvcd_system_exit_error();
	}
	const char *option = nextParam(cmd_params);
	if (my_string_equals(option, "-detect")) {
		loc_new_copyDetection(cmd_params, "-detect");
	} else if (my_string_equals(option, "-print")) {
		loc_print_detections(cmd_params, "-print");
	} else if (my_string_equals(option, "-merge")) {
		loc_merge_copyDetection(cmd_params, "-merge");
	} else {
		my_log_error("unknown parameter %s\n", option);
	}
	return pvcd_system_exit_ok(cmd_params);
}
