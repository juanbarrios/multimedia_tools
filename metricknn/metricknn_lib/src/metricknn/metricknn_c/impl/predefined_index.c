/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

MknnIndexParams *mknn_predefIndex_LinearScan_indexParams() {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "LINEARSCAN");
	return parameters;
}
MknnResolverParams *mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (range != 0)
		mknn_resolverParams_setRange(parameters, range);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	return parameters;
}

MknnResolverParams *mknn_predefIndex_LinearScan_resolverExactFarthestNeighbors(
		int64_t knn, int64_t max_threads) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	mknn_resolverParams_addString(parameters, "method", "FARTHEST");
	return parameters;
}

MknnIndexParams *mknn_predefIndex_Laesa_indexParams(int64_t num_pivots) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "LAESA");
	mknn_indexParams_addInt(parameters, "num_pivots", num_pivots);
	return parameters;
}
MknnResolverParams *mknn_predefIndex_Laesa_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (range != 0)
		mknn_resolverParams_setRange(parameters, range);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	return parameters;
}
MknnResolverParams *mknn_predefIndex_Laesa_resolverApproximateNearestNeighbors(
		int64_t knn, double range, int64_t max_threads,
		double approximation_percentage) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (range != 0)
		mknn_resolverParams_setRange(parameters, range);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	mknn_resolverParams_addString(parameters, "method", "APPROX");
	mknn_resolverParams_addDouble(parameters, "approximation",
			approximation_percentage);
	return parameters;
}

MknnIndexParams *mknn_predefIndex_SnakeTable_indexParams(int64_t num_pivots) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "SNAKETABLE");
	mknn_indexParams_addInt(parameters, "num_pivots", num_pivots);
	return parameters;
}

MknnResolverParams *mknn_predefIndex_SnakeTable_resolverExactNearestNeighbors(
		int64_t knn, double range, int64_t max_threads) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (range != 0)
		mknn_resolverParams_setRange(parameters, range);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	return parameters;
}

MknnIndexParams *mknn_predefIndex_FlannLinearScan_indexParams() {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "FLANN-LINEARSCAN");
	return parameters;
}

MknnIndexParams *mknn_predefIndex_FlannKdTree_indexParams(int64_t num_trees) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "FLANN-KDTREE");
	mknn_indexParams_addInt(parameters, "num_trees", num_trees);
	return parameters;
}
MknnIndexParams *mknn_predefIndex_FlannKmeansTree_indexParams(int64_t branching,
		int64_t iterations) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "FLANN-KMEANS");
	mknn_indexParams_addInt(parameters, "branching", branching);
	mknn_indexParams_addInt(parameters, "iterations", iterations);
	return parameters;
}
MknnIndexParams *mknn_predefIndex_FlannLSH_indexParams(int64_t table_number,
		int64_t key_size, int64_t multi_probe_level) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "FLANN-LSH");
	mknn_indexParams_addInt(parameters, "table_number", table_number);
	mknn_indexParams_addInt(parameters, "key_size", key_size);
	mknn_indexParams_addInt(parameters, "multi_probe_level", multi_probe_level);
	return parameters;
}
MknnIndexParams *mknn_predefIndex_FlannAutoIndex_indexParams(
		double target_precision, double build_weight, double memory_weight,
		double sample_fraction) {
	MknnIndexParams *parameters = mknn_indexParams_newEmpty();
	mknn_indexParams_setIndexId(parameters, "FLANN-AUTO");
	mknn_indexParams_addDouble(parameters, "target_precision",
			target_precision);
	mknn_indexParams_addDouble(parameters, "build_weight", build_weight);
	mknn_indexParams_addDouble(parameters, "memory_weight", memory_weight);
	mknn_indexParams_addDouble(parameters, "sample_fraction", sample_fraction);
	return parameters;
}
MknnResolverParams *mknn_predefIndex_Flann_resolverExactNearestNeighbors(
		int64_t knn, int64_t max_threads) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	mknn_resolverParams_addInt(parameters, "num_checks", -1);
	return parameters;
}
MknnResolverParams *mknn_predefIndex_Flann_resolverApproximateNearestNeighbors(
		int64_t knn, int64_t max_threads, int64_t num_checks) {
	MknnResolverParams *parameters = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(parameters, knn);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(parameters, max_threads);
	mknn_resolverParams_addInt(parameters, "num_checks", num_checks);
	return parameters;
}
