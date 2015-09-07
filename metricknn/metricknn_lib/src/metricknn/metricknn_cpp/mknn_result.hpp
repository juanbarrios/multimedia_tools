/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_RESULT_HPP_
#define MKNN_RESULT_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

class ResultQuery;
class Resolver;

/**
 * The result of a search for a set of queries.
 */
class Result {
public:
	/**
	 *
	 * @return amount of resolved queries.
	 */
	long long getNumQueries();
	/**
	 *
	 * @return total time in seconds spent by the search.
	 */
	double getTotalSearchTime();
	/**
	 *
	 * @return total amount of distances evaluated during the search.
	 */
	long long getTotalDistanceEvaluations();
	/**
	 *
	 * @return the resolver that generated this result.
	 */
	Resolver &getResolver();
	/**
	 *
	 * @return the query dataset that generated this result.
	 */
	Dataset &getQueryDataset();
	/**
	 * Returns the result for each query in the query dataset.
	 * @param num_query the number of query to return, between 0 and #getNumQueries - 1.
	 * @return result for each query object.
	 */
	ResultQuery getResultQuery(long long num_query);

	/**
	 * Default constructor.
	 */
	Result();
	/**
	 * Default destructor.
	 */
	virtual ~Result();
	/**
	 * Copy constructor.
	 */
	Result(const Result &other);
	/**
	 * Assignment operator.
	 */
	Result &operator=(const Result &other);

protected:
	/**
	 * Internal opaque class
	 */
	class Impl;
	/**
	 * Internal opaque class
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Resolver;
};

/**
 * The result of a single query.
 */

class ResultQuery {
public:
	long long num_nns; /**< number of nearest neighbors. */
	double *nn_distance; /**< the distance of each nearest neighbor to the query object. */
	long long *nn_position; /**< the position in the search dataset of each nearest neighbor. The actual object can be retrieved by calling Dataset::getObject. */
	long long num_distance_evaluations; /**< amount of distances evaluated for resolving this query. */
};

}
#endif
