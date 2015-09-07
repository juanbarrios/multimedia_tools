/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_EVALUATION_ANSWERS_HPP
#define MKNN_EVALUATION_ANSWERS_HPP

#include <map>
#include <string>
#include <set>
#include <vector>
#include "../metricknn_impl.hpp"

namespace mknn {
template<typename TypeObject> class MknnRankedAnswer {
public:
	MknnRankedAnswer();
	/**
	 *
	 * @param id_object
	 * @param rank_position starting from 0.
	 */
	MknnRankedAnswer(TypeObject id_object, long long rank_position);
	~MknnRankedAnswer();
	const TypeObject& getIdObject() const;
	/**
	 * the rank, 0 is the first.
	 */
	long long getRank() const;
	/**
	 * comparator by rank.
	 * @param other
	 * @return
	 */
	bool operator <(const MknnRankedAnswer& other) const;

private:
	TypeObject id_object;
	//rank starting from 0
	long long rank_position;
};

template<typename TypeQuery, typename TypeObject> class MknnAnswers {
public:
	MknnAnswers();
	~MknnAnswers();
	void addAnswer(TypeQuery id_query, TypeObject id_object,
			long long rank_position);
	long long getNumRanks();
	std::set<TypeQuery> getQueryNames();
	std::vector<MknnRankedAnswer<TypeObject>> &getQueryAnswers(
			TypeQuery id_query);
private:
	std::map<TypeQuery, std::vector<MknnRankedAnswer<TypeObject>>>answers;
};

typedef MknnRankedAnswer<std::string> MknnRankedAnswer_s;
typedef MknnAnswers<std::string, std::string> MknnAnswers_s;

}
#endif
