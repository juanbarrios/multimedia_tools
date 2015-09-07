/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct LinearScan_Index {
	MknnDataset *search_dataset;
	MknnDistance *distance;
	bool is_fast_search;
};

static struct MknnIndexInstance linearScan_index_new(const char *id_index,
		MknnIndexParams *params_index, MknnDataset *search_dataset,
		MknnDistance *distance) {
	struct LinearScan_Index *state = MY_MALLOC(1, struct LinearScan_Index);
	state->search_dataset = search_dataset;
	state->distance = distance;
	//a fast search means a search that is resolved too fast
	//hence the cost of locks and threads becomes relevant
	//it depends on dataset size and distance computation time
	//TODO: test if the distance is fast (LP, Manhattan)
	state->is_fast_search = false;
	if (mknn_dataset_getNumObjects(search_dataset) < 1000)
		state->is_fast_search = true;
	struct MknnIndexInstance newIdx = { 0 };
	newIdx.state_index = state;
	newIdx.func_index_build = NULL;
	newIdx.func_index_load = NULL;
	newIdx.func_index_save = NULL;
	newIdx.func_index_release = free;
	return newIdx;
}
/* ******************************************************* */
struct LinearScan_Search {
	int64_t knn;
	double range;
	int64_t max_threads;
	struct LinearScan_Index *state_index;
	MknnDistanceEval **dist_evals;
	MknnHeap **heapsNNs;
	MknnDataset *query_dataset;
	MknnResult *result;
};

static void linearScan_resolveOneQuery(int64_t query_id,
		MknnDataset *query_dataset, MknnDataset *search_dataset, double range,
		MknnDistanceEval *distance_eval, MknnHeap *heapNNs, MknnResult *result) {
	mknn_heap_reset(heapNNs);
	void *query = mknn_dataset_getObject(query_dataset, query_id);
	double rangeSearch = range;
	int64_t num_database_objects = mknn_dataset_getNumObjects(search_dataset);
	for (int64_t i = 0; i < num_database_objects; ++i) {
		void *obj = mknn_dataset_getObject(search_dataset, i);
		double dist = mknn_distanceEval_evalTh(distance_eval, query, obj,
				rangeSearch);
		mknn_heap_storeBestDistances(dist, i, heapNNs, &rangeSearch);
	}
	mknn_result_storeMatchesInResultQuery(result, query_id, heapNNs,
			num_database_objects);
}
static void linearScan_resolver_query(int64_t current_process,
		void *state_object, int64_t current_thread) {
	struct LinearScan_Search *state = state_object;
	MknnDistanceEval *distance_eval = state->dist_evals[current_thread];
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	linearScan_resolveOneQuery(current_process, state->query_dataset,
			state->state_index->search_dataset, state->range, distance_eval,
			heapNNs, state->result);
}
static void linearScan_resolverBuffered_query(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread) {
	struct LinearScan_Search *state = state_object;
	MknnDistanceEval *distance_eval = state->dist_evals[current_thread];
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	for (int64_t current_process = start_process;
			current_process < end_process_notIncluded; ++current_process) {
		linearScan_resolveOneQuery(current_process, state->query_dataset,
				state->state_index->search_dataset, state->range, distance_eval,
				heapNNs, state->result);
	}
}
static MknnResult *linearScan_resolver_search(void *state_resolver,
		MknnDataset *query_dataset) {
	int64_t num_query_objects = mknn_dataset_getNumObjects(query_dataset);
	struct LinearScan_Search *state = state_resolver;
	state->query_dataset = query_dataset;
	state->result = mknn_result_newEmpty(num_query_objects, state->knn);
	state->dist_evals = mknn_distance_createDistEvalArray(
			state->state_index->distance, state->max_threads,
			mknn_dataset_getDomain(query_dataset),
			mknn_dataset_getDomain(state->state_index->search_dataset));
	//in a fast search the parallelism is made in large blocks
	//thus reducing the cost of synchronizing threads
	if (state->state_index->is_fast_search)
		my_parallel_buffered(num_query_objects, state,
				linearScan_resolverBuffered_query, NULL, state->max_threads,
				0);
	else
		my_parallel_incremental(num_query_objects, state,
				linearScan_resolver_query, "linear scan", state->max_threads);
	mknn_distanceEval_releaseArray(state->dist_evals, state->max_threads);
	return state->result;
}
static void linearScan_resolver_release(void *state_resolver) {
	struct LinearScan_Search *state = state_resolver;
	mknn_heap_releaseMulti(state->heapsNNs, state->max_threads);
	free(state);
}
struct MknnResolverInstance linearScan_resolver_new(void *state_index,
		const char *id_index, MknnResolverParams *params_resolver) {
	struct LinearScan_Search *state = MY_MALLOC(1, struct LinearScan_Search);
	state->state_index = state_index;
	state->knn = mknn_resolverParams_getKnn(params_resolver);
	if (state->knn < 1)
		state->knn = 1;
	state->range = mknn_resolverParams_getRange(params_resolver);
	state->max_threads = mknn_resolverParams_getMaxThreads(params_resolver);
	if (state->max_threads < 1)
		state->max_threads = my_parallel_getNumberOfCores();
	const char *name_method = mknn_resolverParams_getString(params_resolver,
			"method");
	if (name_method == NULL
			|| my_string_equals_ignorecase("NEAREST", name_method)) {
		state->heapsNNs = mknn_heap_newMultiMaxHeap(state->knn,
				state->max_threads);
		if (state->range == 0)
			state->range = DBL_MAX;
	} else if (my_string_equals_ignorecase("FARTHEST", name_method)) {
		state->heapsNNs = mknn_heap_newMultiMinHeap(state->knn,
				state->max_threads);
		if (state->range == 0)
			state->range = -DBL_MAX;
	} else {
		my_log_info("unknown method %s\n", name_method);
		mknn_predefIndex_helpPrintIndex(id_index);
	}
	struct MknnResolverInstance newResolver = { 0 };
	newResolver.state_resolver = state;
	newResolver.func_resolver_search = linearScan_resolver_search;
	newResolver.func_resolver_release = linearScan_resolver_release;
	return newResolver;
}
/*********************************************************/
void register_index_linearscan() {
	metricknn_register_index("LINEARSCAN", NULL, "method=[NEAREST|FARTHEST]",
	NULL, linearScan_index_new, linearScan_resolver_new);
}
