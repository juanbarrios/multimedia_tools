/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

struct V_VideoOffset {
	struct SearchFile *videoR;
	double offset_min, offset_max;
	int64_t numVoters;
};

static double det_voteNN(struct SearchSegment *kfQuery,
		struct SearchSegment *kfRef, double offset_min, double offset_max,
		struct ParamsVoting *pv) {
	double startMin = kfQuery->segment->start_second + offset_min;
	double startMax = kfQuery->segment->start_second + offset_max;
	double endMin = kfQuery->segment->end_second + offset_min;
	double endMax = kfQuery->segment->end_second + offset_max;
	double refMin = kfRef->segment->start_second;
	double refMax = kfRef->segment->end_second;
	switch (pv->matchType) {
	case 0: //stricter
		if (MY_BETWEEN(refMin, startMin,
				startMax) && MY_BETWEEN(refMax, endMin, endMax))
			return pv->matchVote;
		break;
	case 1:
		if (MY_INTERSECT(startMin, endMax, refMin, refMax))
			return pv->matchVote;
		break;
	case 2:
		if (MY_INTERSECT(startMin, endMax, refMin, refMax))
			return pv->matchVote;
		double fd1_m = startMin - pv->stepSize;
		double fh2_m = endMax + pv->stepSize;
		if (MY_INTERSECT(fd1_m, fh2_m, refMin, refMax))
			return pv->matchVote / 2;
		break;
	}
	return pv->missCost;
}
static double det_voteSegment(MyMapIntObj *mapVotes, struct D_Frame *frame,
		struct V_VideoOffset *probe, struct ParamsVoting *pv,
		struct SearchSegment **nn_voto) {
	*nn_voto = NULL;
	double max_vote = pv->missCost;
	double weight_rank = 1.0;
	//double maxDist = frame->query->af->max_dist;
	struct SearchSegment *kfQuery = frame->ssegmentQuery;
	int i;
	for (i = 0; i < frame->numNNs; ++i) {
		struct D_Match *nn = frame->nns[i];
		struct SearchSegment *kfRef = nn->ssegmentRef;
		if ((kfRef->sfile == probe->videoR)
				&& my_mapIntObj_get(mapVotes, kfRef->segment->selected_frame)
						== NULL) {
			double v = det_voteNN(kfQuery, kfRef, probe->offset_min,
					probe->offset_max, pv);
			if (v > 0) {
				double weight_dist = 1;
				if (pv->dist_hist != NULL) {
					//double dist = nn->distancia / maxDist;
					double alpha = mknn_histogram_getQuantile(pv->dist_hist, nn->distance);
					weight_dist = 1 - alpha;
					if (weight_dist < 0)
						weight_dist = -weight_dist;
				}
				v *= weight_dist * weight_rank;
				if (v > max_vote) {
					max_vote = v;
					*nn_voto = kfRef;
				}
			}
		}
		weight_rank *= pv->rankWeight;
	}
	return max_vote;
}
struct V_Match {
	struct SearchSegment *desde_q, *hasta_q;
	struct SearchSegment *desde_r, *hasta_r;
	double score;
	int64_t numVotantes, numSinVotos;
};
struct V_Matcher {
	struct V_Match currentMatch, bestMatch;
	MyMapIntObj *mapVotes;
};

static void addVote(struct V_Matcher *m, struct SearchSegment *kfQuery,
		struct SearchSegment *nn_voto, double voto) {
	//add vote to currentMatch
	if (m->currentMatch.desde_q == NULL)
		m->currentMatch.desde_q = kfQuery;
	m->currentMatch.hasta_q = kfQuery;
	if ((m->currentMatch.desde_r == NULL)
			|| (nn_voto->segment->start_frame
					< m->currentMatch.desde_r->segment->start_frame))
		m->currentMatch.desde_r = nn_voto;
	if ((m->currentMatch.hasta_r == NULL)
			|| (nn_voto->segment->end_frame
					> m->currentMatch.hasta_r->segment->end_frame))
		m->currentMatch.hasta_r = nn_voto;
	m->currentMatch.score += voto;
	m->currentMatch.numVotantes++;
	//register voter to avoid double voting
	my_mapIntObj_add(m->mapVotes, nn_voto->segment->selected_frame, &m);
	//save current match if it has higher score
	if (m->currentMatch.score >= m->bestMatch.score) {
		m->bestMatch.desde_q = m->currentMatch.desde_q;
		m->bestMatch.hasta_q = m->currentMatch.hasta_q;
		m->bestMatch.desde_r = m->currentMatch.desde_r;
		m->bestMatch.hasta_r = m->currentMatch.hasta_r;
		m->bestMatch.score = m->currentMatch.score;
		m->bestMatch.numVotantes = m->currentMatch.numVotantes;
		m->bestMatch.numSinVotos = m->currentMatch.numSinVotos;
	}
}
static void reportMatch(struct V_Matcher *m, struct ParamsVoting *pv,
		void *stateVot) {
	if (m->bestMatch.score <= 0)
		return;
	det_reportDetection(m->bestMatch.desde_q, m->bestMatch.hasta_q,
			m->bestMatch.desde_r, m->bestMatch.hasta_r, m->bestMatch.score,
			m->bestMatch.numVotantes, m->bestMatch.numSinVotos, stateVot);
}
static void reinitMatcher(struct V_Matcher *m) {
	if (m->mapVotes != NULL) {
		my_mapIntObj_release(m->mapVotes, false);
		m->mapVotes = NULL;
	}
	if (m->mapVotes == NULL)
		m->mapVotes = my_mapIntObj_new_ABB();
	m->bestMatch.desde_q = m->currentMatch.desde_q = NULL;
	m->bestMatch.hasta_q = m->currentMatch.hasta_q = NULL;
	m->bestMatch.desde_r = m->currentMatch.desde_r = NULL;
	m->bestMatch.hasta_r = m->currentMatch.hasta_r = NULL;
	m->bestMatch.score = m->currentMatch.score = 0;
	m->bestMatch.numVotantes = m->currentMatch.numVotantes = 0;
	m->bestMatch.numSinVotos = m->currentMatch.numSinVotos = 0;
}
static void det_testOffsetInVideo(struct D_Query* queryVideo,
		struct V_VideoOffset *probe, struct ParamsVoting *pv, void *stateVot) {
	struct V_Matcher *m = MY_MALLOC(1, struct V_Matcher);
	reinitMatcher(m);
	int64_t i;
	for (i = 0; i < queryVideo->numFrames; ++i) {
		struct D_Frame *frame = queryVideo->frames[i];
		struct SearchSegment *nn_voto = NULL;
		double voto = det_voteSegment(m->mapVotes, frame, probe, pv, &nn_voto);
		if (voto > 0 && nn_voto != NULL) {
			addVote(m, frame->ssegmentQuery, nn_voto, voto);
		} else if (m->currentMatch.score > 0) {
			m->currentMatch.score += voto;
			m->currentMatch.numSinVotos++;
			if (m->currentMatch.score < 0) {
				reportMatch(m, pv, stateVot);
				reinitMatcher(m);
			}
		}
	}
	reportMatch(m, pv, stateVot);
	my_mapIntObj_release(m->mapVotes, false);
	MY_FREE(m);
}

static void det_contabilizar_offset(struct SearchFile *videoR, double ofMin,
		double ofMax, MyVectorObj ***ptr_listPerVideoR, int64_t *ptr_lengthList) {
	if (*ptr_listPerVideoR == NULL) {
		*ptr_listPerVideoR = MY_MALLOC(videoR->col->numFiles, MyVectorObj *);
		*ptr_lengthList = videoR->col->numFiles;
	}
	my_assert_indexRangeInt("id", videoR->id_seq, *ptr_lengthList);
	MyVectorObj **listPerVideoR = *ptr_listPerVideoR;
	if (listPerVideoR[videoR->id_seq] == NULL)
		listPerVideoR[videoR->id_seq] = my_vectorObj_new();
	MyVectorObj *vs = listPerVideoR[videoR->id_seq];
	int64_t i, numVoters = 1;
	for (i = 0; i < my_vectorObj_size(vs); ++i) {
		struct V_VideoOffset *desf = my_vectorObj_get(vs, i);
		if (desf->videoR != videoR)
			my_log_error("error in videoR\n");
		if (MY_INTERSECT(desf->offset_min, desf->offset_max, ofMin, ofMax)) {
			ofMin = MIN(desf->offset_min, ofMin);
			ofMax = MAX(desf->offset_max, ofMax);
			numVoters += desf->numVoters;
			my_vectorObj_remove(vs, i);
			i = -1;
		}
	}
	struct V_VideoOffset *desf = MY_MALLOC(1, struct V_VideoOffset);
	desf->videoR = videoR;
	desf->offset_min = ofMin;
	desf->offset_max = ofMax;
	desf->numVoters = numVoters;
	my_vectorObj_add(vs, desf);
}

#define INVALID_OFFSET_SAME_VIDEO 10*60

static void det_gatherVideoOffsets(struct D_Query* queryVideo,
		struct ParamsVoting *pv, MyVectorObj ***ptr_listPerVideoR,
		int64_t *ptr_lengthList) {
	int64_t i, j;
	for (i = 0; i < queryVideo->numFrames; ++i) {
		struct D_Frame *frameQ = queryVideo->frames[i];
		struct VideoSegment *segmentQ = frameQ->ssegmentQuery->segment;
		struct SearchFile *fileQ = frameQ->ssegmentQuery->sfile;
		for (j = 0; j < frameQ->numNNs; ++j) {
			struct D_Match *nn = frameQ->nns[j];
			struct 	VideoSegment *segmentR = nn->ssegmentRef->segment;
			struct SearchFile *fileR = nn->ssegmentRef->sfile;
			double of1 = segmentR->start_second - segmentQ->start_second;
			double of2 = segmentR->start_second - segmentQ->end_second;
			double of3 = segmentR->end_second - segmentQ->start_second;
			double of4 = segmentR->end_second - segmentQ->end_second;
			double ofMin = MIN(MIN(of1,of2),MIN(of3,of4)) - pv->stepSize;
			double ofMax = MAX(MAX(of1,of2),MAX(of3,of4)) + pv->stepSize;
			if ((fileQ == fileR)
					&& MY_INTERSECT(ofMin, ofMax, -INVALID_OFFSET_SAME_VIDEO,
							INVALID_OFFSET_SAME_VIDEO)) {
				continue;
			}
			det_contabilizar_offset(fileR, ofMin, ofMax, ptr_listPerVideoR,
					ptr_lengthList);
		}
	}
}
struct Options {
	MyVectorObj* probes;
	struct D_Query* queryVideo;
	struct ParamsVoting *pv;
	void *stateVot;
};
static void det_voting_thread(int64_t currentProcess, void* optptr, int64_t numThread) {
	struct Options *opt = optptr;
	struct V_VideoOffset *probe = my_vectorObj_get(opt->probes, currentProcess);
	det_testOffsetInVideo(opt->queryVideo, probe, opt->pv, opt->stateVot);
}
void det_detectByVoting(void *qqueryVideo, struct ParamsVoting *pv,
		int64_t numThreads, void *stateVot) {
	struct D_Query *queryVideo = qqueryVideo;
	MyVectorObj **listPerVideoR = NULL;
	int64_t lengthList = 0;
	det_gatherVideoOffsets(queryVideo, pv, &listPerVideoR, &lengthList);
	MyVectorObj* probes = my_vectorObj_new();
	int64_t k;
	for (k = 0; k < lengthList; ++k) {
		MyVectorObj* vs = listPerVideoR[k];
		if (vs == NULL)
			continue;
		int64_t i;
		for (i = 0; i < my_vectorObj_size(vs); ++i) {
			struct V_VideoOffset *desf = my_vectorObj_get(vs, i);
			if (desf->numVoters < 6)
				continue;
			double start, len = pv->stepSize;
			for (start = desf->offset_min - len / 2.0; start < desf->offset_max;
					start += len) {
				struct V_VideoOffset *probe = MY_MALLOC(1,
						struct V_VideoOffset);
				probe->videoR = desf->videoR;
				probe->offset_min = start;
				probe->offset_max = start + len;
				my_vectorObj_add(probes, probe);
			}
		}
		my_vectorObj_release(vs, 1);
	}
	MY_FREE(listPerVideoR);
	struct Options opt;
	opt.pv = pv;
	opt.probes = probes;
	opt.queryVideo = queryVideo;
	opt.stateVot = stateVot;
	my_parallel_incremental(my_vectorObj_size(probes), &opt, det_voting_thread,
			"voting", numThreads);
	my_vectorObj_release(probes, 1);
}

