/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

struct VoteCount {
	char *nameFrameR;
	double score;
};
static MyVectorObj* sumVotesFrame(struct QueryTxt *frameTxt) {
	MyMapStringObj *mapVotes = my_mapStringObj_newCaseSensitive();
	for (int64_t i = 0; i < frameTxt->num_queries; ++i) {
		MyVectorObj *nnList = frameTxt->nns[i];
		for (int64_t j = 0; j < my_vectorObj_size(nnList); ++j) {
			struct NNTxt *nn = my_vectorObj_get(nnList, j);
			char *nameFrameR = my_subStringC_startFirst(nn->name_nn, '|');
			struct VoteCount *vc = my_mapStringObj_get(mapVotes, nameFrameR);
			if (vc == NULL) {
				vc = MY_MALLOC(1, struct VoteCount);
				vc->nameFrameR = nameFrameR;
				my_mapStringObj_add(mapVotes, nameFrameR, vc);
			} else {
				free(nameFrameR);
			}
			vc->score += pow(0.97, j);
		}
	}
	MyVectorObj *arr = my_mapStringObj_valuesVector(mapVotes);
	//my_mapStringObj_release(mapVotes, false, false);
	return arr;
}
static int qsort_compare_QueryTxt(const void *a, const void *b) {
	struct QueryTxt *o1 = *(struct QueryTxt**) a;
	struct QueryTxt *o2 = *(struct QueryTxt**) b;
	char *st1 = my_subStringC_firstFirst(o1->name_file, '@', '(');
	char *st2 = my_subStringC_firstFirst(o2->name_file, '@', '(');
	int64_t nf1 = my_parse_int(st1);
	int64_t nf2 = my_parse_int(st2);
	MY_FREE_MULTI(st1, st2);
	return my_compare_int(nf1, nf2);
}
struct FrameQ {
	char *nameFrameQ;
	MyVectorObj *nns;
	struct QueryTxt *queryTxt;
};
struct QVideo {
	const char *nameVideoQ;
	double totalSearchTime;
	int64_t numFrames;
	struct FrameQ **framesQ;
};
struct Params {
	int64_t maxNNOut;
	MyVectorObj *videoList;
};
static int compare_VoteScore(const void *a, const void *b) {
	struct VoteCount *o1 = *(struct VoteCount **) a;
	struct VoteCount *o2 = *(struct VoteCount **) b;
	return my_compare_double(o1->score, o2->score);
}
static void mergeFrames(int64_t currentProcess, void* state, int64_t numThread) {
	struct Params *params = state;
	struct QVideo *vid = my_vectorObj_get(params->videoList, currentProcess);
	for (int64_t j = 0; j < vid->numFrames; ++j) {
		struct FrameQ *frm = vid->framesQ[j];
		struct QueryTxt *frameTxt = frm->queryTxt;
		frm->nns = sumVotesFrame(frameTxt);
		for (int64_t k = 0; k < my_vectorObj_size(frm->nns); ++k) {
			struct VoteCount *vc = my_vectorObj_get(frm->nns, k);
			vc->score = (1.0 / vc->score);
		}
		my_vectorObj_qsort(frm->nns, compare_VoteScore);
		while (my_vectorObj_size(frm->nns) > params->maxNNOut) {
			struct VoteCount *vc = my_vectorObj_remove(frm->nns,
					my_vectorObj_size(frm->nns) - 1);
			MY_FREE_MULTI(vc->nameFrameR, vc);
		}
	}
}
static MyVectorObj *groupCommonVideos(MyVectorObj *ssVectorList) {
	MyMapStringObj *namevid2list = my_mapStringObj_newCaseSensitive();
	int64_t j;
	for (j = 0; j < my_vectorObj_size(ssVectorList); ++j) {
		struct QueryTxt *frameQ = my_vectorObj_get(ssVectorList, j);
		char *nameVideoQ = my_subStringC_startFirst(frameQ->name_file, '@');
		MyVectorObj *listTxt = my_mapStringObj_get(namevid2list, nameVideoQ);
		if (listTxt == NULL) {
			listTxt = my_vectorObj_new();
			my_mapStringObj_add(namevid2list, nameVideoQ, listTxt);
		} else
			MY_FREE(nameVideoQ);
		my_vectorObj_add(listTxt, frameQ);
	}
	int64_t totFrames = 0;
	MyVectorObj *listVideos = my_vectorObj_new();
	for (j = 0; j < my_mapStringObj_size(namevid2list); ++j) {
		const char *nameVideoQ = my_mapStringObj_getKeyAt(namevid2list, j);
		MyVectorObj *listTxt = my_mapStringObj_getObjAt(namevid2list, j);
		my_vectorObj_qsort(listTxt, qsort_compare_QueryTxt);
		struct QVideo *vid = MY_MALLOC(1, struct QVideo);
		vid->nameVideoQ = nameVideoQ;
		vid->numFrames = my_vectorObj_size(listTxt);
		vid->framesQ = MY_MALLOC(vid->numFrames, struct FrameQ*);
		int64_t k;
		for (k = 0; k < vid->numFrames; ++k) {
			struct QueryTxt *queryTxt = my_vectorObj_get(listTxt, k);
			vid->totalSearchTime += queryTxt->search_time;
			struct FrameQ *frameQ = MY_MALLOC(1, struct FrameQ);
			frameQ->queryTxt = queryTxt;
			int64_t pos = my_string_indexOf(queryTxt->name_file, "@");
			frameQ->nameFrameQ =
					(pos < 0) ?
							my_newString_string("") :
							my_subStringI_fromEnd(queryTxt->name_file, pos + 1);
			vid->framesQ[k] = frameQ;
		}
		my_vectorObj_release(listTxt, 0);
		totFrames += vid->numFrames;
		my_vectorObj_add(listVideos, vid);
	}
	my_log_info("%"PRIi64" videos and %"PRIi64" frames\n",
			my_vectorObj_size(listVideos), totFrames);
	my_mapStringObj_release(namevid2list, false, false);
	return listVideos;
}
int pvcd_mergeLocalToGlobal(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s\n", getBinaryName(cmd_params));
		my_log_info("  -ss filenameSsVector        Mandatory.\n");
		my_log_info("  [-maxVectorsIn num]         Optional.\n");
		my_log_info("  [-maxNNOut num]             Optional.\n");
		my_log_info("  -out newFilename            Mandatory.\n");
		return pvcd_system_exit_error();
	}
	int64_t maxVectorsIn = INT64_MAX, maxNNOut = INT64_MAX;
	const char *inFilename = NULL, *outFilename = NULL;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-ss")) {
			inFilename = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-maxVectorsIn")) {
			maxVectorsIn = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-maxNNOut")) {
			maxNNOut = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-out")) {
			outFilename = nextParam(cmd_params);
		} else {
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
		}
	}
	my_assert_notNull("in", inFilename);
	my_assert_notNull("out", outFilename);
	MyVectorObj *ssVectorList = loadVectorsFileTxt(inFilename, maxVectorsIn);
	struct Params param = { 0 };
	param.maxNNOut = maxNNOut;
	param.videoList = groupCommonVideos(ssVectorList);
	my_parallel_incremental(my_vectorObj_size(param.videoList), &param,
			mergeFrames, "merge", NUM_CORES);
	FILE *out = my_io_openFileWrite1Config(outFilename, "PVCD", "SS", 1, 1);
	char *stCmd = get_command_line(cmd_params);
	fprintf(out, "#%s\n", stCmd);
	MY_FREE(stCmd);
	int64_t i, j, k;
	for (i = 0; i < my_vectorObj_size(param.videoList); ++i) {
		struct QVideo *vid = my_vectorObj_get(param.videoList, i);
		fprintf(out, "query=%s\tsegments=%"PRIi64"\tsearchTime=%1.1lf\n",
				vid->nameVideoQ, vid->numFrames, vid->totalSearchTime);
		for (j = 0; j < vid->numFrames; ++j) {
			struct FrameQ *frameQ = vid->framesQ[j];
			fprintf(out, "%s", frameQ->nameFrameQ);
			for (k = 0; k < my_vectorObj_size(frameQ->nns); ++k) {
				struct VoteCount *nn = my_vectorObj_get(frameQ->nns, k);
				char *st = my_newString_double(nn->score);
				fprintf(out, "\t%s\t%s", nn->nameFrameR, st);
				MY_FREE(st);
			}
			if (my_vectorObj_size(frameQ->nns) == 0)
				fprintf(out, "\t");
			fprintf(out, "\n");
		}
	}
	fclose(out);
	return pvcd_system_exit_ok(cmd_params);
}
