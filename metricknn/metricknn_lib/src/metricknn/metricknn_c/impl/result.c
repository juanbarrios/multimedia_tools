/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnResult {
	int64_t num_queries, num_nn_max;
	double total_search_time;
	int64_t total_distance_evaluations;
	MknnResultQuery *all_results;
	int64_t *all_positions;
	double *all_distances;
	MknnResolver *resolver;
	MknnDataset *query_dataset;
	bool free_resolver_on_release;
	bool free_query_dataset_on_release;
};

MknnResult *mknn_result_newEmpty(int64_t num_queries, int64_t num_nn_max) {
	MknnResult *result = MY_MALLOC(1, MknnResult);
	result->num_queries = num_queries;
	result->all_results = MY_MALLOC(result->num_queries, MknnResultQuery);
	result->all_positions = MY_MALLOC(result->num_queries * num_nn_max,
			int64_t);
	result->all_distances = MY_MALLOC(result->num_queries * num_nn_max, double);
	for (int64_t i = 0; i < result->num_queries; ++i) {
		MknnResultQuery *res = result->all_results + i;
		res->nn_position = result->all_positions + num_nn_max * i;
		res->nn_distance = result->all_distances + num_nn_max * i;
	}
	return result;
}

int64_t mknn_result_getNumQueries(MknnResult *result) {
	if (result == NULL)
		return 0;
	return result->num_queries;
}
double mknn_result_getTotalSearchTime(MknnResult *result) {
	if (result == NULL)
		return 0;
	return result->total_search_time;
}
int64_t mknn_result_getTotalDistanceEvaluations(MknnResult *result) {
	if (result == NULL)
		return 0;
	return result->total_distance_evaluations;
}
MknnResolver *mknn_result_getResolver(MknnResult *result) {
	if (result == NULL)
		return NULL;
	return result->resolver;
}
MknnDataset *mknn_result_getQueryDataset(MknnResult *result) {
	if (result == NULL)
		return NULL;
	return result->query_dataset;
}
MknnResultQuery *mknn_result_getResultQuery(MknnResult *result,
		int64_t num_query) {
	return result->all_results + num_query;
}
#if 0
static void mknn_result_saveOld(MknnResult *result, const char *filename_write) {
	FILE *out = my_io_openFileWrite1(filename_write);
	fwrite(&result->num_queries, sizeof(int64_t), 1, out);
	fwrite(&result->total_distance_evaluations, sizeof(int64_t), 1, out);
	fwrite(&result->total_search_time, sizeof(double), 1, out);
	for (int64_t i = 0; i < result->num_queries; ++i) {
		MknnResultQuery *res = result->all_results + i;
		fwrite(&res->num_nns, sizeof(int64_t), 1, out);
		fwrite(&res->num_distance_evaluations, sizeof(int64_t), 1, out);
		fwrite(res->nn_position, sizeof(int64_t), res->num_nns, out);
		fwrite(res->nn_distance, sizeof(double), res->num_nns, out);
	}
	fclose(out);
}
static MknnResult *mknn_result_loadOld(const char *filename_write) {
	FILE *in = my_io_openFileRead1(filename_write, true);
	MknnResult *result = MY_MALLOC(1, MknnResult);
	fread(&result->num_queries, sizeof(int64_t), 1, in);
	fread(&result->total_distance_evaluations, sizeof(int64_t), 1, in);
	fread(&result->total_search_time, sizeof(double), 1, in);
	result->all_results = MY_MALLOC(result->num_queries, MknnResultQuery);
	for (int64_t i = 0; i < result->num_queries; ++i) {
		MknnResultQuery *res = result->all_results + i;
		fread(&res->num_nns, sizeof(int64_t), 1, in);
		fread(&res->num_distance_evaluations, sizeof(int64_t), 1, in);
		res->nn_position = MY_MALLOC(res->num_nns, int64_t);
		res->nn_distance = MY_MALLOC(res->num_nns, double);
		fread(res->nn_position, sizeof(int64_t), res->num_nns, in);
		fread(res->nn_distance, sizeof(double), res->num_nns, in);
	}
	fclose(in);
	return result;
}
#endif
void mknn_result_release(MknnResult *result) {
	if (result == NULL)
		return;
	free(result->all_results);
	free(result->all_positions);
	free(result->all_distances);
	if (result->free_query_dataset_on_release)
		mknn_dataset_release(result->query_dataset);
	if (result->free_resolver_on_release)
		mknn_resolver_release(result->resolver);
	free(result);
}

void mknn_result_updateTotalDistanceEvaluations(MknnResult *result) {
	result->total_distance_evaluations = 0;
	for (int64_t i = 0; i < result->num_queries; ++i) {
		MknnResultQuery *res = mknn_result_getResultQuery(result, i);
		result->total_distance_evaluations += res->num_distance_evaluations;
	}
}
void mknn_result_setTotalSearchTime(MknnResult *result,
		double total_search_time) {
	result->total_search_time = total_search_time;
}
void mknn_result_setResolverQueryDataset(MknnResult *result,
		MknnResolver *resolver, MknnDataset *query_dataset,
		bool free_resolver_on_release,
		bool free_query_dataset_on_release) {
	result->resolver = resolver;
	result->query_dataset = query_dataset;
	result->free_resolver_on_release = free_resolver_on_release;
	result->free_query_dataset_on_release = free_query_dataset_on_release;
}
void mknn_result_storeMatchesInResultQuery(MknnResult *result,
		int64_t num_query, MknnHeap *heap, int64_t cont_evaluations) {
	MknnResultQuery *res = mknn_result_getResultQuery(result, num_query);
	res->num_distance_evaluations = cont_evaluations;
	int64_t heap_size = mknn_heap_getSize(heap);
	res->num_nns = heap_size;
	mknn_heap_sortElements(heap);
	for (int64_t i = 0; i < heap_size; ++i) {
		res->nn_distance[i] = mknn_heap_getDistanceAtPosition(heap, i);
		res->nn_position[i] = mknn_heap_getObjectIdAtPosition(heap, i);
	}
}

