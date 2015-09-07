/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#ifndef NO_FLANN
#include <flann/flann.h>
#endif

struct LAESA_Index {
	int64_t num_pivots;
	MknnDataset *search_dataset;
	MknnDistance *distance;
	//index
	int64_t *pivots_position;
	void **pivots;
	double **pivot_table;
};
static void laesa_index_buildPivotTable_thread(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread) {
	struct LAESA_Index *state = state_object;
	MknnDistanceEval *distance_eval = mknn_distance_newDistanceEval(
			state->distance, mknn_dataset_getDomain(state->search_dataset),
			mknn_dataset_getDomain(state->search_dataset));
	for (int64_t id_obj = start_process; id_obj < end_process_notIncluded;
			++id_obj) {
		void *obj = mknn_dataset_getObject(state->search_dataset, id_obj);
		for (int64_t id_piv = 0; id_piv < state->num_pivots; ++id_piv) {
			void *piv = state->pivots[id_piv];
			double d = mknn_distanceEval_eval(distance_eval, piv, obj);
			state->pivot_table[id_obj][id_piv] = d;
		}
	}
	mknn_distanceEval_release(distance_eval);
}
static void laesa_index_buildPivotTable(struct LAESA_Index *state,
		int64_t max_threads) {
	state->pivots = MY_MALLOC_NOINIT(state->num_pivots, void*);
	for (int64_t i = 0; i < state->num_pivots; ++i) {
		state->pivots[i] = mknn_dataset_getObject(state->search_dataset,
				state->pivots_position[i]);
	}
	int64_t num_objects = mknn_dataset_getNumObjects(state->search_dataset);
	my_log_info(
			"populating pivot table (%"PRIi64" objects, %"PRIi64" pivots, %"PRIi64" threads)...\n",
			num_objects, state->num_pivots, max_threads);
	state->pivot_table = MY_MALLOC_MATRIX(num_objects, state->num_pivots,
			double);
	my_parallel_buffered(num_objects, state, laesa_index_buildPivotTable_thread,
			"building pivot table", max_threads, 0);
}
static void laesa_index_save(void *state_index, const char *id_index,
		MknnIndexParams *params_index, const char *filename_write) {
	struct LAESA_Index *state = state_index;
	FILE *out = my_io_openFileWrite1Config(filename_write, "MetricKnn",
			"IndexLAESA", 1, 0);
	fprintf(out, "num_pivots=%"PRIi64"\n", state->num_pivots);
	for (int64_t i = 0; i < state->num_pivots; ++i) {
		fprintf(out, "piv_%02"PRIi64"=%"PRIi64"\n", i,
				state->pivots_position[i]);
	}
	fclose(out);
}
static void laesa_index_load(void *state_index, const char *id_index,
		MknnIndexParams *params_index, const char *filename_read) {
	struct LAESA_Index *state = state_index;
	state->pivots_position = MY_MALLOC(state->num_pivots, int64_t);
	MyMapStringObj *prop = my_io_loadProperties(filename_read, 1, "MetricKnn",
			"IndexLAESA", 1, 0);
	int64_t num_pivots = my_parse_int(my_mapStringObj_get(prop, "num_pivots"));
	my_assert_equalInt("num_pivots", state->num_pivots, num_pivots);
	for (int64_t i = 0; i < state->num_pivots; ++i) {
		char *name = my_newString_format("piv_%02"PRIi64"", i);
		char *value = my_mapStringObj_get(prop, name);
		state->pivots_position[i] = my_parse_int(value);
		MY_FREE(name);
	}
	int64_t max_threads = my_parse_int0(
			my_mapStringObj_get(prop, "max_threads"));
	if (max_threads <= 0)
		max_threads = my_parallel_getNumberOfCores();
	my_mapStringObj_release(prop, true, true);
	laesa_index_buildPivotTable(state, max_threads);
}

static void laesa_index_build(void *state_index, const char *id_index,
		MknnIndexParams *params_index) {
	struct LAESA_Index *state = state_index;
	int64_t num_sets_eval = mknn_indexParams_getInt(params_index, "sets_eval");
	int64_t max_threads = mknn_indexParams_getInt(params_index, "max_threads");
	if (max_threads <= 0)
		max_threads = my_parallel_getNumberOfCores();
	if (num_sets_eval <= 0)
		num_sets_eval = MAX(2, max_threads);
	state->pivots_position = MY_MALLOC(state->num_pivots, int64_t);
	mknn_laesa_select_pivots_sss(state->search_dataset, state->distance,
			state->num_pivots, num_sets_eval, max_threads,
			state->pivots_position);
	laesa_index_buildPivotTable(state, max_threads);
}
static void laesa_index_release(void *state_index) {
	struct LAESA_Index *state = state_index;
	MY_FREE_MULTI(state->pivots_position, state->pivots);
	MY_FREE_MATRIX(state->pivot_table, state->num_pivots);
	MY_FREE(state);
}

static struct MknnIndexInstance laesa_index_new(const char *id_index,
		MknnIndexParams *params_index, MknnDataset *search_dataset,
		MknnDistance *distance) {
	struct LAESA_Index *state = MY_MALLOC(1, struct LAESA_Index);
	state->num_pivots = mknn_indexParams_getInt(params_index, "num_pivots");
	if (state->num_pivots <= 0) {
		my_log_info("num_pivots must be greater than 0\n");
		mknn_predefIndex_helpPrintIndex(id_index);
	}
	state->search_dataset = search_dataset;
	state->distance = distance;
	struct MknnIndexInstance newIdx = { 0 };
	newIdx.state_index = state;
	newIdx.func_index_build = laesa_index_build;
	newIdx.func_index_load = laesa_index_load;
	newIdx.func_index_save = laesa_index_save;
	newIdx.func_index_release = laesa_index_release;
	return newIdx;
}
/***************************************************/

#define METHOD_EXACT_SEARCH 1
#define METHOD_LB_ONLY 2
#define METHOD_APPROX_SEARCH 3

#ifndef NO_FLANN
#define METHOD_APPROX_SEARCH_USING_FLANN 4
#endif

struct LAESA_Search {
	int64_t knn;
	double range;
	int64_t max_threads;
	MknnDataset *query_dataset;
	struct LAESA_Index *state_index;
	struct MknnResult *result;
	int64_t numPivotsDiv4, numPivotsMod4;
	//
	int64_t method;
	MknnDistanceEval **dist_evals;
	MknnHeap **heapsNNs;
	double approx_pct;
	int64_t approx_size;
	double **dist_query_pivots;
	int64_t *dist_evaluations;
	MknnHeap **heapsLBs;
	//
#ifndef NO_FLANN
	struct FLANNParameters fnn_parameters;
	flann_index_t fnn_index;
	double *fnn_datatable;
	int **fnn_ids;
	double **fnn_dists;
#endif
};

static bool laesa_tryToDiscard(int64_t num_obj, double *dist_query_pivots,
		double rangeSearch, struct LAESA_Search *state) {
	double *pivot_table = state->state_index->pivot_table[num_obj];
	int64_t numA = state->numPivotsDiv4;
	while (numA > 0) {
		double lb1 = fabs(dist_query_pivots[0] - pivot_table[0]);
		double lb2 = fabs(dist_query_pivots[1] - pivot_table[1]);
		double lb3 = fabs(dist_query_pivots[2] - pivot_table[2]);
		double lb4 = fabs(dist_query_pivots[3] - pivot_table[3]);
		if (lb1 > rangeSearch || lb2 > rangeSearch || lb3 > rangeSearch
				|| lb4 > rangeSearch)
			return true;
		dist_query_pivots += 4;
		pivot_table += 4;
		numA--;
	}
	int64_t numB = state->numPivotsMod4;
	while (numB > 0) {
		double lb = fabs(dist_query_pivots[0] - pivot_table[0]);
		if (lb > rangeSearch)
			return true;
		dist_query_pivots += 1;
		pivot_table += 1;
		numB--;
	}
	return false;
}

//returns DBL_MAX when obj should be discarded (lb > rangeSearch)
//otherwise returns maximum lower bound
static double laesa_computeMaxLB(int64_t num_obj, double *dist_query_pivots,
		double rangeSearch, struct LAESA_Search *state) {
	double *pivot_table = state->state_index->pivot_table[num_obj];
	int64_t numA = state->numPivotsDiv4;
	double maxLB = 0;
	while (numA > 0) {
		double lb1 = fabs(dist_query_pivots[0] - pivot_table[0]);
		double lb2 = fabs(dist_query_pivots[1] - pivot_table[1]);
		double lb3 = fabs(dist_query_pivots[2] - pivot_table[2]);
		double lb4 = fabs(dist_query_pivots[3] - pivot_table[3]);
		double maxxLB = MAX(MAX(lb1,lb2), MAX(lb3,lb4));
		if (maxxLB > rangeSearch)
			return DBL_MAX;
		if (maxxLB > maxLB)
			maxLB = maxxLB;
		dist_query_pivots += 4;
		pivot_table += 4;
		numA--;
	}
	int64_t numB = state->numPivotsMod4;
	while (numB > 0) {
		double lb = fabs(dist_query_pivots[0] - pivot_table[0]);
		if (lb > rangeSearch)
			return DBL_MAX;
		else if (lb > maxLB)
			maxLB = lb;
		dist_query_pivots += 1;
		pivot_table += 1;
		numB--;
	}
	return maxLB;
}
static void laesa_resolveSearch_exact(void *query, struct LAESA_Search *state,
		int64_t current_thread) {
	double *dist_query_pivots = state->dist_query_pivots[current_thread];
	MknnDistanceEval *distance_eval = state->dist_evals[current_thread];
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	mknn_heap_reset(heapNNs);
	double rangeSearch = state->range;
	int64_t num_database_objects = mknn_dataset_getNumObjects(
			state->state_index->search_dataset);
	int64_t cont_discarded = 0;
	for (int64_t i = 0; i < num_database_objects; ++i) {
		if (laesa_tryToDiscard(i, dist_query_pivots, rangeSearch, state)) {
			cont_discarded++;
			continue;
		}
		void *obj = mknn_dataset_getObject(state->state_index->search_dataset,
				i);
		double dist = mknn_distanceEval_evalTh(distance_eval, query, obj,
				rangeSearch);
		mknn_heap_storeBestDistances(dist, i, heapNNs, &rangeSearch);
	}
	state->dist_evaluations[current_thread] += num_database_objects
			- cont_discarded;
}
static void laesa_resolveSearch_onlyLB(void *query, struct LAESA_Search *state,
		int64_t current_thread) {
	double *dist_query_pivots = state->dist_query_pivots[current_thread];
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	mknn_heap_reset(heapNNs);
	double rangeLowerBound = state->range;
	int64_t num_database_objects = mknn_dataset_getNumObjects(
			state->state_index->search_dataset);
	for (int64_t i = 0; i < num_database_objects; ++i) {
		double maxLB = laesa_computeMaxLB(i, dist_query_pivots, rangeLowerBound,
				state);
		mknn_heap_storeBestDistances(maxLB, i, heapNNs, &rangeLowerBound);
	}
}
static void laesa_resolveSearch_approx(void *query, struct LAESA_Search *state,
		int64_t current_thread) {
	double *dist_query_pivots = state->dist_query_pivots[current_thread];
	MknnDistanceEval *distance_eval = state->dist_evals[current_thread];
	MknnHeap *heapLBs = state->heapsLBs[current_thread];
	mknn_heap_reset(heapLBs);
	double rangeLowerBound = state->range;
	int64_t num_database_objects = mknn_dataset_getNumObjects(
			state->state_index->search_dataset);
	for (int64_t i = 0; i < num_database_objects; ++i) {
		double maxLB = laesa_computeMaxLB(i, dist_query_pivots, rangeLowerBound,
				state);
		mknn_heap_storeBestDistances(maxLB, i, heapLBs, &rangeLowerBound);
	}
//evaluate actual distance for the lowest LBs
	int64_t length = mknn_heap_getSize(heapLBs);
	double rangeSearch = state->range;
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	mknn_heap_reset(heapNNs);
	int64_t cont_discarded_by_lb = 0;
	for (int64_t n = 0; n < length; ++n) {
		if (mknn_heap_getDistanceAtPosition(heapLBs, n) > rangeSearch) {
			cont_discarded_by_lb++;
			continue;
		}
		int64_t object_id = mknn_heap_getObjectIdAtPosition(heapLBs, n);
		void *obj = mknn_dataset_getObject(state->state_index->search_dataset,
				object_id);
		double dist = mknn_distanceEval_evalTh(distance_eval, query, obj,
				rangeSearch);
		mknn_heap_storeBestDistances(dist, object_id, heapNNs, &rangeSearch);
	}
	state->dist_evaluations[current_thread] += length - cont_discarded_by_lb;
}

#ifndef NO_FLANN
static void laesa_resolveSearch_approxFlann(void *query,
		struct LAESA_Search *state, int64_t current_thread) {
	int *nns_ids = state->fnn_ids[current_thread];
	double *nns_dists_f = state->fnn_dists[current_thread];
	double *dist_query_pivots = state->dist_query_pivots[current_thread];
	flann_find_nearest_neighbors_index_double(state->fnn_index,
			dist_query_pivots, 1, nns_ids, nns_dists_f, state->approx_size,
			&state->fnn_parameters);
//evaluate actual distance for the lowest LBs
	MknnDistanceEval *distance_eval = state->dist_evals[current_thread];
	MknnHeap *heapNNs = state->heapsNNs[current_thread];
	mknn_heap_reset(heapNNs);
	double rangeSearch = state->range;
	int64_t num_database_objects = mknn_dataset_getNumObjects(
			state->state_index->search_dataset);
	for (int64_t j = 0; j < state->approx_size; ++j) {
		int pos = nns_ids[j];
		my_assert_indexRangeInt("flann pos", pos, num_database_objects);
		void *obj = mknn_dataset_getObject(state->state_index->search_dataset,
				pos);
		double dist = mknn_distanceEval_evalTh(distance_eval, query, obj,
				rangeSearch);
		mknn_heap_storeBestDistances(dist, j, heapNNs, &rangeSearch);
	}
	state->dist_evaluations[current_thread] += state->approx_size;
}
#endif
static void laesa_computeDistQueryToPivots(void *query,
		struct LAESA_Search *state, int64_t current_thread) {
	MknnDistanceEval *distance = state->dist_evals[current_thread];
	double *dist_query_pivots = state->dist_query_pivots[current_thread];
	for (int64_t id_piv = 0; id_piv < state->state_index->num_pivots;
			++id_piv) {
		void *piv = state->state_index->pivots[id_piv];
		dist_query_pivots[id_piv] = mknn_distanceEval_eval(distance, query,
				piv);
	}
}
static void laesa_resolveSearch_query(int64_t current_process,
		void *state_object, int64_t current_thread) {
	struct LAESA_Search *state = state_object;
	void *query = mknn_dataset_getObject(state->query_dataset, current_process);
	laesa_computeDistQueryToPivots(query, state, current_thread);
	state->dist_evaluations[current_thread] = 0;
	if (state->method == METHOD_EXACT_SEARCH) {
		laesa_resolveSearch_exact(query, state, current_thread);
	} else if (state->method == METHOD_LB_ONLY) {
		laesa_resolveSearch_onlyLB(query, state, current_thread);
	} else if (state->method == METHOD_APPROX_SEARCH) {
		laesa_resolveSearch_approx(query, state, current_thread);
	}
#ifndef NO_FLANN
	if (state->method == METHOD_APPROX_SEARCH_USING_FLANN) {
		laesa_resolveSearch_approxFlann(query, state, current_thread);
	}
#endif
	state->dist_evaluations[current_thread] += state->state_index->num_pivots;
	mknn_result_storeMatchesInResultQuery(state->result, current_process,
			state->heapsNNs[current_thread],
			state->dist_evaluations[current_thread]);
}

static MknnResult *laesa_resolver_search(void *state_resolver,
		MknnDataset *query_dataset) {
	struct LAESA_Search *state = state_resolver;
	int64_t num_query_objects = mknn_dataset_getNumObjects(query_dataset);
	state->query_dataset = query_dataset;
	state->result = mknn_result_newEmpty(num_query_objects, state->knn);
	state->dist_evals = mknn_distance_createDistEvalArray(
			state->state_index->distance, state->max_threads,
			mknn_dataset_getDomain(query_dataset),
			mknn_dataset_getDomain(state->state_index->search_dataset));
	my_parallel_incremental(num_query_objects, state, laesa_resolveSearch_query,
			"laesa search", state->max_threads);
	mknn_distanceEval_releaseArray(state->dist_evals, state->max_threads);
	return state->result;
}

static void laesa_resolver_release(void *state_resolver) {
	struct LAESA_Search *state = state_resolver;
	MY_FREE(state->dist_evaluations);
	MY_FREE_MATRIX(state->dist_query_pivots, state->max_threads);
	mknn_heap_releaseMulti(state->heapsNNs, state->max_threads);
	if (state->method == METHOD_APPROX_SEARCH) {
		mknn_heap_releaseMulti(state->heapsLBs, state->max_threads);
	}
#ifndef NO_FLANN
	if (state->method == METHOD_APPROX_SEARCH_USING_FLANN) {
		MY_FREE(state->fnn_datatable);
		flann_free_index(state->fnn_index, &state->fnn_parameters);
		MY_FREE_MATRIX(state->fnn_ids, state->max_threads);
		MY_FREE_MATRIX(state->fnn_dists, state->max_threads);
	}
#endif
	MY_FREE(state);
}
struct MknnResolverInstance laesa_resolver_new(void *state_index,
		const char *id_index, MknnResolverParams *params_resolver) {
	struct LAESA_Search *state = MY_MALLOC(1, struct LAESA_Search);
	state->state_index = state_index;
	state->knn = mknn_resolverParams_getKnn(params_resolver);
	if (state->knn < 1)
		state->knn = 1;
	state->range = mknn_resolverParams_getRange(params_resolver);
	if (state->range == 0)
		state->range = DBL_MAX;
	state->max_threads = mknn_resolverParams_getMaxThreads(params_resolver);
	if (state->max_threads < 1)
		state->max_threads = my_parallel_getNumberOfCores();
	state->numPivotsDiv4 = state->state_index->num_pivots / 4;
	state->numPivotsMod4 = state->state_index->num_pivots % 4;
	const char *name_method = mknn_resolverParams_getString(params_resolver,
			"method");
	if (name_method == NULL
			|| my_string_equals_ignorecase("EXACT", name_method)) {
		state->method = METHOD_EXACT_SEARCH;
	} else if (my_string_equals_ignorecase("LB_ONLY", name_method)) {
		state->method = METHOD_LB_ONLY;
	} else if (my_string_equals_ignorecase("APPROX", name_method)) {
		state->method = METHOD_APPROX_SEARCH;
#ifndef NO_FLANN
	} else if (my_string_equals_ignorecase("L1APPROXFLANN", name_method)) {
		state->method = METHOD_APPROX_SEARCH_USING_FLANN;
#endif
	} else {
		my_log_info("unknown method %s\n", name_method);
		mknn_predefIndex_helpPrintIndex(id_index);
	}
	state->approx_pct = mknn_resolverParams_getDouble(params_resolver,
			"approximation");
	int64_t num_objects = mknn_dataset_getNumObjects(
			state->state_index->search_dataset);
	state->approx_size = num_objects * state->approx_pct;
	state->approx_size = MAX(state->approx_size, 1);
	state->dist_evaluations = MY_MALLOC_NOINIT(state->max_threads, int64_t);
	state->dist_query_pivots = MY_MALLOC_MATRIX(state->max_threads,
			state->state_index->num_pivots, double);
	state->heapsNNs = mknn_heap_newMultiMaxHeap(state->knn, state->max_threads);
	if (state->method == METHOD_APPROX_SEARCH) {
		state->heapsLBs = mknn_heap_newMultiMaxHeap(state->approx_size,
				state->max_threads);
	}
#ifndef NO_FLANN
	else if (state->method == METHOD_APPROX_SEARCH_USING_FLANN) {
		int64_t num_pivots = state->state_index->num_pivots;
		state->fnn_datatable = MY_MALLOC_NOINIT(num_objects * num_pivots,
				double);
		for (int64_t j = 0; j < num_objects; ++j) {
			for (int64_t id_piv = 0; id_piv < num_pivots; ++id_piv) {
				double d = state->state_index->pivot_table[j][id_piv];
				state->fnn_datatable[j * num_pivots + id_piv] = d;
			}
		}
		flann_set_distance_type(FLANN_DIST_L1, 0);
		struct FLANNParameters params = DEFAULT_FLANN_PARAMETERS;
		params.algorithm = FLANN_INDEX_LINEAR;
		params.log_level = FLANN_LOG_INFO;
		float speedup = 0;
		state->fnn_index = flann_build_index_double(state->fnn_datatable,
				num_objects, num_pivots, &speedup, &params);
		state->fnn_parameters = params;
		state->fnn_ids = MY_MALLOC_MATRIX(state->max_threads,
				state->approx_size, int);
		state->fnn_dists = MY_MALLOC_MATRIX(state->max_threads,
				state->approx_size, double);
	}
#endif
	struct MknnResolverInstance newResolver = { 0 };
	newResolver.state_resolver = state;
	newResolver.func_resolver_search = laesa_resolver_search;
	newResolver.func_resolver_release = laesa_resolver_release;
	return newResolver;
}

void register_index_laesa() {
	metricknn_register_index("LAESA", "num_pivots=[int],sets_eval=[int]",
			"method=[EXACT|APPROX|L1APPROXFLANN|LB_ONLY],approximation=[percentage]",
			NULL, laesa_index_new, laesa_resolver_new);
}

