/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_EVALUATION_EVAL_HPP
#define MKNN_EVALUATION_EVAL_HPP

#include <map>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include "../metricknn_impl.hpp"

namespace mknn {
class MknnQueryEvaluation;
class MknnCategoryEvaluation;

template<typename TypeQuery, typename TypeCategory, typename TypeObject>
class MknnEvaluation {
public:
	MknnEvaluation(MknnGroundTruth<TypeQuery, TypeCategory, TypeObject> &gt);
	void evaluate(MknnAnswers<TypeQuery, TypeObject> &ans, bool with_details);
	void print(std::string filename_output, std::string file_header_text);
	double getMAP();
private:
	void evaluate_query(MknnQueryEvaluation &res,
			MknnAnswers<TypeQuery, TypeObject> &ans, TypeQuery id_query,
			bool with_details);
	void evaluate_category(MknnCategoryEvaluation &res,
			TypeCategory id_category);

	MknnGroundTruth<TypeQuery, TypeCategory, TypeObject> &gt;
	long long max_rank;
	long long expected_corrects;
	std::vector<long long> corrects_at_rank;
	std::vector<double> precision_at_rank;
	double mean_average_precision, mean_average_rank;
	double average_precision_first, average_rank_first;
	std::map<TypeQuery, MknnQueryEvaluation> evaluationByQuery;
	std::map<TypeCategory, MknnCategoryEvaluation> evaluationByCategory;
};

class MknnQueryEvaluation {
public:
	MknnQueryEvaluation();
private:
	long long num_categories_expected;
	long long num_categories_found;
	long long num_categories_missed;
	long long num_objects_expected;
	long long num_objects_found;
	long long num_objects_missed;
	double average_precision, average_rank;
	double precision_first, rank_first;
	std::string summary;

	template<typename A, typename B, typename C>
	friend class MknnEvaluation;
};
class MknnCategoryEvaluation {
public:
	MknnCategoryEvaluation();
private:
	long long num_queries;
	long long num_objects_expected;
	double queries_mean_average_precision, queries_mean_average_rank;
	double queries_average_precision_first, queries_average_rank_first;

	template<typename A, typename B, typename C>
	friend class MknnEvaluation;
};

typedef MknnEvaluation<std::string, std::string, std::string> MknnEvaluation_s;

}
#endif
