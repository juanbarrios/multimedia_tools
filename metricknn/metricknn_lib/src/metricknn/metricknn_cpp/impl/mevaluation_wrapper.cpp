/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include <string>
#include <vector>
#include <map>
#include <set>
#include "../metricknn_impl.hpp"
//#include <myutils/myutils_cpp.hpp>

extern "C" {

MknnGroundTruth_s *mknn_evaluation_gt_newGroundTruth_s() {
	mknn::MknnGroundTruth_s *gt = new mknn::MknnGroundTruth_s();
	return reinterpret_cast<MknnGroundTruth_s*>(gt);
}

void mknn_evaluation_gt_releaseGroundTruth_s(MknnGroundTruth_s *mgt) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	delete gt;
}

void mknn_evaluation_gt_addCorrectAnswer_s(MknnGroundTruth_s *mgt,
		const char *id_query, const char * id_category) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	gt->addCorrectAnswer(std::string((id_query == NULL) ? "" : id_query),
			std::string((id_category == NULL) ? "" : id_category));
}
void mknn_evaluation_gt_addCategoryObject_s(MknnGroundTruth_s *mgt,
		const char * id_category, const char * id_object) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	gt->addCategoryObject(std::string((id_category == NULL) ? "" : id_category),
			std::string((id_object == NULL) ? "" : id_object));
}
int64_t mknn_evaluation_gt_getTotalQueries_s(MknnGroundTruth_s *mgt) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	return gt->getTotalQueries();
}

int64_t mknn_evaluation_gt_getTotalCategories_s(MknnGroundTruth_s *mgt) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	return gt->getTotalCategories();
}

int64_t mknn_evaluation_gt_getTotalObjects_s(MknnGroundTruth_s *mgt) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	return gt->getTotalObjects();
}

void mknn_evaluation_gt_getCategoriesQuery_s(MknnGroundTruth_s *mgt,
		const char * id_query, MyVectorString *list_id_categories) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	std::set<std::string> list;
	gt->getCategoriesQuery(std::string((id_query == NULL) ? "" : id_query),
			list);
	for (auto const &s : list) {
		my_vectorStringConst_add(list_id_categories, s.c_str());
	}
}

void mknn_evaluation_gt_getCategoriesObject_s(MknnGroundTruth_s *mgt,
		const char * id_object, MyVectorString *list_id_categories) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	std::set<std::string> list;
	gt->getCategoriesObject(std::string((id_object == NULL) ? "" : id_object),
			list);
	for (auto const &s : list) {
		my_vectorStringConst_add(list_id_categories, s.c_str());
	}
}

void mknn_evaluation_gt_getObjectsCategory_s(MknnGroundTruth_s *mgt,
		const char * id_category, MyVectorString *list_id_objects) {
	mknn::MknnGroundTruth_s* gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	std::set<std::string> list;
	gt->getObjectsCategory(
			std::string((id_category == NULL) ? "" : id_category), list);
	for (auto const &s : list) {
		my_vectorStringConst_add(list_id_objects, s.c_str());
	}
}

MknnAnswers_s *mknn_evaluation_ans_newAnswers_s() {
	mknn::MknnAnswers_s *ans = new mknn::MknnAnswers_s();
	return reinterpret_cast<MknnAnswers_s*>(ans);
}

void mknn_evaluation_ans_releaseAnswers_s(MknnAnswers_s *mans) {
	mknn::MknnAnswers_s* ans = reinterpret_cast<mknn::MknnAnswers_s*>(mans);
	delete ans;
}

void mknn_evaluation_ans_addAnswer_s(MknnAnswers_s *mans, const char * id_query,
		const char * id_object, long long rank_position) {
	mknn::MknnAnswers_s* ans = reinterpret_cast<mknn::MknnAnswers_s*>(mans);
	ans->addAnswer(std::string((id_query == NULL) ? "" : id_query),
			std::string((id_object == NULL) ? "" : id_object), rank_position);
}

MknnEvaluation_s *mknn_evaluation_newEvaluation_s(MknnGroundTruth_s *mgt) {
	mknn::MknnGroundTruth_s*gt = reinterpret_cast<mknn::MknnGroundTruth_s*>(mgt);
	mknn::MknnEvaluation_s *ev = new mknn::MknnEvaluation_s(*gt);
	return reinterpret_cast<MknnEvaluation_s*>(ev);
}

void mknn_evaluation_releaseEvaluation_s(MknnEvaluation_s *mev) {
	mknn::MknnEvaluation_s *ev = reinterpret_cast<mknn::MknnEvaluation_s *>(mev);
	delete ev;
}

void mknn_evaluation_evaluate_s(MknnEvaluation_s *mev, MknnAnswers_s *mans, bool with_details) {
	mknn::MknnEvaluation_s *ev = reinterpret_cast<mknn::MknnEvaluation_s *>(mev);
	mknn::MknnAnswers_s *ans = reinterpret_cast<mknn::MknnAnswers_s *>(mans);
	ev->evaluate(*ans, with_details);
}
double mknn_evaluation_getMAP_s(MknnEvaluation_s *mev) {
	mknn::MknnEvaluation_s *ev = reinterpret_cast<mknn::MknnEvaluation_s *>(mev);
	return ev->getMAP();
}

void mknn_evaluation_print_s(MknnEvaluation_s *mev,const char *filename_output, const char *file_header_text) {
	mknn::MknnEvaluation_s *ev = reinterpret_cast<mknn::MknnEvaluation_s *>(mev);
	ev->print(std::string((filename_output == NULL) ? "" : filename_output),
			std::string((file_header_text == NULL) ? "" : file_header_text));
}
}

