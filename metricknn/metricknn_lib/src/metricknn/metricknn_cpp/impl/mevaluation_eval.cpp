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
#include <iostream>
#include <fstream>
#include "../metricknn_impl.hpp"
#include <myutils/myutils_cpp.hpp>

using namespace mknn;

MknnQueryEvaluation::MknnQueryEvaluation() :
		num_categories_expected(0), num_categories_found(0), num_categories_missed(
				0), num_objects_expected(0), num_objects_found(0), num_objects_missed(
				0), average_precision(0), average_rank(0), precision_first(0), rank_first(
				0), summary("") {
}
MknnCategoryEvaluation::MknnCategoryEvaluation() :
		num_queries(0), num_objects_expected(0), queries_mean_average_precision(
				0), queries_mean_average_rank(0), queries_average_precision_first(
				0), queries_average_rank_first(0) {
}

template<typename TypeQuery, typename TypeCategory, typename TypeObject>
MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::MknnEvaluation(
		MknnGroundTruth<TypeQuery, TypeCategory, TypeObject> &gt) :
		gt(gt), max_rank(0), expected_corrects(0), mean_average_precision(0), mean_average_rank(
				0), average_precision_first(0), average_rank_first(0) {
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::evaluate(
		MknnAnswers<TypeQuery, TypeObject> &ans, bool with_details) {
	this->max_rank = ans.getNumRanks();
	this->corrects_at_rank.clear();
	this->precision_at_rank.clear();
	if (with_details) {
		this->corrects_at_rank.resize(this->max_rank);
		this->precision_at_rank.resize(this->max_rank);
	}
	this->evaluationByQuery.clear();
	for (TypeQuery id_query : ans.getQueryNames()) {
		MknnQueryEvaluation res;
		evaluate_query(res, ans, id_query, with_details);
		if (res.num_objects_expected == 0)
			continue;
		this->expected_corrects += res.num_objects_expected;
		this->mean_average_precision += res.average_precision;
		this->mean_average_rank += res.average_rank;
		this->average_precision_first += res.precision_first;
		this->average_rank_first += res.rank_first;
		this->evaluationByQuery[id_query] = res;
	}
	this->mean_average_precision /= this->evaluationByQuery.size();
	this->mean_average_rank /= this->evaluationByQuery.size();
	this->average_precision_first /= this->evaluationByQuery.size();
	this->average_rank_first /= this->evaluationByQuery.size();
	this->evaluationByCategory.clear();
	for (TypeCategory id_category : this->gt.getCategories()) {
		MknnCategoryEvaluation res;
		evaluate_category(res, id_category);
		if (res.num_objects_expected == 0)
			continue;
		this->evaluationByCategory[id_category] = res;
	}
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::print(
		std::string filename_output, std::string file_header_text) {
	std::cout << "writing " << filename_output << std::endl;
	std::ofstream out_file;
	out_file.open(filename_output);
	if (!out_file.is_open())
		throw std::runtime_error("can't open file " + filename_output);
	if (file_header_text.size() > 0)
		out_file << file_header_text << std::endl;
	out_file << "Mean Average Precision (MAP)\t" << this->mean_average_precision
			<< std::endl;
	out_file << "Mean Average Rank (starting at 1)\t" << this->mean_average_rank
			<< std::endl;
	out_file << "Average Precision First Correct\t"
			<< this->average_precision_first << std::endl;
	out_file << "Average Rank First Correct (starting at 1)\t"
			<< this->average_rank_first << std::endl;
	out_file << "Evaluated Rank (max nearest neighbors)\t" << this->max_rank
			<< std::endl;
	bool print_precision_recall = true;
	bool print_summary_by_query = true;
	bool print_stats_by_query = true;
	bool print_stats_by_category = true;
	bool print_stats_by_rank = true;
	if (print_precision_recall) {
		out_file << std::endl << "RECALL\tPRECISION" << std::endl;
		for (long long i = 0; i < this->max_rank; i += 1) {
			double precision = this->precision_at_rank[i];
			double next_precision =
					(i < this->max_rank - 1) ?
							this->precision_at_rank[i + 1] : -1;
			double recall = this->corrects_at_rank[i]
					/ (double) this->expected_corrects;
			if (precision != next_precision)
				out_file << recall << "\t" << precision << std::endl;
		}
	}
	if (print_summary_by_query) {
		out_file << std::endl << "RESULTS BY QUERY ("
				<< this->evaluationByQuery.size() << ")" << std::endl;
		out_file << "id_query\tcategories\tsummary" << std::endl;
		for (auto const &it : this->evaluationByQuery) {
			const TypeQuery &id_query = it.first;
			std::set<TypeCategory> categories_set;
			gt.getCategoriesQuery(id_query, categories_set);
			std::string categories_txt = my::toString::collection(
					categories_set, "", ",", "");
			const MknnQueryEvaluation &res = it.second;
			out_file << id_query << "\t" << categories_txt << "\t"
					<< res.summary << std::endl;
		}
	}
	if (print_stats_by_query) {
		out_file << std::endl << "STATS BY QUERY (total queries="
				<< this->evaluationByQuery.size() << ")" << std::endl;
		out_file
				<< "id_query\tids_categories\tnum_categories\tnum_objects\taverage_precision\taverage_rank\tprecision_first\trank_first"
				<< std::endl;
		for (auto const &it : this->evaluationByQuery) {
			const TypeQuery &id_query = it.first;
			std::set<TypeCategory> categories_set;
			gt.getCategoriesQuery(id_query, categories_set);
			std::string cat_names = my::toString::collection(categories_set, "",
					",", "");
			const MknnQueryEvaluation &res = it.second;
			out_file << id_query << "\t" << cat_names << "\t"
					<< res.num_categories_expected << "\t"
					<< res.num_objects_expected << "\t" << res.average_precision
					<< "\t" << res.average_rank << "\t" << res.precision_first
					<< "\t" << res.rank_first << std::endl;
		}
	}
	if (print_stats_by_category) {
		out_file << std::endl << "STATS BY CATEGORY (total categories="
				<< this->evaluationByCategory.size() << ")" << std::endl;
		out_file
				<< "id_category\tnum_queries\tnum_objects\tmean_average_precision\tmean_average_rank\taverage_precision_first\taverage_rank_first"
				<< std::endl;
		for (auto const &it : this->evaluationByCategory) {
			const TypeCategory &id_category = it.first;
			const MknnCategoryEvaluation &res = it.second;
			out_file << id_category << "\t" << res.num_queries << "\t"
					<< res.num_objects_expected << "\t"
					<< res.queries_mean_average_precision << "\t"
					<< res.queries_mean_average_rank << "\t"
					<< res.queries_average_precision_first << "\t"
					<< res.queries_average_rank_first << std::endl;
		}
	}
	if (print_stats_by_rank) {
		out_file << std::endl << "STATS BY RANK (total ranks=" << this->max_rank
				<< ")" << std::endl;
		out_file << "RANK\tCORRECTS\tRECALL\tPRECISION" << std::endl;
		for (long long i = 0; i < this->max_rank; i += 1) {
			double recall = this->corrects_at_rank[i]
					/ (double) this->expected_corrects;
			double precision = this->precision_at_rank[i];
			out_file << i << "\t" << this->corrects_at_rank[i] << "\t" << recall
					<< "\t" << precision << std::endl;
		}
	}
	out_file.close();
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
double MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::getMAP() {
	return this->mean_average_precision;
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::evaluate_query(
		MknnQueryEvaluation &res, MknnAnswers<TypeQuery, TypeObject> &ans,
		TypeQuery id_query,
		bool with_details) {
	//expected categories
	std::set<TypeQuery> ids_categories_expected;
	gt.getCategoriesQuery(id_query, ids_categories_expected);
	if (ids_categories_expected.size() == 0)
		return;
	//expected objects
	std::set<TypeObject> ids_objects_expected;
	for (auto const &id_category : ids_categories_expected) {
		std::set<TypeObject> list;
		gt.getObjectsCategory(id_category, list);
		if (list.size() == 0)
			std::cout << "query '" << id_query << "' refers an empty category '"
					<< id_category << "'" << std::endl;
		else
			gt.getObjectsCategory(id_category, ids_objects_expected);
	}
	if (ids_objects_expected.size() == 0)
		return;
	res.summary.clear();
	if (with_details)
		res.summary.resize(this->max_rank, '?');
	//sort answers by rank
	std::vector<MknnRankedAnswer<TypeObject> > answers = ans.getQueryAnswers(
			id_query);
	std::sort(answers.begin(), answers.end());
	//init
	std::set<TypeCategory> ids_categories_found;
	std::set<TypeObject> ids_objects_found;
	double sumRank = 0, sumInvRank = 0;
	for (MknnRankedAnswer<TypeObject> const &rans : answers) {
		bool is_correct = (ids_objects_expected.find(rans.getIdObject())
				!= ids_objects_expected.end());
		bool is_false = !is_correct;
		bool is_duplicated = false;
		if (is_correct) {
			auto size_prev = ids_objects_found.size();
			ids_objects_found.insert(rans.getIdObject());
			is_duplicated = (ids_objects_found.size() == size_prev);
			if (is_duplicated)
				is_correct = false;
		}
		//categories found
		if (is_correct) {
			gt.getCategoriesObject(rans.getIdObject(), ids_categories_found);
			//sum stats
			sumInvRank += ((double) ids_objects_found.size())
					/ ((double) (rans.getRank() + 1));
			sumRank += rans.getRank() + 1;
			//first correct
			if (ids_objects_found.size() == 1) {
				res.precision_first = 1.0 / ((double) (rans.getRank() + 1));
				res.rank_first = rans.getRank() + 1;
			}
		}
		if (with_details) {
			if (is_false) {
				res.summary[rans.getRank()] = '.';		//false
			} else if (is_duplicated) {
				res.summary[rans.getRank()] = 'D';		//duplicated
			} else if (is_correct) {
				res.summary[rans.getRank()] = 'C';		//correct
				//corrects at rank
				for (long long k = rans.getRank(); k < this->max_rank; ++k)
					this->corrects_at_rank[k]++;
				//precision at rank
				double current_precision = ((double) ids_objects_found.size())
						/ ((double) (rans.getRank() + 1));
				for (long long k = rans.getRank(); k >= 0; --k)
					this->precision_at_rank[k] = MAX(current_precision,
							this->precision_at_rank[k]);
			}
		}
	}
	res.num_categories_expected = ids_categories_expected.size();
	res.num_categories_found = ids_categories_found.size();
	res.num_categories_missed = res.num_categories_expected
			- res.num_categories_found;
	res.num_objects_expected = ids_objects_expected.size();
	res.num_objects_found = ids_objects_found.size();
	res.num_objects_missed = res.num_objects_expected - res.num_objects_found;
	res.average_precision = sumInvRank / (double) res.num_objects_expected;
	// the rank for the missed objects is the maximum rank+1
	sumRank += (this->max_rank + 1) * res.num_objects_missed;
	res.average_rank = sumRank / (double) res.num_objects_expected;
}

template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnEvaluation<TypeQuery, TypeCategory, TypeObject>::evaluate_category(
		MknnCategoryEvaluation &cev, TypeCategory id_category) {
	std::set<TypeQuery> list_id_queries;
	this->gt.getQueriesCategory(id_category, list_id_queries);
	for (const TypeQuery &id_query : list_id_queries) {
		MknnQueryEvaluation &res = this->evaluationByQuery[id_query];
		cev.queries_mean_average_precision += res.average_precision;
		cev.queries_mean_average_rank += res.average_rank;
		cev.queries_average_precision_first += res.precision_first;
		cev.queries_average_rank_first += res.rank_first;
	}
	std::set<TypeObject> list_id_objects;
	this->gt.getObjectsCategory(id_category, list_id_objects);
	cev.num_queries = list_id_queries.size();
	cev.num_objects_expected = list_id_objects.size();
	cev.queries_mean_average_precision /= cev.num_queries;
	cev.queries_mean_average_rank /= cev.num_queries;
	cev.queries_average_precision_first /= cev.num_queries;
	cev.queries_average_rank_first /= cev.num_queries;
}

template class MknnEvaluation<std::string, std::string, std::string> ;

