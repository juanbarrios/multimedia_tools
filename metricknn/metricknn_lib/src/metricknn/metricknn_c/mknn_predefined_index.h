/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PREDEFINED_INDEX_H_
#define MKNN_PREDEFINED_INDEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 *  MetricKnn provides a set of pre-defined indexes identified.
 *
 * The complete list of pre-defined indexes provided by MetricKnn can be printed by
 * calling #mknn_predefIndex_helpListIndexes. The parameters supported by each
 * index can be printed by calling #mknn_predefIndex_helpPrintIndex.
 *
 *  @file
 */

/**
 * @name Help functions.
 * @{
 */
/**
 * Lists to standard output all pre-defined indexes.
 */
void mknn_predefIndex_helpListIndexes();

/**
 * Prints to standard output the help for a index.
 *
 * @param id_index the unique identifier of a pre-defined index.
 */
void mknn_predefIndex_helpPrintIndex(const char *id_index);
/**
 * Tests whether the given string references a valid pre-defined index.
 *
 * @param id_index the unique identifier of a pre-defined index.
 * @return true whether @p id_index corresponds to a pre-defined index, and false otherwise.
 */
bool mknn_predefIndex_testIndexId(const char *id_index);
/**
 * @}
 */

/**
 * @name Linear Scan.
 * @{
 */

/**
 * Resolves searches using the linear scan algorithm.
 *
 * The algorithm compares each query object to each object in @p search_dataset sequentially and
 * retrieves the nearest objects.
 *
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_LinearScan_indexParams();

/**
 * Creates a new resolver for exact search of the K nearest neighbors
 * using the linear scan index.
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param range the search range. A value @<= 0 or constant @c DBL_MAX (defined in float.h) mean maximum range.
 * @param max_threads the maximum number of threads to be used.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads);

/**
 * Creates a new resolver for exact search of the K farthest neighbors
 * using the linear scan index.
 *
 * @param knn number of farthest neighbors to return. A value @< 1 means 1.
 * @param max_threads the maximum number of threads to be used.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_LinearScan_resolverExactFarthestNeighbors(
		int64_t knn, int64_t max_threads);

/**
 * @}
 */

/**
 * @name LAESA.
 * @{
 */

/**
 * Resolves searches using the LAESA algorithm.
 *
 * In order to obtain the exact nearest neighbors, the distance @p distance must
 * satisfy the metric properties, otherwise the exact search will be incorrect.
 *
 * @param num_pivots Amount of pivots to compute and store in memory.
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_Laesa_indexParams(int64_t num_pivots);

/**
 * Creates a new resolver for exact search using the LAESA index.
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param range the search range. A value @<= 0 or constant @c DBL_MAX mean maximum range.
 * @param max_threads the maximum number of threads to be used.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_Laesa_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads);
/**
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param range the search range. A value @<= 0 or the constant @c DBL_MAX mean maximum range.
 * @param max_threads the maximum number of threads to be used.
 * @param approximation_percentage The amount of distance evaluation to compute. A value between 0 and 1, where a value closer to 0 means
 * faster (but lower quality) search.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_Laesa_resolverApproximateNearestNeighbors(
		int64_t knn, double range, int64_t max_threads,
		double approximation_percentage);

/**
 * @}
 */

/**
 * @name Snake Table.
 * @{
 */

/**
 * Resolves searches using a Snake Table.
 *
 * In order to obtain the exact nearest neighbors, the distance @p distance must
 * satisfy the metric properties, otherwise the exact search will be incorrect.
 *
 * @param num_pivots Amount of pivots to store in memory.
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_SnakeTable_indexParams(int64_t num_pivots);

/**
 * Creates a new resolver for exact search using the SnakeTable index.
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param range the search range. A value @<= 0 or constant @c DBL_MAX mean maximum range.
 * @param max_threads the maximum number of threads to be used.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_SnakeTable_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads);

/**
 * @}
 */

/**
 * @name FLANN Indexes.
 * @{
 */

/**
 * Resolves searches using the FLANN's implementation of linear scan algorithm.
 *
 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
 *
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_FlannLinearScan_indexParams();

/**
 * Resolves searches using FLANN's implementation of kd-tree.
 *
 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
 *
 * @param num_trees Amount of trees to build.
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_FlannKdTree_indexParams(int64_t num_trees);

/**
 * Resolves searches using FLANN's implementation of k-means tree.
 *
 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
 *
 * @param branching branching factor. Number of centroids at each level.
 * @param iterations max iterations to perform in one kmeans clustering.
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_FlannKmeansTree_indexParams(int64_t branching,
		int64_t iterations);

/**
 * Resolves searches using FLANN's implementation of Local Sensitive Hashing.
 *
 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
 *
 * @param table_number The number of hash tables to use
 * @param key_size The length of the key in the hash tables
 * @param multi_probe_level Number of levels to use in multi-probe LSH, 0 for standard LSH
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_FlannLSH_indexParams(int64_t table_number,
		int64_t key_size, int64_t multi_probe_level);
/**
 * Resolves searches using FLANN's automatic selection of the best algorithm.
 *
 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
 *
 * @param target_precision precision desired
 * @param build_weight build tree time weighting factor
 * @param memory_weight index memory weigthing factor
 * @param sample_fraction what fraction of the dataset to use for autotuning
 * @return parameters to create an index (it must be released with mknn_indexParams_release or bound to the new index)
 */
MknnIndexParams *mknn_predefIndex_FlannAutoIndex_indexParams(
		double target_precision, double build_weight, double memory_weight,
		double sample_fraction);

/**
 * Creates a new resolver for exact searches using FLANN's indexes.
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param max_threads the maximum number of threads to be used.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_Flann_resolverExactNearestNeighbors(
		int64_t knn, int64_t max_threads);

/**
 * Creates a new resolver for approximate searches using FLANN's indexes.
 *
 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
 * @param max_threads the maximum number of threads to be used.
 * @param num_checks  how many leafs to check in one search. A value @>= 1, where a value closer to 1 means
 * faster (but lower quality) search.
 * @return parameters to create a resolver for the given similarity search (it must be released with mknn_resolverParams_release or bound to the new resolver)
 */
MknnResolverParams *mknn_predefIndex_Flann_resolverApproximateNearestNeighbors(
		int64_t knn, int64_t max_threads, int64_t num_checks);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
