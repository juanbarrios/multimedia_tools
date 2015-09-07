/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PREDEFINED_INDEX_HPP_
#define MKNN_PREDEFINED_INDEX_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 *  MetricKnn provides a set of pre-defined indexes.
 *
 * The complete list of pre-defined indexes provided by MetricKnn can be printed by
 * calling PredefIndex::helpListIndexes. The parameters supported by each
 * index can be printed by calling PredefIndex::helpPrintIndex.
 *
 */
class PredefIndex {
public:
	/**
	 * @name Help functions.
	 * @{
	 */
	/**
	 * Lists to standard output all pre-defined indexes.
	 */
	static void helpListIndexes();

	/**
	 * Prints to standard output the help for a index.
	 *
	 * @param id_index the unique identifier of a pre-defined index.
	 */
	static void helpPrintIndex(std::string id_index);
	/**
	 * Tests whether the given string references a valid pre-defined index.
	 *
	 * @param id_index the unique identifier of a pre-defined index.
	 * @return true whether @p id_index corresponds to a pre-defined index, and false otherwise.
	 */
	static bool testIndexId(std::string id_index);
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
	 * @return the new index (must be released with delete)
	 */
	static IndexParams LinearScan_indexParams();
	/**
	 * Creates a new resolver for exact search of the K nearest neighbors
	 * using the linear scan index.
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param range the search range. A value @<= 0 or constant @c DBL_MAX mean maximum range.
	 * @param max_threads the maximum number of threads to be used.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams LinearScan_resolverExactNearestNeighbors(long long knn,
			double range, long long max_threads);

	/**
	 * Creates a new resolver for exact search of the K farthest neighbors
	 * using the linear scan index.
	 *
	 * @param knn number of farthest neighbors to return. A value @< 1 means 1.
	 * @param max_threads the maximum number of threads to be used.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams LinearScan_resolverExactFarthestNeighbors(
			long long knn, long long max_threads);

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
	 * In order to obtain the exact nearest neighbors, the distance @p dist_factory must
	 * satisfy the metric properties, otherwise the exact search will be incorrect.
	 *
	 * @param num_pivots Amount of pivots to compute and store in memory.
	 * @return the new index (must be released with delete)
	 */
	static IndexParams Laesa_indexParams(long long num_pivots);
	/**
	 * Creates a new resolver for exact search using the LAESA index.
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param range the search range. A value @<= 0 or constant @c DBL_MAX mean maximum range.
	 * @param max_threads the maximum number of threads to be used.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams Laesa_resolverExactNearestNeighbors(long long knn,
			double range, long long max_threads);
	/**
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param range the search range. A value @<= 0 or the constant @c DBL_MAX mean maximum range.
	 * @param max_threads the maximum number of threads to be used.
	 * @param approximation_percentage The amount of distance evaluation to compute. A value between 0 and 1, where a value closer to 0 means
	 * faster (but lower quality) search.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams Laesa_resolverApproximateNearestNeighbors(
			long long knn, double range, long long max_threads,
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
	 * In order to obtain the exact nearest neighbors, the distance @p dist_factory must
	 * satisfy the metric properties, otherwise the exact search will be incorrect.
	 *
	 * @param num_pivots Amount of pivots to store in memory.
	 * @return the new index (must be released with delete)
	 */
	static IndexParams SnakeTable_indexParams(long long num_pivots);
	/**
	 * Creates a new resolver for exact search using the SnakeTable index.
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param range the search range. A value @<= 0 or constant @c DBL_MAX mean maximum range.
	 * @param max_threads the maximum number of threads to be used.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams SnakeTable_resolverExactNearestNeighbors(long long knn,
			double range, long long max_threads);

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
	 * @return the new index (must be released with delete)
	 */
	static IndexParams FlannLinearScan_indexParams();

	/**
	 * Resolves searches using FLANN's implementation of kd-tree.
	 *
	 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
	 *
	 * @param num_trees Amount of trees to build.
	 * @return the new index (must be released with delete)
	 */
	static IndexParams FlannKdTree_indexParams(long long num_trees);

	/**
	 * Resolves searches using FLANN's implementation of k-means tree.
	 *
	 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
	 *
	 * @param branching branching factor. Number of centroids at each level.
	 * @param iterations max iterations to perform in one kmeans clustering.
	 * @return the new index (must be released with delete)
	 */
	static IndexParams FlannKmeansTree_indexParams(long long branching,
			long long iterations);

	/**
	 * Resolves searches using FLANN's implementation of Local Sensitive Hashing.
	 *
	 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
	 *
	 * @param table_number The number of hash tables to use
	 * @param key_size The length of the key in the hash tables
	 * @param multi_probe_level Number of levels to use in multi-probe LSH, 0 for standard LSH
	 * @return the new index (must be released with delete)
	 */
	static IndexParams FlannLSH_indexParams(long long table_number,
			long long key_size, long long multi_probe_level);
	/**
	 * Resolves searches using FLANN's automatic selection of the best algorithm.
	 *
	 * This method is just an interface to FLANN library. See FLANN's documentation http://www.cs.ubc.ca/research/flann/ .
	 *
	 * @param target_precision precision desired
	 * @param build_weight build tree time weighting factor
	 * @param memory_weight index memory weigthing factor
	 * @param sample_fraction what fraction of the dataset to use for autotuning
	 * @return the new index (must be released with delete)
	 */
	static IndexParams FlannAutoIndex_indexParams(double target_precision,
			double build_weight, double memory_weight, double sample_fraction);
	/**
	 * Creates a new resolver for exact searches using FLANN's indexes.
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param max_threads the maximum number of threads to be used.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams Flann_resolverExactNearestNeighbors(long long knn,
			long long max_threads);

	/**
	 * Creates a new resolver for approximate searches using FLANN's indexes.
	 *
	 * @param knn number of nearest neighbors to return. A value @< 1 means 1.
	 * @param max_threads the maximum number of threads to be used.
	 * @param num_checks  how many leafs to check in one search. A value @>= 1, where a value closer to 1 means
	 * faster (but lower quality) search.
	 * @return an object with the configuration of a similarity search.
	 */
	static ResolverParams Flann_resolverApproximateNearestNeighbors(
			long long knn, long long max_threads, long long num_checks);

	/**
	 * @}
	 */

};

}

#endif
