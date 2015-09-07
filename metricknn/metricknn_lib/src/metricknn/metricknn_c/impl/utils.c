/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

static void sample_random_pairs(MknnDataset *dataset_src,
		MknnDataset *dataset_dst, int64_t sample_size, int64_t *position_src,
		int64_t *position_dst) {
	int64_t size_src = mknn_dataset_getNumObjects(dataset_src);
	int64_t size_dst = mknn_dataset_getNumObjects(dataset_dst);
	my_random_intList(0, size_src, position_src, sample_size);
	my_random_intList(0, size_dst, position_dst, sample_size);
//When sampling pairs from the same dataset, a sample is
//ignored when the objects are the same.
//This avoids to over-sample distance "0"
	if (dataset_src == dataset_dst) {
		for (int64_t i = 0; i < sample_size; ++i) {
			while (position_src[i] == position_dst[i]) {
				position_src[i] = my_random_int(0, size_src);
				position_dst[i] = my_random_int(0, size_dst);
			}
		}
	}
}

struct P_HistParamThread {
	MknnDistanceEval **dist_evals;
	MknnDataset *dataset_src;
	MknnDataset *dataset_dst;
	int64_t **local_buffer_positions_src, **local_buffer_positions_dst;
	int64_t *sample_position_src, *sample_position_dst;
	double *sample_distance;
};
static void priv_sampleDistances_thread(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread) {
	struct P_HistParamThread *param = state_object;
	int64_t length = end_process_notIncluded - start_process;
	int64_t *pos_src =
			(param->sample_position_src == NULL) ?
					param->local_buffer_positions_src[current_thread] :
					param->sample_position_src + start_process;
	int64_t *pos_dst =
			(param->sample_position_dst == NULL) ?
					param->local_buffer_positions_dst[current_thread] :
					param->sample_position_dst + start_process;
	sample_random_pairs(param->dataset_src, param->dataset_dst, length, pos_src,
			pos_dst);
	MknnDistanceEval *distance_eval = param->dist_evals[current_thread];
	double *dist_buffer = param->sample_distance + start_process;
	for (int64_t i = 0; i < length; ++i) {
		void *obj_src = mknn_dataset_getObject(param->dataset_src, pos_src[i]);
		void *obj_dst = mknn_dataset_getObject(param->dataset_dst, pos_dst[i]);
		double d = mknn_distanceEval_eval(distance_eval, obj_src, obj_dst);
		dist_buffer[i] = d;
	}
}
#define BUFFER_SIZE 1024

//out_distances is the distance of the samples pair. cannot be NULL.
//out_position_src is the first element of the sampled pair. can be NULL.
//out_position_dst is the second element of the sampled pair. can be NULL.
void mknn_sample_distances_multithread(MknnDataset *dataset_src,
		MknnDataset *dataset_dst, int64_t sample_size, MknnDistance *distance,
		int64_t max_threads, double *out_distances, int64_t *out_position_src,
		int64_t *out_position_dst) {
	struct P_HistParamThread param = { 0 };
	param.dataset_src = dataset_src;
	param.dataset_dst = dataset_dst;
	param.dist_evals = mknn_distance_createDistEvalArray(distance, max_threads,
			mknn_dataset_getDomain(dataset_src),
			mknn_dataset_getDomain(dataset_dst));
	if (out_position_src == NULL)
		param.local_buffer_positions_src = MY_MALLOC_MATRIX(max_threads,
				BUFFER_SIZE, int64_t);
	if (out_position_dst == NULL)
		param.local_buffer_positions_dst = MY_MALLOC_MATRIX(max_threads,
				BUFFER_SIZE, int64_t);
	param.sample_distance = out_distances;
	param.sample_position_src = out_position_src;
	param.sample_position_dst = out_position_dst;
	my_parallel_buffered(sample_size, &param, priv_sampleDistances_thread,
			"sample", max_threads, BUFFER_SIZE);
	mknn_distanceEval_releaseArray(param.dist_evals, max_threads);
	if (out_position_src == NULL)
		MY_FREE(param.local_buffer_positions_src);
	if (out_position_dst == NULL)
		MY_FREE(param.local_buffer_positions_dst);
}

/*******************************/
MknnHistogram *mknn_computeDistHistogram(MknnDataset *dataset,
		int64_t num_samples, MknnDistance *distance, int64_t max_threads) {
	my_log_info_time("computing histogram of distances (%"PRIi64" samples)\n",
			num_samples);
	double *samples = MY_MALLOC(num_samples, double);
	mknn_sample_distances_multithread(dataset, dataset, num_samples, distance,
			max_threads, samples, NULL, NULL);
	MknnHistogram *hist_distObjs = mknn_histogram_new(num_samples, samples, 1000);
	MY_FREE(samples);
	return hist_distObjs;
}
/*******************************/
struct P_MatrixDistances {
	int64_t n;
	MknnDataset *dataset;
	MknnDistanceEval **distEvals;
	double *matrix;
};
static void compute_matrix_row(int64_t current_process, void *state_object,
		int64_t current_thread) {
	struct P_MatrixDistances *state = state_object;
	int64_t row = current_process;
	void *vec1 = mknn_dataset_getObject(state->dataset, row);
	for (int64_t j = 0; j <= row; ++j) {
		void *vec2 = mknn_dataset_getObject(state->dataset, j);
		double d = mknn_distanceEval_eval(state->distEvals[current_thread],
				vec1, vec2);
		state->matrix[row * state->n + j] = d;
		state->matrix[j * state->n + row] = d; //symmetry
	}
}
void mknn_fillDistanceMatrix(MknnDataset *dataset, MknnDistance *distance,
		double *distance_matrix, int64_t max_threads) {
	struct P_MatrixDistances *state = MY_MALLOC(1, struct P_MatrixDistances);
	state->n = mknn_dataset_getNumObjects(dataset);
	state->dataset = dataset;
	my_log_info("computing matrix of distances (%"PRIi64" samples)\n",
			state->n);
	state->distEvals = mknn_distance_createDistEvalArray(distance, max_threads,
			mknn_dataset_getDomain(dataset), mknn_dataset_getDomain(dataset));
	state->matrix = distance_matrix;
	my_parallel_incremental(state->n, state, compute_matrix_row,
			"matrix of distances", max_threads);
	mknn_distanceEval_releaseArray(state->distEvals, max_threads);
	free(state);
}

