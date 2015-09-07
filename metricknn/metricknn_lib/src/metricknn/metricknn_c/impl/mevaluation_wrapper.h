/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_EVALUATION_WRAPPER_H
#define MKNN_EVALUATION_WRAPPER_H

#include "../metricknn_impl.h"

typedef struct MknnGroundTruth_s MknnGroundTruth_s;
typedef struct MknnAnswers_s MknnAnswers_s;
typedef struct MknnEvaluation_s MknnEvaluation_s;

MknnGroundTruth_s *mknn_evaluation_gt_newGroundTruth_s();

void mknn_evaluation_gt_releaseGroundTruth_s(MknnGroundTruth_s *mgt);

void mknn_evaluation_gt_addCorrectAnswer_s(MknnGroundTruth_s *mgt,
		const char* id_query, const char* id_category);

void mknn_evaluation_gt_addCategoryObject_s(MknnGroundTruth_s *mgt,
		const char* id_category, const char* id_object);

int64_t mknn_evaluation_gt_getTotalQueries_s(MknnGroundTruth_s *mgt);

int64_t mknn_evaluation_gt_getTotalCategories_s(MknnGroundTruth_s *mgt);

int64_t mknn_evaluation_gt_getTotalObjects_s(MknnGroundTruth_s *mgt);

void mknn_evaluation_gt_getCategoriesQuery_s(MknnGroundTruth_s *mgt,
		const char* id_query, MyVectorString *list_id_categories);

void mknn_evaluation_gt_getCategoriesObject_s(MknnGroundTruth_s *mgt,
		const char* id_object, MyVectorString *list_id_categories);

void mknn_evaluation_gt_getObjectsCategory_s(MknnGroundTruth_s *mgt,
		const char* id_category, MyVectorString *list_id_objects);

MknnAnswers_s *mknn_evaluation_ans_newAnswers_s();

void mknn_evaluation_ans_releaseAnswers_s(MknnAnswers_s *ans);

void mknn_evaluation_ans_addAnswer_s(MknnAnswers_s *ans, const char* id_query,
		const char* id_answer, long long rank_answer);

MknnEvaluation_s *mknn_evaluation_newEvaluation_s(MknnGroundTruth_s *mgt);

void mknn_evaluation_releaseEvaluation_s(MknnEvaluation_s *mev);

void mknn_evaluation_evaluate_s(MknnEvaluation_s *mev, MknnAnswers_s *mans, bool with_details);

double mknn_evaluation_getMAP_s(MknnEvaluation_s *mev);

void mknn_evaluation_print_s(MknnEvaluation_s *mev, const char *filename_output, const char *file_header_text);

#endif
