/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct SnakeTable_Index {
	int64_t default_num_pivots;
	MknnDataset *search_dataset;
	MknnDistance *distance;
};

static void snaketable_index_release(void *state_index) {
	struct SnakeTable_Index *state = state_index;
	MY_FREE(state);
}
static struct MknnIndexInstance snaketable_index_new(const char *id_index,
		MknnIndexParams *params_index, MknnDataset *search_dataset,
		MknnDistance *distance) {
	struct SnakeTable_Index *state = MY_MALLOC(1, struct SnakeTable_Index);
	state->default_num_pivots = mknn_indexParams_getInt(params_index,
			"num_pivots");
	state->search_dataset = search_dataset;
	state->distance = distance;
	struct MknnIndexInstance newIdx = { 0 };
	newIdx.state_index = state;
	newIdx.func_index_build = NULL;
	newIdx.func_index_load = NULL;
	newIdx.func_index_save = NULL;
	newIdx.func_index_release = snaketable_index_release;
	return newIdx;
}
/***************************************/
struct RecordSnakeTable {
	int64_t querySID;
	double distance;
};
struct SnakeTable_SearchThread {
	MknnDistanceEval *distEval_query2query;
	MknnDistanceEval *distEval_query2ref;
	MknnHeap *heapNNs;
	int64_t dist_evaluations, max_query_objects;
	struct RecordSnakeTable **pivot_table;
	void **dynamic_pivots;
	void **all_pivots;
	double *dist2pivot;
	int64_t *pivotSID;
	int64_t num_pivots, pos_new_pivot, id_last_not_pivot;
	bool *flag_computed_dist2pivots;
	int64_t *num_pivots_reg, *pos_last_reg;
	struct SnakeTable_Search *state;
};

typedef void (*mknn_func_snaketable_start_searches)(
		struct SnakeTable_SearchThread *st);
typedef void (*mknn_func_snaketable_resolve_search)(void *query,
		int64_t querySID, struct SnakeTable_SearchThread *st);
typedef void (*mknn_func_snaketable_end_searches)(
		struct SnakeTable_SearchThread *st);

struct SnakeTable_Search {
	struct SnakeTable_Index *index;
	int64_t maxDynPivots, knn, num_objects, max_threads;
	double range;
	MknnDataset *query_dataset_full;
	MknnResult *result;
	struct SnakeTable_SearchThread *search_threads;
	mknn_func_snaketable_start_searches func_start_searches;
	mknn_func_snaketable_resolve_search func_resolve_search;
	mknn_func_snaketable_end_searches func_end_searches;
};

static double computeActualDistance(void *query, void *object, int64_t obj_id,
		double *rangeSearch_ptr, struct SnakeTable_SearchThread *st) {
	double distance = mknn_distanceEval_evalTh(st->distEval_query2ref, query,
			object, *rangeSearch_ptr);
	mknn_heap_storeBestDistances(distance, obj_id, st->heapNNs,
			rangeSearch_ptr);
	st->dist_evaluations++;
	return distance;
}
/**************************/
static void snakeV1_start_searches(struct SnakeTable_SearchThread *st) {
	st->dynamic_pivots = MY_MALLOC(st->state->maxDynPivots, void*);
	st->dist2pivot = MY_MALLOC(st->state->maxDynPivots, double);
	st->pivotSID = MY_MALLOC(st->state->maxDynPivots, int64_t);
	st->num_pivots = st->pos_new_pivot = 0;
	st->id_last_not_pivot = -1;
}
static void snakeV1_end_searches(struct SnakeTable_SearchThread *st) {
	MY_FREE_MULTI(st->dynamic_pivots, st->dist2pivot, st->pivotSID);
}
static bool snakeV1_tryToDiscard(int64_t objSID, double rangeSearch,
		struct SnakeTable_SearchThread *st) {
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	for (int64_t idPiv = 0; idPiv < st->num_pivots; ++idPiv) {
		if (records[idPiv].querySID != st->pivotSID[idPiv]) {
			continue;
		}
		double lowerBoundDistance = fabs(
				st->dist2pivot[idPiv] - records[idPiv].distance);
		if (lowerBoundDistance > rangeSearch) {
			return 1;
		}
	}
	return 0;
}
static void snakeV1_addDistanceToTable(int64_t querySID, int64_t objSID,
		double distance, struct SnakeTable_SearchThread *st) {
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	records[st->pos_new_pivot].querySID = querySID;
	records[st->pos_new_pivot].distance = distance;
}
static void snakeV1_registerNewPivot(int64_t querySID, void *query,
		struct SnakeTable_SearchThread *st) {
	if (st->dynamic_pivots[st->pos_new_pivot] != NULL)
		st->id_last_not_pivot = st->pivotSID[st->pos_new_pivot];
	st->dynamic_pivots[st->pos_new_pivot] = query;
	st->pivotSID[st->pos_new_pivot] = querySID;
	if (st->num_pivots < st->state->maxDynPivots)
		st->num_pivots++;
	st->pos_new_pivot =
			(st->pos_new_pivot == st->state->maxDynPivots - 1) ?
					0 : st->pos_new_pivot + 1;
}
static void snakeV1_resolve_search(void *query, int64_t querySID,
		struct SnakeTable_SearchThread *st) {
	for (int64_t idPiv = 0; idPiv < st->num_pivots; ++idPiv) {
		st->dist2pivot[idPiv] = mknn_distanceEval_eval(st->distEval_query2query,
				query, st->dynamic_pivots[idPiv]);
	}
	st->dist_evaluations += st->num_pivots;
	double rangeSearch = st->state->range;
	for (int64_t objSID = 0; objSID < st->state->num_objects; ++objSID) {
		void *object = mknn_dataset_getObject(st->state->index->search_dataset,
				objSID);
		if (snakeV1_tryToDiscard(objSID, rangeSearch, st))
			continue;
		//not discarded
		double distance = computeActualDistance(query, object, objSID,
				&rangeSearch, st);
		//insert distance
		snakeV1_addDistanceToTable(querySID, objSID, distance, st);
	}
	snakeV1_registerNewPivot(querySID, query, st);
}
/**************************/
static void snakeV2_start_searches(struct SnakeTable_SearchThread *st) {
	MY_REALLOC(st->dist2pivot, st->max_query_objects, double);
	MY_REALLOC(st->flag_computed_dist2pivots, st->max_query_objects, bool);
	MY_REALLOC(st->all_pivots, st->max_query_objects, void*);
}
static void snakeV2_end_searches(struct SnakeTable_SearchThread *st) {
	MY_FREE_MULTI(st->dist2pivot, st->flag_computed_dist2pivots);
}

static bool snakeV2_tryToDiscard(void *query, int64_t objSID,
		double rangeSearch, int64_t *out_pos_new_pivot,
		struct SnakeTable_SearchThread *st) {
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	const bool isMin = true, isMax = false;
	int64_t pos_best = -1;
	double dist_best = isMin ? DBL_MAX : (isMax ? -DBL_MAX : 0);
	for (int64_t idPiv = 0; idPiv < st->num_pivots; ++idPiv) {
		int64_t pivSID = records[idPiv].querySID;
		if (pivSID < 0) {
			pos_best = idPiv;
			dist_best = 0;
			continue;
		}
		if (!st->flag_computed_dist2pivots[pivSID]) {
			void *piv = st->all_pivots[pivSID];
			st->dist2pivot[pivSID] = mknn_distanceEval_eval(
					st->distEval_query2query, query, piv);
			st->flag_computed_dist2pivots[pivSID] = 1;
			st->dist_evaluations++;
		}
		double lowerBoundDistance = fabs(
				st->dist2pivot[pivSID] - records[idPiv].distance);
		if (lowerBoundDistance > rangeSearch) {
			return 1;
		}
		if (pos_best < 0 || (isMin && dist_best < records[idPiv].distance)
				|| (isMax && dist_best > records[idPiv].distance)) {
			dist_best = records[idPiv].distance;
			pos_best = idPiv;
		}
	}
	*out_pos_new_pivot = pos_best;
	return 0;
}
static void snakeV2_addDistanceToTable(int64_t querySID, int64_t objSID,
		double distance, int64_t pos_new_pivot,
		struct SnakeTable_SearchThread *st) {
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	if (st->num_pivots < st->state->maxDynPivots)
		pos_new_pivot = st->num_pivots;
	records[pos_new_pivot].querySID = querySID;
	records[pos_new_pivot].distance = distance;
}
static void snakeV2_resolve_search(void *query, int64_t querySID,
		struct SnakeTable_SearchThread *st) {
	MY_SETZERO(st->flag_computed_dist2pivots, st->max_query_objects, bool);
	st->all_pivots[querySID] = query;
	double rangeSearch = st->state->range;
	for (int64_t objSID = 0; objSID < st->state->num_objects; ++objSID) {
		void *obj = mknn_dataset_getObject(st->state->index->search_dataset,
				objSID);
		int64_t pos_new_pivot = 0;
		if (snakeV2_tryToDiscard(query, objSID, rangeSearch, &pos_new_pivot,
				st))
			continue;
		//not discarded
		double distance = computeActualDistance(query, obj, objSID,
				&rangeSearch, st);
		//insert distance
		snakeV2_addDistanceToTable(querySID, objSID, distance, pos_new_pivot,
				st);
	}
	if (st->num_pivots < st->state->maxDynPivots)
		st->num_pivots++;
}
/**************************/
static void snakeV3_start_searches(struct SnakeTable_SearchThread *st) {
	//MY_REALLOC(st->dist2pivot, st->max_query_objects, double);
	//MY_REALLOC(st->flag_computed_dist2pivots, st->max_query_objects, bool);
	//MY_REALLOC(st->all_pivots, st->max_query_objects, void*);
	//MY_REALLOC(st->num_pivots_reg, st->num_objects, int64_t);
	//MY_REALLOC(st->pos_last_reg, st->num_objects, int64_t);
	st->dist2pivot = MY_MALLOC(st->max_query_objects, double);
	st->flag_computed_dist2pivots = MY_MALLOC(st->max_query_objects, bool);
	st->all_pivots = MY_MALLOC(st->max_query_objects, void*);
	st->num_pivots_reg = MY_MALLOC(st->state->num_objects, int64_t);
	st->pos_last_reg = MY_MALLOC(st->state->num_objects, int64_t);
}
static void snakeV3_end_searches(struct SnakeTable_SearchThread *st) {
	MY_FREE_MULTI(st->dist2pivot, st->flag_computed_dist2pivots,
			st->num_pivots_reg, st->pos_last_reg);
	MY_FREE(st->all_pivots);
}
static bool snakeV3_tryToDiscard(void *query, int64_t objSID,
		double rangeSearch, struct SnakeTable_SearchThread *st) {
	int64_t num_pivs = st->num_pivots_reg[objSID];
	if (num_pivs == 0)
		return 0;
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	int64_t current_pos = st->pos_last_reg[objSID];
	int64_t pos_last = st->pos_last_reg[objSID];
	do {
		int64_t pivSID = records[current_pos].querySID;
		if (!st->flag_computed_dist2pivots[pivSID]) {
			void *piv = st->all_pivots[pivSID];
			st->dist2pivot[pivSID] = mknn_distanceEval_eval(
					st->distEval_query2query, query, piv);
			st->flag_computed_dist2pivots[pivSID] = 1;
			st->dist_evaluations++;
		}
		double lowerBoundDistance = fabs(
				st->dist2pivot[pivSID] - records[current_pos].distance);
		if (lowerBoundDistance > rangeSearch) {
			return true;
		}
		current_pos = (current_pos == 0) ? num_pivs - 1 : current_pos - 1;
	} while (current_pos != pos_last);
	return false;
}
static void snakeV3_addDistanceToTable(int64_t querySID, int64_t objSID,
		double distance, struct SnakeTable_SearchThread *st) {
	struct RecordSnakeTable *records = st->pivot_table[objSID];
	int64_t num_pivs = st->num_pivots_reg[objSID];
	if (num_pivs < st->state->maxDynPivots) {
		records[num_pivs].querySID = querySID;
		records[num_pivs].distance = distance;
		st->pos_last_reg[objSID] = num_pivs;
		st->num_pivots_reg[objSID]++;
	} else {
		int64_t new_pos = (st->pos_last_reg[objSID] + 1)
				% st->state->maxDynPivots;
		records[new_pos].querySID = querySID;
		records[new_pos].distance = distance;
		st->pos_last_reg[objSID] = new_pos;
	}
}
static void snakeV3_resolve_search(void *query, int64_t querySID,
		struct SnakeTable_SearchThread *st) {
	MY_SETZERO(st->flag_computed_dist2pivots, st->max_query_objects, bool);
	st->all_pivots[querySID] = query;
	double rangeSearch = st->state->range;
	for (int64_t objSID = 0; objSID < st->state->num_objects; ++objSID) {
		void *obj = mknn_dataset_getObject(st->state->index->search_dataset,
				objSID);
		if (snakeV3_tryToDiscard(query, objSID, rangeSearch, st))
			continue;
		//not discarded
		double distance = computeActualDistance(query, obj, objSID,
				&rangeSearch, st);
		//insert distance
		snakeV3_addDistanceToTable(querySID, objSID, distance, st);
	}
}
/**************************/
static void snaketable_resolver_query(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread) {
	struct SnakeTable_Search *state = state_object;
	MknnDataset *query_dataset = mknn_datasetLoader_SubsetSegment(
			state->query_dataset_full, start_process,
			end_process_notIncluded - start_process, false);
	struct SnakeTable_SearchThread *st = state->search_threads + current_thread;
	st->distEval_query2query = mknn_distance_newDistanceEval(
			state->index->distance, mknn_dataset_getDomain(query_dataset),
			mknn_dataset_getDomain(query_dataset));
	st->distEval_query2ref = mknn_distance_newDistanceEval(
			state->index->distance, mknn_dataset_getDomain(query_dataset),
			mknn_dataset_getDomain(state->index->search_dataset));
	struct RecordSnakeTable default_value = { .querySID = -1, .distance = -1 };
	for (int64_t m = 0; m < state->num_objects; ++m)
		for (int64_t n = 0; n < state->maxDynPivots; ++n)
			st->pivot_table[m][n] = default_value;
	st->max_query_objects = mknn_dataset_getNumObjects(query_dataset);
	state->func_start_searches(st);
	for (int64_t i = 0; i < st->max_query_objects; ++i) {
		mknn_heap_reset(st->heapNNs);
		st->dist_evaluations = 0;
		void *query = mknn_dataset_getObject(query_dataset, i);
		int64_t querySID = i;
		state->func_resolve_search(query, querySID, st);
		mknn_result_storeMatchesInResultQuery(state->result, start_process + i,
				st->heapNNs, st->dist_evaluations);
		if (lt != NULL)
			my_progress_add1(lt);
	}
	state->func_end_searches(st);
	mknn_dataset_release(query_dataset);
	mknn_distanceEval_release(st->distEval_query2query);
	mknn_distanceEval_release(st->distEval_query2ref);
}
static MknnResult *snaketable_resolver_search(void *state_resolver,
		MknnDataset *query_dataset) {
	struct SnakeTable_Search *state = state_resolver;
	int64_t num_query_objects = mknn_dataset_getNumObjects(query_dataset);
	state->query_dataset_full = query_dataset;
	state->result = mknn_result_newEmpty(num_query_objects, state->knn);
	my_parallel_buffered(num_query_objects, state, snaketable_resolver_query,
			"snake table", state->max_threads, 0);
	return state->result;
}

static void snaketable_resolver_release(void *state_resolver) {
	struct SnakeTable_Search *state = state_resolver;
	for (int64_t i = 0; i < state->max_threads; ++i) {
		struct SnakeTable_SearchThread *st = state->search_threads + i;
		mknn_heap_release(st->heapNNs);
		MY_FREE_MATRIX(st->pivot_table, state->num_objects);
	}
	MY_FREE(state->search_threads);
	MY_FREE(state);
}
//version==1 uses sparse rows and always resolves queries
//version==2 replaces the minimum lowerbound
//version==3 equal to V2 plus compact rows
static struct MknnResolverInstance snaketable_resolver_new(void *state_index,
		const char *id_index, MknnResolverParams *params_resolver) {
	struct SnakeTable_Search *state = MY_MALLOC(1, struct SnakeTable_Search);
	state->index = state_index;
	state->num_objects = mknn_dataset_getNumObjects(
			state->index->search_dataset);
	state->knn = mknn_resolverParams_getKnn(params_resolver);
	if (state->knn < 1)
		state->knn = 1;
	state->range = mknn_resolverParams_getRange(params_resolver);
	if (state->range == 0)
		state->range = DBL_MAX;
	state->max_threads = mknn_resolverParams_getMaxThreads(params_resolver);
	if (state->max_threads < 1)
		state->max_threads = my_parallel_getNumberOfCores();
	state->maxDynPivots = mknn_resolverParams_getInt(params_resolver,
			"num_pivots");
	if (state->maxDynPivots < 1)
		state->maxDynPivots = state->index->default_num_pivots;
	if (state->maxDynPivots < 1) {
		my_log_info("num_pivots must be greater than 0\n");
		mknn_predefIndex_helpPrintIndex(id_index);
	}
	int64_t version = mknn_resolverParams_getInt(params_resolver, "version");
	if (version == 1) {
		state->func_start_searches = snakeV1_start_searches;
		state->func_resolve_search = snakeV1_resolve_search;
		state->func_end_searches = snakeV1_end_searches;
	} else if (version == 2) {
		state->func_start_searches = snakeV2_start_searches;
		state->func_resolve_search = snakeV2_resolve_search;
		state->func_end_searches = snakeV2_end_searches;
	} else {
		state->func_start_searches = snakeV3_start_searches;
		state->func_resolve_search = snakeV3_resolve_search;
		state->func_end_searches = snakeV3_end_searches;
	}
	state->search_threads = MY_MALLOC(state->max_threads,
			struct SnakeTable_SearchThread);
	for (int64_t n = 0; n < state->max_threads; ++n) {
		struct SnakeTable_SearchThread *st = state->search_threads + n;
		st->state = state;
		st->heapNNs = mknn_heap_newMaxHeap(state->knn);
		st->pivot_table = MY_MALLOC_MATRIX(state->num_objects,
				state->maxDynPivots, struct RecordSnakeTable);
	}
	struct MknnResolverInstance newResolver = { 0 };
	newResolver.state_resolver = state;
	newResolver.func_resolver_search = snaketable_resolver_search;
	newResolver.func_resolver_release = snaketable_resolver_release;
	return newResolver;
}
void register_index_snaketable() {
	metricknn_register_index("SNAKETABLE", "num_pivots=[int]",
			"num_pivots=[int]", NULL, snaketable_index_new,
			snaketable_resolver_new);
}
