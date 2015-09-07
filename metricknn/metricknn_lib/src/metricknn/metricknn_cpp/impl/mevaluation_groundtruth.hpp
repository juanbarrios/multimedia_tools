/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_EVALUATION_GROUND_TRUTH_HPP
#define MKNN_EVALUATION_GROUND_TRUTH_HPP

#include <map>
#include <string>
#include <set>
#include <vector>
#include "../metricknn_impl.hpp"

namespace mknn {
template<typename TypeQuery, typename TypeCategory, typename TypeObject> class MknnGroundTruth {
public:
	MknnGroundTruth();
	~MknnGroundTruth();
	void addCorrectAnswer(TypeQuery id_query, TypeCategory id_category);
	void addCategoryObject(TypeCategory id_category, TypeObject id_object);
	long long getTotalQueries();
	long long getTotalCategories();
	long long getTotalObjects();
	void getCategoriesQuery(TypeQuery id_query,
			std::set<TypeCategory> &list_id_categories);
	void getCategoriesObject(TypeObject id_object,
			std::set<TypeCategory> &list_id_categories);
	void getObjectsCategory(TypeCategory id_category,
			std::set<TypeObject> &list_id_object);
	void getQueriesCategory(TypeCategory id_category,
			std::set<TypeQuery> &list_id_queries);
	std::set<TypeCategory> getCategories();

private:
	std::map<TypeQuery, std::set<TypeCategory>> list_categories_by_query;
	std::map<TypeCategory, std::set<TypeObject>> list_objects_by_category;
	std::map<TypeQuery, std::set<TypeObject>> list_queries_by_category;
	std::map<TypeObject, std::set<TypeCategory>> list_categories_by_object;
};

typedef MknnGroundTruth<std::string, std::string, std::string> MknnGroundTruth_s;
}
#endif
