/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef METRICKNN_IMPL_H_
#define METRICKNN_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"
#include <myutils/myutils_c.h>

typedef struct MknnHeap MknnHeap;
typedef struct MknnHistogram MknnHistogram;

#include "impl/macros_util.h"
#include "impl/heap.h"
#include "impl/histogram.h"
#include "impl/mevaluation_wrapper.h"

/******************/

MknnDatatype mknn_datatype_convertMy2Mknn(const MyDatatype my_datatype);
MyDatatype mknn_datatype_convertMknn2My(const MknnDatatype mknn_datatype);

void mknn_dataset_setCompactVectors(MknnDataset *dataset,
		void *compactVectors_pointer, bool free_compactVectors_on_release);

void mknn_dataset_computeStatsVectors(MknnDataset *dataset,
		struct MyDataStatsCompute *stats);

struct MyDataStatsCompute *mknn_dataset_computeDataStats(MknnDataset *dataset);

/******************/
void mknn_dataset_printObjectsRaw(MknnDataset *dataset, FILE *out);

void mknn_dataset_printObjectsText(MknnDataset *dataset, FILE *out);

/******************/

typedef void (*mknn_function_printHelp)(const char *id);

/* *************** */
MknnDistanceEval **mknn_distance_createDistEvalArray(MknnDistance *distance,
		int64_t num_instances, MknnDomain *domain_left,
		MknnDomain *domain_right);
void mknn_distanceEval_releaseArray(MknnDistanceEval **dist_evals,
		int64_t num_instances);

/* *************** */

struct MknnDistEvalInstance {
	void *state_distEval;
	mknn_function_distanceEval_eval func_distanceEval_eval;
	mknn_function_distanceEval_releaseState func_distanceEval_release;
};

typedef struct MknnDistanceInstance (*mknn_function_distance_new)(
		const char *id_dist, MknnDistanceParams *params_distance);

typedef void (*mknn_function_distance_build)(void *state_distance,
		const char *id_dist, MknnDistanceParams *params_distance);

typedef void (*mknn_function_distance_load)(void *state_distance,
		const char *id_dist, MknnDistanceParams *params_distance,
		const char *filename_read);

typedef void (*mknn_function_distance_save)(void *state_distance,
		const char *id_dist, MknnDistanceParams *params_distance,
		const char *filename_write);

typedef void (*mknn_function_distance_release)(void *state_distance);

typedef struct MknnDistEvalInstance (*mknn_function_distanceEval_new)(
		void *state_distance, MknnDomain *domain_left,
		MknnDomain *domain_right);

struct MknnDistanceInstance {
	void *state_distance;
	mknn_function_distance_build func_distance_build;
	mknn_function_distance_load func_distance_load;
	mknn_function_distance_save func_distance_save;
	mknn_function_distance_release func_distance_release;
	mknn_function_distanceEval_new func_distanceEval_new;
};

void mknn_register_default_distances();

void mknn_register_distance(MknnGeneralDomain general_domain,
		const char *id_dist, const char *help_line,
		mknn_function_printHelp func_printHelp,
		mknn_function_distance_new func_distance_new);

/* **************** */

typedef struct MknnIndexInstance (*mknn_function_index_new)(
		const char *id_index, MknnIndexParams *params_index,
		MknnDataset *search_dataset, MknnDistance *distance);

typedef void (*mknn_function_index_build)(void *state_index,
		const char *id_index, MknnIndexParams *params_index);

typedef void (*mknn_function_index_load)(void *state_index,
		const char *id_index, MknnIndexParams *params_index,
		const char *filename_read);

typedef void (*mknn_function_index_save)(void *state_index,
		const char *id_index, MknnIndexParams *params_index,
		const char *filename_write);

typedef void (*mknn_function_index_release)(void *state_index);

/* **************** */

typedef struct MknnResolverInstance (*mknn_function_resolver_new)(
		void *state_index, const char *id_index,
		MknnResolverParams *parameters_resolver);

typedef MknnResult *(*mknn_function_resolver_search)(void *state_resolver,
		MknnDataset *query_dataset);

typedef void (*mknn_function_resolver_release)(void *state_resolver);

struct MknnIndexInstance {
	void *state_index;
	mknn_function_index_build func_index_build;
	mknn_function_index_load func_index_load;
	mknn_function_index_save func_index_save;
	mknn_function_index_release func_index_release;
};
struct MknnResolverInstance {
	void *state_resolver;
	mknn_function_resolver_search func_resolver_search;
	mknn_function_resolver_release func_resolver_release;
};

void mknn_register_default_indexes();

void metricknn_register_index(const char *id_index, const char *help_build,
		const char *help_search, mknn_function_printHelp func_printHelp,
		mknn_function_index_new func_index_new,
		mknn_function_resolver_new func_resolver_new);

MknnResolver *mknn_resolver_newInternal(
		struct MknnResolverInstance resolverInstance, MknnIndex *index,
		MknnResolverParams *parameters,
		bool free_parameters_on_resolver_release);

/* **************** */

MknnResult *mknn_result_newEmpty(int64_t num_queries, int64_t num_nn_max);

void mknn_result_updateTotalDistanceEvaluations(MknnResult *result);
void mknn_result_setTotalSearchTime(MknnResult *result,
		double total_search_time);
void mknn_result_setResolverQueryDataset(MknnResult *result,
		MknnResolver *resolver, MknnDataset *query_dataset,
		bool free_resolver_on_release,
		bool free_query_dataset_on_release);
void mknn_result_storeMatchesInResultQuery(MknnResult *result,
		int64_t num_query, MknnHeap *heap, int64_t cont_evaluations);

void mknn_sample_distances_multithread(MknnDataset *dataset_src,
		MknnDataset *dataset_dst, int64_t sample_size, MknnDistance *distance,
		int64_t num_threads, double *out_distances, int64_t *out_position_src,
		int64_t *out_position_dst);

MknnHistogram *mknn_computeDistHistogram(MknnDataset *dataset,
		int64_t num_samples, MknnDistance *distance, int64_t max_threads);

/**
 *
 * @param dataset
 * @param distance
 * @param distance_matrix an array with size numobjects*numobjects
 * @param max_threads
 */
void mknn_fillDistanceMatrix(MknnDataset *dataset, MknnDistance *distance,
		double *distance_matrix, int64_t max_threads);

/* **************** */

void mknn_laesa_select_pivots_sss(MknnDataset *dataset, MknnDistance *distance,
		int64_t num_pivots, int64_t num_sets_eval, int64_t max_threads,
		int64_t *selected_positions);

/* **************** */

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>
IplImage *mknn_dataset_convertToIplImage(MknnDataset *dataset);
#endif

#ifdef __cplusplus
}
#endif

#endif
