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
#include <myutils/myutils_cpp.hpp>
#include <iostream>

using namespace mknn;

template<typename TypeObject>
MknnRankedAnswer<TypeObject>::MknnRankedAnswer() :
		rank_position(-1) {
}
template<typename TypeObject>
MknnRankedAnswer<TypeObject>::MknnRankedAnswer(TypeObject id_object,
		long long rank_position) :
		id_object(id_object), rank_position(rank_position) {
	if (rank_position < 0)
		throw std::runtime_error("invalid rank");
}
template<typename TypeObject>
MknnRankedAnswer<TypeObject>::~MknnRankedAnswer() {
}
template<typename TypeObject>
const TypeObject& MknnRankedAnswer<TypeObject>::getIdObject() const {
	return id_object;
}
template<typename TypeObject>
long long MknnRankedAnswer<TypeObject>::getRank() const {
	return rank_position;
}
template<typename TypeObject>
bool MknnRankedAnswer<TypeObject>::operator <(
		const MknnRankedAnswer& other) const {
	return (rank_position < other.rank_position);
}

template<typename TypeQuery, typename TypeObject>
MknnAnswers<TypeQuery, TypeObject>::MknnAnswers() {
}
template<typename TypeQuery, typename TypeObject>
MknnAnswers<TypeQuery, TypeObject>::~MknnAnswers() {
}
template<typename TypeQuery, typename TypeObject>
void MknnAnswers<TypeQuery, TypeObject>::addAnswer(TypeQuery id_query,
		TypeObject id_object, long long rank_position) {
	this->answers[id_query].push_back(
			MknnRankedAnswer<TypeObject>(id_object, rank_position));
}
template<typename TypeQuery, typename TypeObject>
long long MknnAnswers<TypeQuery, TypeObject>::getNumRanks() {
	long long max_rank = -1;
	for (auto const &it1 : this->answers) {
		for (auto const &it2 : it1.second) {
			if (it2.getRank() > max_rank)
				max_rank = it2.getRank();
		}
	}
	return max_rank + 1;
}
template<typename TypeQuery, typename TypeObject>
std::set<TypeQuery> MknnAnswers<TypeQuery, TypeObject>::getQueryNames() {
	std::set<TypeQuery> names;
	for (auto const &it1 : this->answers) {
		names.insert(it1.first);
	}
	return names;
}
template<typename TypeQuery, typename TypeObject>
std::vector<MknnRankedAnswer<TypeObject>> &MknnAnswers<TypeQuery, TypeObject>::getQueryAnswers(
		TypeQuery id_query) {
	return this->answers[id_query];
}

template class MknnRankedAnswer<std::string> ;
template class MknnAnswers<std::string, std::string> ;

