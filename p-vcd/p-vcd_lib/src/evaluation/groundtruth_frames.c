/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "groundtruth_frames.h"

struct GroundTruthFrames {
	struct SearchCollection *colQuery, *colReference;
	MknnGroundTruth_s *mgt;
};
struct GroundTruthFrames *newGroundTruthFramesEmpty(
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	struct GroundTruthFrames *gtf = MY_MALLOC(1, struct GroundTruthFrames);
	gtf->colQuery = colQuery;
	gtf->colReference = colReference;
	gtf->mgt = mknn_evaluation_gt_newGroundTruth_s();
	return gtf;
}
void GroundTruthFrames_add(struct GroundTruthFrames *gtf,
		struct SearchSegment *kfq, struct SearchSegment *kfr) {
	//int64_t kfq_id = getUIDSearchSegment(kfq);
	//int64_t kfr_id = getUIDSearchSegment(kfr);
	char *kfq_id = toString_SearchSegmentAndFile(kfq);
	char *kfr_id = toString_SearchSegmentAndFile(kfr);
	mknn_evaluation_gt_addCorrectAnswer_s(gtf->mgt, kfq_id, kfr_id);
	mknn_evaluation_gt_addCategoryObject_s(gtf->mgt, kfr_id, kfr_id);
	free(kfq_id);
	free(kfr_id);
}
void GroundTruthFrames_addQueryCategory(struct GroundTruthFrames *gtf,
		struct SearchSegment *kfq, const char *category) {
	//int64_t kfq_id = getUIDSearchSegment(kfq);
	char *kfq_id = toString_SearchSegmentAndFile(kfq);
	mknn_evaluation_gt_addCorrectAnswer_s(gtf->mgt, kfq_id, category);
	free(kfq_id);
}
void GroundTruthFrames_addCategoryObject(struct GroundTruthFrames *gtf,
		const char *category, struct SearchSegment *kfr) {
	//int64_t kfr_id = getUIDSearchSegment(kfr);
	char *kfr_id = toString_SearchSegmentAndFile(kfr);
	mknn_evaluation_gt_addCategoryObject_s(gtf->mgt, category, kfr_id);
	free(kfr_id);
}

MknnEvaluation_s *GroundTruthFrames_evaluateFile(struct GroundTruthFrames *gtf,
		struct ArchivoFrames *af) {
	MknnAnswers_s *ans = mknn_evaluation_ans_newAnswers_s();
	for (int64_t j = 0; j < my_vectorObj_size(af->allQueries); ++j) {
		struct D_Query *qv = my_vectorObj_get(af->allQueries, j);
		for (int64_t k = 0; k < qv->numFrames; ++k) {
			struct D_Frame *qframe = qv->frames[k];
			//int64_t kfq_id = getUIDSearchSegment(qframe->ssegmentQuery);
			char *kfq_id = toString_SearchSegmentAndFile(qframe->ssegmentQuery);
			for (int64_t i = 0; i < qframe->numNNs; ++i) {
				struct D_Match *nn = qframe->nns[i];
				//int64_t kfr_id = getUIDSearchSegment(nn->ssegmentRef);
				char *kfr_id = toString_SearchSegmentAndFile(nn->ssegmentRef);
				mknn_evaluation_ans_addAnswer_s(ans, kfq_id, kfr_id, i);
				free(kfr_id);
			}
			free(kfq_id);
		}
	}
	MknnEvaluation_s *mev = mknn_evaluation_newEvaluation_s(gtf->mgt);
	mknn_evaluation_evaluate_s(mev, ans, true);
	mknn_evaluation_ans_releaseAnswers_s(ans);
	return mev;
}

struct GroundTruthFrames *newGroundTruthFileSegments(
		const char *file_groundTruthSegments, struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	struct GroundTruthSegments *gts = newGroundTruthSegments(
			file_groundTruthSegments, colQuery, colReference);
	struct GroundTruthFrames *gtf = newGroundTruthFrames_segms(gts);
	releaseGroundTruthSegments(gts);
	my_log_info(
			"ground truth loaded: %"PRIi64" queries, %"PRIi64" objects\n",
			mknn_evaluation_gt_getTotalQueries_s(gtf->mgt),
			mknn_evaluation_gt_getTotalObjects_s(gtf->mgt));
	return gtf;
}
struct GroundTruthFrames *newGroundTruthFileFrames(
		const char *file_groundTruthFrames, struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	struct GroundTruthFrames *gtf = newGroundTruthFramesEmpty(colQuery,
			colReference);
	MyVectorString *lines = my_io_loadLinesFileConfig(file_groundTruthFrames,
			"PVCD", "GroundTruthFrames", 1, 0);
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		const char *st = my_tokenizer_nextToken(tk);
		struct SearchSegment *kfq = findSearchSegmentInCollection(gtf->colQuery,
				st, 0);
		if (kfq == NULL)
			my_log_info("cannot locate '%s'\n", st);
		st = my_tokenizer_nextToken(tk);
		struct SearchSegment *kfr = findSearchSegmentInCollection(
				gtf->colReference, st, 0);
		if (kfr == NULL)
			my_log_info("cannot locate '%s'\n", st);
		my_tokenizer_releaseValidateEnd(tk);
		if (kfq != NULL && kfr != NULL) {
			GroundTruthFrames_add(gtf, kfq, kfr);
		}
	}
	my_vectorString_release(lines, true);
	my_log_info(
			"ground truth loaded: %"PRIi64" queries, %"PRIi64" objects\n",
			mknn_evaluation_gt_getTotalQueries_s(gtf->mgt),
			mknn_evaluation_gt_getTotalObjects_s(gtf->mgt));
	return gtf;
}
struct GroundTruthFrames *newGroundTruthFilesFramesCategories(
		const char *file_groundTruthFramesCategories,
		const char *file_groundTruthCategoriesFrames,
		struct SearchCollection *colQuery,
		struct SearchCollection *colReference) {
	MyVectorString * lines = my_io_loadLinesFileConfig(
			file_groundTruthCategoriesFrames, "PVCD",
			"GroundTruthCategoriesFrames", 1, 0);
	//load categories
	struct GroundTruthFrames *gtf = newGroundTruthFramesEmpty(colQuery,
			colReference);
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *category = my_newString_string(my_tokenizer_nextToken(tk));
		const char *st = my_tokenizer_nextToken(tk);
		struct SearchSegment *kfr = findSearchSegmentInCollection(colReference,
				st, 0);
		if (kfr == NULL)
			my_log_info("cannot locate object '%s'\n", st);
		my_tokenizer_releaseValidateEnd(tk);
		if (kfr != NULL)
			GroundTruthFrames_addCategoryObject(gtf, category, kfr);
		free(category);
	}
	my_vectorString_release(lines, true);
	lines = my_io_loadLinesFileConfig(file_groundTruthFramesCategories, "PVCD",
			"GroundTruthFramesCategories", 1, 0);
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		const char *st = my_tokenizer_nextToken(tk);
		struct SearchSegment *kfq = findSearchSegmentInCollection(gtf->colQuery,
				st, 0);
		if (kfq == NULL)
			my_log_info("cannot locate query '%s'\n", st);
		char *category = my_newString_string(my_tokenizer_nextToken(tk));
		my_tokenizer_releaseValidateEnd(tk);
		if (kfq != NULL)
			GroundTruthFrames_addQueryCategory(gtf, kfq, category);
		free(category);
	}
	my_vectorString_release(lines, true);
	my_log_info(
			"ground truth loaded: %"PRIi64" queries, %"PRIi64" categories, %"PRIi64" objects\n",
			mknn_evaluation_gt_getTotalQueries_s(gtf->mgt),
			mknn_evaluation_gt_getTotalCategories_s(gtf->mgt),
			mknn_evaluation_gt_getTotalObjects_s(gtf->mgt));
	return gtf;
}
void releaseGroundTruthFrames(struct GroundTruthFrames *gtf) {
	mknn_evaluation_gt_releaseGroundTruth_s(gtf->mgt);
	free(gtf);
}
