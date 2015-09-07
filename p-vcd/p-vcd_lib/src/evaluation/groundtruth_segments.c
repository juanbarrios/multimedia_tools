/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "groundtruth_segments.h"

struct GT_Segment {
	double seg_ini_q, seg_end_q, seg_ini_r, seg_end_r;
	int64_t idSegment;
	struct SearchFile *arcRef;
};
struct GroundTruthSegments {
	struct SearchCollection *colQuery, *colReference;
	int64_t numSegments;
	//each Array_obj contains struct GT_Segment*
	MyVectorObj **segments;
};

static MyVectorObj *searchVideosWithName(struct SearchCollection *col,
		char *videoname) {
	int64_t i;
	MyVectorObj *ret = my_vectorObj_new();
	for (i = 0; i < col->numFiles; ++i) {
		struct SearchFile *arc = col->sfiles[i];
		FileDB *fdb = arc->fdb;
		char *stBase1 = my_subStringC_lastEnd(fdb->filenameReal, '/');
		char *stBase2 = my_subStringC_startLast(stBase1, '.');
		char *stVid1 = my_subStringC_lastEnd(arc->name, '/');
		char *stVid2 = my_subStringC_startLast(stVid1, '.');
		if (my_string_equals_ignorecase(videoname, stBase1)
				|| my_string_equals_ignorecase(videoname, stBase2)
				|| my_string_equals_ignorecase(videoname, stVid1)
				|| my_string_equals_ignorecase(videoname, stVid2))
			my_vectorObj_add(ret, arc);
		MY_FREE_MULTI(stBase1, stBase2, stVid1, stVid2);
	}
	return ret;
}

static struct GroundTruthSegments *loadSegments_from_file(
		const char *file_groundTruth, struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	my_log_info("reading %s\n", file_groundTruth);
	struct GroundTruthSegments *gts = MY_MALLOC(1, struct GroundTruthSegments);
	gts->colQuery = colQuery;
	gts->colReference = colReference;
	gts->segments = MY_MALLOC(gts->colQuery->numFiles, MyVectorObj*);
	MyVectorString *lines = my_io_loadLinesFileConfig(file_groundTruth, "PVCD",
			"GroundTruthSegments", 1, 0);
	int64_t idSegment = 0;
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *videoname_q = my_newString_string(my_tokenizer_nextToken(tk));
		double seg_ini_q = my_tokenizer_nextFraction(tk);
		double seg_end_q = my_tokenizer_nextFraction(tk);
		char *videoname_r = my_newString_string(my_tokenizer_nextToken(tk));
		double seg_ini_r = my_tokenizer_nextFraction(tk);
		double seg_end_r = my_tokenizer_nextFraction(tk);
		my_tokenizer_releaseValidateEnd(tk);
		MyVectorObj *queries = searchVideosWithName(colQuery, videoname_q);
		MyVectorObj *originals = searchVideosWithName(colReference,
				videoname_r);
		for (int64_t i = 0; i < my_vectorObj_size(queries); ++i) {
			struct SearchFile *arcQuery = my_vectorObj_get(queries, i);
			FileDB *fdbq = arcQuery->fdb;
			double iniq = seg_ini_q - fdbq->secStartTime;
			double endq = seg_end_q - fdbq->secStartTime;
			if (!MY_INTERSECT(iniq, endq, 0, fdbq->lengthSec))
				continue;
			for (int64_t j = 0; j < my_vectorObj_size(originals); ++j) {
				struct SearchFile *arcRef = my_vectorObj_get(originals, j);
				FileDB *fdbr = arcRef->fdb;
				double inir = seg_ini_r - fdbr->secStartTime;
				double endr = seg_end_r - fdbr->secStartTime;
				if (!MY_INTERSECT(inir, endr, 0, fdbr->lengthSec))
					continue;
				struct GT_Segment *s = MY_MALLOC(1, struct GT_Segment);
				s->seg_ini_q = iniq;
				s->seg_end_q = endq;
				s->seg_ini_r = inir;
				s->seg_end_r = endr;
				s->arcRef = arcRef;
				s->idSegment = idSegment;
				if (gts->segments[arcQuery->id_seq] == NULL)
					gts->segments[arcQuery->id_seq] = my_vectorObj_new();
				my_vectorObj_add(gts->segments[arcQuery->id_seq], s);
			}
			idSegment++;
		}
	}
	gts->numSegments = idSegment;
	my_vectorString_release(lines, true);
	return gts;
}
struct GroundTruthSegments *newGroundTruthSegments(const char *file_groundTruth,
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	struct GroundTruthSegments *gts = loadSegments_from_file(file_groundTruth,
			colQuery, colReference);
	my_log_info("ground truth loaded with %"PRIi64" segments\n",
			gts->numSegments);
	return gts;
}
void releaseGroundTruthSegments(struct GroundTruthSegments *gts) {
	if (gts == NULL)
		return;
	for (int i = 0; i < gts->colQuery->numFiles; ++i) {
		MyVectorObj *segms = gts->segments[i];
		if (segms != NULL)
			my_vectorObj_release(segms, 1);
	}
	MY_FREE_MULTI(gts->segments, gts);
}

//-1 => incorrect
//>=0 => correct
int64_t groundtruth_correctSegment(struct GroundTruthSegments *gts,
		struct SearchFile *arcQuery, double seg_ini_q, double seg_end_q,
		struct SearchFile *arcRef, double seg_ini_r, double seg_end_r) {
	my_assert_indexRangeInt("id query video", arcQuery->id_seq,
			gts->colQuery->numFiles);
	my_assert_indexRangeInt("id ref video", arcRef->id_seq,
			gts->colReference->numFiles);
	MyVectorObj *segms = gts->segments[arcQuery->id_seq];
	if (segms == NULL)
		return -1;
	for (int64_t i = 0; i < my_vectorObj_size(segms); ++i) {
		struct GT_Segment *segm = my_vectorObj_get(segms, i);
		if (arcRef == segm->arcRef //
				&& MY_INTERSECT(segm->seg_ini_q, segm->seg_end_q, seg_ini_q, seg_end_q)
				&& MY_INTERSECT(segm->seg_ini_r, segm->seg_end_r, seg_ini_r, seg_end_r)) {
			return segm->idSegment;
		} else if (segm->arcRef == arcRef) {
			my_log_info("detection almost correct %s %s\n", arcQuery->name,
					arcRef->name);
		}
	}
	return -1;
}

static double expectedSegR(struct GT_Segment *segm, double seg_q) {
	double offset_ini = segm->seg_ini_r - segm->seg_ini_q;
	double offset_end = segm->seg_end_r - segm->seg_end_q;
	double offset_rate = (offset_end - offset_ini)
			/ (segm->seg_end_q - segm->seg_ini_q);
	double offset = offset_ini + offset_rate * (seg_q - segm->seg_ini_q);
	return seg_q + offset;
}
static char *expectedNumFrameR(struct GT_Segment *segm, double seg_q) {
	double seg_r = expectedSegR(segm, seg_q);
	double fps_r = segm->arcRef->seg->video_fps;
	int64_t numFrame = my_math_round_int(seg_r * fps_r);
	return my_newString_int(numFrame);
}

#define DEBUG_GT 0
static void processSegm(struct GroundTruthFrames *gtf,
		struct SearchFile *arcQuery, struct GT_Segment *segm) {
	for (int64_t i = 0; i < arcQuery->numSegments; ++i) {
		struct SearchSegment *kfq = arcQuery->ssegments[i];
		if (!MY_BETWEEN(kfq->segment->selected_second, segm->seg_ini_q,
				segm->seg_end_q))
			continue;
		double time_r = expectedSegR(segm, kfq->segment->selected_second);
		for (int64_t j = 0; j < segm->arcRef->numSegments; ++j) {
			struct SearchSegment *kfr = segm->arcRef->ssegments[j];
			if (!MY_BETWEEN(time_r, kfr->segment->start_second,
					kfr->segment->end_second))
				continue;
			GroundTruthFrames_add(gtf, kfq, kfr);
			if (DEBUG_GT) {
				char *stQ = toString_SearchSegmentAndFile(kfq);
				char *stR = toString_SearchSegmentAndFile(kfr);
				char *st1 = expectedNumFrameR(segm,
						kfq->segment->selected_second);
				char *st2 = expectedNumFrameR(segm, kfq->segment->start_second);
				char *st3 = expectedNumFrameR(segm, kfq->segment->end_second);
				my_log_info("%s => %s   expected @%s(%s-%s)\n", stQ, stR, st1,
						st2, st3);
				MY_FREE_MULTI(stQ, stR, st1, st2, st3);
			}
		}
	}
}

struct GroundTruthFrames* newGroundTruthFrames_segms(
		struct GroundTruthSegments *gts) {
	struct GroundTruthFrames *gtf = newGroundTruthFramesEmpty(gts->colQuery,
			gts->colReference);
	for (int64_t i = 0; i < gts->colQuery->numFiles; ++i) {
		MyVectorObj *sgms = gts->segments[i];
		if (sgms == NULL)
			continue;
		struct SearchFile *arcQuery = gts->colQuery->sfiles[i];
		for (int64_t j = 0; j < my_vectorObj_size(sgms); ++j) {
			struct GT_Segment *segm = my_vectorObj_get(sgms, j);
			processSegm(gtf, arcQuery, segm);
		}
	}
	return gtf;
}
