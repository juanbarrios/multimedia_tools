/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_RESULT_H_
#define MKNN_RESULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * A MknnResult stores the result for a query dataset, and MknnResultQuery stores the result for each query object.
 *
 * MknnResultQuery is non-opaque, thus the results can be read be accessing the object attributes.
 * Note that MknnResultQuery objects are binded to MknnResult, thus by releasing MknnResult (#mknn_result_release) all the
 * MknnResultQuery objects will be released too.
 *
 *  @file
 */

/**
 *
 * @param result
 * @return amount of resolved queries.
 */
int64_t mknn_result_getNumQueries(MknnResult *result);
/**
 *
 * @param result
 * @return total time in seconds spent by the search.
 */
double mknn_result_getTotalSearchTime(MknnResult *result);
/**
 *
 * @param result
 * @return total amount of distances evaluated during the search.
 */
int64_t mknn_result_getTotalDistanceEvaluations(MknnResult *result);
/**
 *
 * @param result
 * @return the resolver that generated this result.
 */
MknnResolver *mknn_result_getResolver(MknnResult *result);

/**
 * @param result
 * @return the query dataset that generated this result.
 */
MknnDataset *mknn_result_getQueryDataset(MknnResult *result);
/**
 * Returns the result for each query in the query dataset.
 * @param result
 * @param num_query the number of query to return, between 0 and #mknn_result_getNumQueries - 1.
 * @return result for each query object.
 */
MknnResultQuery *mknn_result_getResultQuery(MknnResult *result,
		int64_t num_query);
/**
 * Releases the result of a search (including the result for each query) that may have been returned by #mknn_result_getResultQuery.
 *
 * @param result the result object to be released.
 */
void mknn_result_release(MknnResult *result);

/**
 * The result of a single query.
 */
struct MknnResultQuery {
	int64_t num_nns; /**< number of nearest neighbors. */
	double *nn_distance; /**< the distance of each nearest neighbor to the query object. */
	int64_t *nn_position; /**< the position in the search dataset of each nearest neighbor. The actual object can be retrieved by calling #mknn_dataset_getObject. */
	int64_t num_distance_evaluations; /**< amount of distances evaluated for resolving this query. */
};

#ifdef __cplusplus
}
#endif

#endif
