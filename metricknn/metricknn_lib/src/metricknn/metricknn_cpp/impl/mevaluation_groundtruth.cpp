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

using namespace mknn;

template<typename TypeQuery, typename TypeCategory, typename TypeObject>
MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::MknnGroundTruth() {
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::~MknnGroundTruth() {
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::addCorrectAnswer(
		TypeQuery id_query, TypeCategory id_category) {
	this->list_categories_by_query[id_query].insert(id_category);
	this->list_queries_by_category[id_category].insert(id_query);
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::addCategoryObject(
		TypeCategory id_category, TypeObject id_object) {
	this->list_objects_by_category[id_category].insert(id_object);
	this->list_categories_by_object[id_object].insert(id_category);
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
long long MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getTotalQueries() {
	return this->list_categories_by_query.size();
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
long long MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getTotalObjects() {
	return this->list_categories_by_object.size();
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
long long MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getTotalCategories() {
	return this->list_objects_by_category.size();
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getCategoriesQuery(
		TypeQuery id_query, std::set<TypeCategory> &list_id_categories) {
	for (auto const &id_category : this->list_categories_by_query[id_query])
		list_id_categories.insert(id_category);
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getCategoriesObject(
		TypeObject id_object, std::set<TypeCategory> &list_id_categories) {
	for (auto const &id_category : this->list_categories_by_object[id_object])
		list_id_categories.insert(id_category);
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getObjectsCategory(
		TypeCategory id_category, std::set<TypeObject> &list_id_object) {
	for (auto const &id_object : this->list_objects_by_category[id_category])
		list_id_object.insert(id_object);
}
template<typename TypeQuery, typename TypeCategory, typename TypeObject>
void MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getQueriesCategory(
		TypeCategory id_category, std::set<TypeQuery> &list_id_queries) {
	for (auto const &id_query : this->list_queries_by_category[id_category])
		list_id_queries.insert(id_query);
}

template<typename TypeQuery, typename TypeCategory, typename TypeObject>
std::set<TypeCategory> MknnGroundTruth<TypeQuery, TypeCategory, TypeObject>::getCategories() {
	std::set<TypeCategory> list;
	for (const auto &it : this->list_objects_by_category)
		list.insert(it.first);
	return list;
}

template class MknnGroundTruth<std::string, std::string, std::string> ;
