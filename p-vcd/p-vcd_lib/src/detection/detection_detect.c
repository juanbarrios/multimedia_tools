/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

struct StateDetectionsGlobal {
	struct ParamsVoting *pv;
	MyVectorObj *lista;
	const char *filenameIn, *filenameOut;
	CmdParams *cmd_params;
	FILE *out;
	pthread_mutex_t mutex;
};
struct StateDetections {
	MyVectorObj *candidates;
	struct StateDetectionsGlobal *stateGlob;
	pthread_mutex_t mutex;
};

#define SIMILAR_VideoQ_AnyR 1
#define SIMILAR_VideoQ_VideoR 2
#define SIMILAR_VideoQ_SegmentR 3
#define SIMILAR_SegmentQ_AnyR 4
#define SIMILAR_SegmentQ_VideoR 5
#define SIMILAR_SegmentQ_SegmentR 6

static bool isSimilarCandidate(struct Detection *c, struct Detection *newDet,
		int singleDetection) {
	//test Q
	switch (singleDetection) {
	case SIMILAR_SegmentQ_AnyR:
	case SIMILAR_SegmentQ_VideoR:
	case SIMILAR_SegmentQ_SegmentR:
		if (my_math_pctIntersection(c->secStartQ, c->secEndQ, newDet->secStartQ,
				newDet->secEndQ) < 0.5)
			return false;
	}
	//test R
	switch (singleDetection) {
	case SIMILAR_VideoQ_AnyR:
	case SIMILAR_SegmentQ_AnyR:
		return 1;
	case SIMILAR_VideoQ_VideoR:
	case SIMILAR_SegmentQ_VideoR:
		if (c->videoR == newDet->videoR)
			return true;
		break;
	case SIMILAR_VideoQ_SegmentR:
	case SIMILAR_SegmentQ_SegmentR:
		if (c->videoR == newDet->videoR
				&& my_math_pctIntersection(c->secStartR, c->secEndR,
						newDet->secStartR, newDet->secEndR) >= 0.5)
			return true;
		break;
	}
	return false;
}
static bool hasMinimumLength(double secStartQ, double secEndQ,
		struct SearchFile *videoQ, double secStartR, double secEndR,
		struct SearchFile *videoR, struct ParamsVoting *pv) {
	double lengthQ = secEndQ - secStartQ;
	double lengthR = secEndR - secStartR;
	if (pv->minLengthSecondsQ > 0 && lengthQ < pv->minLengthSecondsQ)
		return 0;
	if (pv->minLengthSecondsR > 0 && lengthR < pv->minLengthSecondsR)
		return 0;
	if (pv->minLengthFractionVideoQ > 0
			&& lengthQ / getSecondsSearchFile(videoQ)
					< pv->minLengthFractionVideoQ)
		return 0;
	if (pv->minLengthFractionVideoR
			&& lengthR / getSecondsSearchFile(videoR)
					< pv->minLengthFractionVideoR)
		return 0;
	return 1;
}
static void parseMinimumLengthParam(const char *param, struct ParamsVoting *pv) {
	MyTokenizer *tk = my_tokenizer_new(param, ',');
	while (my_tokenizer_hasNext(tk)) {
		const char *token = my_tokenizer_nextToken(tk);
		bool isQ = false, isR = false;
		if (my_string_startsWith_ignorecase(token, "q")) {
			isQ = true;
		} else if (my_string_startsWith_ignorecase(token, "r")) {
			isR = true;
		}
		bool isFraction = false, isSeconds = false;
		if (my_string_endsWith_ignorecase(token, "s")) {
			isSeconds = true;
		} else if (my_string_endsWith_ignorecase(token, "f")) {
			isFraction = true;
		} else {
			my_log_error("incorrect format %s\n", token);
		}
		int64_t from = (isQ || isR);
		char *st = my_subStringI_fromTo(token, from, strlen(token) - 1);
		double val = isSeconds ? my_parse_seconds(st) : my_parse_fraction(st);
		MY_FREE(st);
		if (!isQ && !isR)
			isQ = isR = true;
		if (isSeconds && isQ)
			pv->minLengthSecondsQ = val;
		if (isSeconds && isR)
			pv->minLengthSecondsR = val;
		if (isFraction && isQ)
			pv->minLengthFractionVideoQ = val;
		if (isFraction && isR)
			pv->minLengthFractionVideoR = val;
	}
	my_tokenizer_release(tk);
}

#define BONUS_FLAWLESS 1.5

void det_reportDetection(struct SearchSegment *desde_q,
		struct SearchSegment *hasta_q, struct SearchSegment *desde_r,
		struct SearchSegment *hasta_r, double score, int64_t numVoters,
		int64_t numMisses, void *stateObj) {
	struct StateDetections *state = stateObj;
	struct ParamsVoting *pv = state->stateGlob->pv;
//discards most false positives
	if (numVoters < numMisses)
		return;
	if (!hasMinimumLength(desde_q->segment->start_second,
			hasta_q->segment->end_second, desde_q->sfile,
			desde_r->segment->start_second, hasta_r->segment->end_second,
			desde_r->sfile, pv))
		return;
//increases score to highly likely detections
	if (numVoters > 10 && numMisses <= 2)
		score *= BONUS_FLAWLESS;
	struct Detection *newDet = MY_MALLOC(1, struct Detection);
	newDet->videoQ = desde_q->sfile;
	newDet->videoR = desde_r->sfile;
	newDet->secStartQ = desde_q->segment->start_second;
	newDet->secEndQ = hasta_q->segment->end_second;
	newDet->secStartR = desde_r->segment->start_second;
	newDet->secEndR = hasta_r->segment->end_second;
	newDet->score = score;
	newDet->numVoters = numVoters;
	newDet->numMisses = numMisses;
	forceSameDetectionLength(newDet);
	if (!hasMinimumLength(newDet->secStartQ, newDet->secEndQ, newDet->videoQ,
			newDet->secStartR, newDet->secEndR, newDet->videoR, pv)) {
		MY_FREE(newDet);
		return;
	}
	MY_MUTEX_LOCK(state->mutex);
//search for identical previous detection
	int64_t i, posToReplace = -1, posMinScore = -1;
	double minScore = DBL_MAX;
	for (i = 0; i < my_vectorObj_size(state->candidates); ++i) {
		struct Detection *c = my_vectorObj_get(state->candidates, i);
		if (isSimilarCandidate(c, newDet, pv->singleDetection)) {
			posToReplace = i;
			break;
		} else if (c->score < minScore) {
			minScore = c->score;
			posMinScore = i;
		}
	}
	if (posToReplace < 0 && pv->maxDetections > 0
			&& my_vectorObj_size(state->candidates) >= pv->maxDetections)
		posToReplace = posMinScore;
	if (posToReplace >= 0) {
		//try to replace an old detection
		struct Detection *c = my_vectorObj_get(state->candidates, posToReplace);
		if (newDet->score > c->score) {
			MY_FREE(c);
			my_vectorObj_set(state->candidates, posToReplace, newDet);
		} else {
			//discarded by score
			MY_FREE(newDet);
		}
	} else {
		//add a new detection
		my_vectorObj_add(state->candidates, newDet);
	}
	MY_MUTEX_UNLOCK(state->mutex);
}

static char *printParameters(bool shortVersion, struct ParamsVoting *pv) {
	if (shortVersion)
		return my_newString_format(
				"maxd_%s,maxnn_%s,maxd_%s,step_%s,wrank_%s,match_%s,miss_%s,mt_%s,minlen_%s,wdist_%s,single_%i",
				pv->maxDetections_st, pv->maxNNLoad_st, pv->maxDistLoad_st,
				pv->stepSize_st, pv->rankWeight_st, pv->matchVote_st,
				pv->missCost_st, pv->matchType_st, pv->minimumLength_st,
				pv->useDistWeight_st, pv->singleDetection);
	return my_newString_format(
			"maxDetections=%s maxNNLoad=%s maxDistLoad=%s stepSize=%s rankWeight=%s matchVote=%s missCost=%s matchType=%s minLength=%s useDistHistogram=%s singleDetection=%s",
			pv->maxDetections_st, pv->maxNNLoad_st, pv->maxDistLoad_st,
			pv->stepSize_st, pv->rankWeight_st, pv->matchVote_st,
			pv->missCost_st, pv->matchType_st, pv->minimumLength_st,
			pv->useDistWeight_st, pv->singleDetection_st);
}
static FILE *det_createOutputFile(struct StateDetectionsGlobal *stateGlob) {
	if (stateGlob->filenameOut == NULL) {
		char *dir = my_io_getDirname(stateGlob->filenameIn);
		char *name = my_io_getFilename(stateGlob->filenameIn);
		char *id = printParameters(1, stateGlob->pv);
		stateGlob->filenameOut = my_newString_format("%s/det,%s-%s", dir, id,
				name);
		MY_FREE_MULTI(dir, name, id);
		if (!my_string_endsWith(stateGlob->filenameOut, ".txt")) {
			char *st = my_newString_concat(stateGlob->filenameOut, ".txt");
			MY_FREE(stateGlob->filenameOut);
			stateGlob->filenameOut = st;
		}
		my_log_info("created file %s\n", stateGlob->filenameOut);
	}
	char *id2 = printParameters(0, stateGlob->pv);
	FILE *out = createDetectionsFile(stateGlob->filenameOut,
			stateGlob->cmd_params, id2);
	MY_FREE(id2);
	return out;
}

static int64_t locateMaximumScorePos(MyVectorObj *candidates) {
	double maxScore = 0;
	int64_t i, maxPos = -1;
	for (i = 0; i < my_vectorObj_size(candidates); ++i) {
		struct Detection *c = my_vectorObj_get(candidates, i);
		if (c->score > maxScore) {
			maxScore = c->score;
			maxPos = i;
		}
	}
	return maxPos;
}
#define BONUS_FIRST 1.5

static void det_detect_thread(int64_t currentProcess, void* stateg,
		int64_t numThread) {
	struct StateDetectionsGlobal *stateGlob = stateg;
	struct D_Query *queryVideo = my_vectorObj_get(stateGlob->lista,
			currentProcess);
	struct StateDetections *state = MY_MALLOC(1, struct StateDetections);
	state->stateGlob = stateGlob;
	state->candidates = my_vectorObj_new();
	MY_MUTEX_INIT(state->mutex);
	det_detectByVoting(queryVideo, stateGlob->pv, NUM_CORES, state);
	if (my_vectorObj_size(state->candidates) > 0) {
		int64_t pos = locateMaximumScorePos(state->candidates);
		struct Detection *bestDet = my_vectorObj_get(state->candidates, pos);
		bestDet->score *= BONUS_FIRST;
	}
	MY_MUTEX_LOCK(stateGlob->mutex);
	if (stateGlob->out == NULL)
		stateGlob->out = det_createOutputFile(stateGlob);
	printDetectionsToFile(stateGlob->out, state->candidates,
			state->stateGlob->pv->maxDetections);
	my_log_info("%"PRIi64" copies detected in %s\n",
			my_vectorObj_size(state->candidates), queryVideo->sfileQuery->name);
	MY_MUTEX_UNLOCK(stateGlob->mutex);
	my_vectorObj_release(state->candidates, 1);
	MY_MUTEX_DESTROY(state->mutex);
	MY_FREE(state);
}
void loc_new_copyDetection(CmdParams *cmd_params, char *argOption) {
	struct ParamsVoting *pv = MY_MALLOC(1, struct ParamsVoting);
	pv->maxNNLoad_st = "0";
	pv->maxDistLoad_st = "0";
	pv->maxDetections_st = "0";
	pv->matchType_st = "1";
	pv->rankWeight_st = "0.75";
	pv->matchVote_st = "1";
	pv->missCost_st = "-0.1";
	pv->stepSize_st = "0.25";
	pv->minimumLength_st = "1s";
	pv->singleDetection_st = "SegmentQ_SegmentR";
	pv->useDistWeight_st = "0";
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"  -ss ssFile.txt       Mandatory. The output of a similarity search.\n");
		my_log_info(
				"  -maxDetections num   Optional. Max detections per query video. (0=unlimited). default=%s\n",
				pv->maxDetections_st);
		my_log_info(
				"  -minLength (q|r|)[value](s|f)[,]+   Optional. Minimum copy length. types: s=seconds, f=fraction, q=query, r=reference. ej: q0.6f=copy length must be at least 60%% of query video. default=%s\n",
				pv->minimumLength_st);
		my_log_info(
				"  -maxNNLoad num       Optional. Maximum number of NN to load (0=unlimited). default=%s\n",
				pv->maxNNLoad_st);
		my_log_info(
				"  -maxDistLoad dist    Optional. Maximum NN distance to load (0=unlimited). default=%s\n",
				pv->maxDistLoad_st);
		my_log_info("  -rankWeight factor   default=%s\n", pv->rankWeight_st);
		my_log_info("  -stepSize secs       default=%s\n", pv->stepSize_st);
		my_log_info("  -matchVote num       default=%s\n", pv->matchVote_st);
		my_log_info("  -missCost num        default=%s\n", pv->missCost_st);
		my_log_info("  -matchType (0|1|2)   default=%s\n", pv->matchType_st);
		my_log_info("  -useDistWeight       Requires a histogram Q_R.\n");
		my_log_info(
				"  -singleDetection (VideoQ|SegmentQ)_(AnyR|VideoR|SegmentR)    Unique detection between Q and R. default=%s\n",
				pv->singleDetection_st);
		my_log_info(
				"  -out filename        Optional. Set filename out. Default value depends on parameters.\n");
		return;
	}
	struct StateDetectionsGlobal *stateGlob = MY_MALLOC(1,
			struct StateDetectionsGlobal);
	MY_MUTEX_INIT(stateGlob->mutex);
	stateGlob->pv = pv;
	stateGlob->cmd_params = cmd_params;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-ss")) {
			stateGlob->filenameIn = nextParam(cmd_params);
			my_assert_fileExists(stateGlob->filenameIn);
		} else if (isNextParam(cmd_params, "-maxNNLoad")) {
			pv->maxNNLoad_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-maxDistLoad")) {
			pv->maxDistLoad_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-maxDetections")) {
			pv->maxDetections_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-singleDetection")) {
			pv->singleDetection_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-minLength")) {
			pv->minimumLength_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-rankWeight")) {
			pv->rankWeight_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-useDistWeight")) {
			pv->useDistWeight_st = "1";
		} else if (isNextParam(cmd_params, "-stepSize")) {
			pv->stepSize_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-matchVote")) {
			pv->matchVote_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-missCost")) {
			pv->missCost_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-matchType")) {
			pv->matchType_st = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-out")) {
			stateGlob->filenameOut = nextParam(cmd_params);
		} else
			my_log_error("unknown parameter '%s'\n", nextParam(cmd_params));
	}
	pv->maxNNLoad = my_parse_int(pv->maxNNLoad_st);
	pv->maxDistLoad = my_parse_double(pv->maxDistLoad_st);
	pv->maxDetections = my_parse_int(pv->maxDetections_st);
	pv->matchType = my_parse_int(pv->matchType_st);
	pv->rankWeight = my_parse_fraction(pv->rankWeight_st);
	pv->matchVote = my_parse_fraction(pv->matchVote_st);
	pv->missCost = my_parse_fraction(pv->missCost_st);
	pv->stepSize = my_parse_fraction(pv->stepSize_st);
	parseMinimumLengthParam(pv->minimumLength_st, pv);
	pv->useDistWeight = my_parse_uint8(pv->useDistWeight_st);
	my_assert_notNull("-ss", stateGlob->filenameIn);
	my_assert_greaterDouble("rankWeight", pv->rankWeight, 0);
	my_assert_lessEqualDouble("rankWeight", pv->rankWeight, 1);
	my_assert_greaterDouble("stepSize", pv->stepSize, 0);
	my_assert_lessEqualDouble("missCost", pv->missCost, 0);
	my_assert_greaterDouble("matchVote", pv->matchVote, 0);
	my_assert_greaterEqual_int("matchType", pv->matchType, 0);
	my_assert_lessEqualInt("matchType", pv->matchType, 2);
	if (my_string_equals_ignorecase(pv->singleDetection_st, "VideoQ_AnyR"))
		pv->singleDetection = SIMILAR_VideoQ_AnyR;
	else if (my_string_equals_ignorecase(pv->singleDetection_st,
			"VideoQ_VideoR"))
		pv->singleDetection = SIMILAR_VideoQ_VideoR;
	else if (my_string_equals_ignorecase(pv->singleDetection_st,
			"VideoQ_SegmentR"))
		pv->singleDetection = SIMILAR_VideoQ_SegmentR;
	else if (my_string_equals_ignorecase(pv->singleDetection_st,
			"SegmentQ_AnyR"))
		pv->singleDetection = SIMILAR_SegmentQ_AnyR;
	else if (my_string_equals_ignorecase(pv->singleDetection_st,
			"SegmentQ_VideoR"))
		pv->singleDetection = SIMILAR_SegmentQ_VideoR;
	else if (my_string_equals_ignorecase(pv->singleDetection_st,
			"SegmentQ_SegmentR"))
		pv->singleDetection = SIMILAR_SegmentQ_SegmentR;
	else
		my_log_error("unknown single detection %s\n", pv->singleDetection_st);
	struct SearchProfile *profile = loadProfileFromFile(stateGlob->filenameIn);
	if (pv->useDistWeight) {
		char *fname = my_newString_format("%s/%s", profile->path_profile,
				"histogram-Q_R.hist");
		pv->dist_hist = mknn_histogram_restore(fname, 1);
		MY_FREE(fname);
	}
	set_logfile(profile->path_profile, cmd_params, "");
	struct ArchivoFrames *af = loadFileSS(stateGlob->filenameIn, pv->maxNNLoad,
			pv->maxDistLoad, profile);
	stateGlob->lista = af->allQueries;
	my_parallel_incremental(my_vectorObj_size(stateGlob->lista), stateGlob,
			det_detect_thread, "detection", 1);
	if (stateGlob->out != NULL)
		fclose(stateGlob->out);
}
