/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct State_DPF_Dist {
	double order;
	int64_t dims_discard;
	double pct_discard;
	double threshold_discard;
};
struct State_DPF_Eval {
	struct State_DPF_Dist *state_dist;
	int64_t num_dim_vectors;
	int64_t num_dim_compute;
	void *diffs_buffer;
};

#define COMPARE_FUNCTIONS(typeBuff) \
static int compare_##typeBuff(const void *a, const void *b) { \
	typeBuff da = *(typeBuff*) a; \
	typeBuff db = *(typeBuff*) b; \
	if (da > db) \
		return -1; \
	else if (da < db) \
		return 1; \
	else \
		return 0; \
}

#define DPF_FUNCTIONS(typeVector1, typeVector2, typeBuff, funAbs) \
static void differences_##typeVector1##_##typeVector2(void *object_left, void *object_right, \
		void *buffer, int64_t dims) { \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	typeBuff *diffs = (typeBuff*) buffer; \
	int64_t numN = dims / 4; \
	while (numN > 0) { \
		diffs[0] = (typeBuff) funAbs(((typeBuff)array1[0]) - ((typeBuff)array2[0])); \
		diffs[1] = (typeBuff) funAbs(((typeBuff)array1[1]) - ((typeBuff)array2[1])); \
		diffs[2] = (typeBuff) funAbs(((typeBuff)array1[2]) - ((typeBuff)array2[2])); \
		diffs[3] = (typeBuff) funAbs(((typeBuff)array1[3]) - ((typeBuff)array2[3])); \
		array1 += 4; \
		array2 += 4; \
		diffs += 4; \
		numN--; \
	} \
	numN = dims % 4; \
	while (numN > 0) { \
		diffs[0] = (typeBuff) funAbs(((typeBuff)array1[0]) - ((typeBuff)array2[0])); \
		array1 += 1; \
		array2 += 1; \
		diffs += 1; \
		numN--; \
	} \
} \
static double dpf_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_DPF_Eval *state = state_distEval; \
	typeBuff *diffs = (typeBuff*) state->diffs_buffer; \
	differences_##typeVector1##_##typeVector2(object_left, object_right, diffs, state->num_dim_vectors); \
	qsort(diffs, state->num_dim_vectors, sizeof(typeBuff), compare_##typeBuff); \
	int64_t real_size = 0; \
	if (state->state_dist->threshold_discard > 0) { \
		while (real_size < state->num_dim_compute \
				&& diffs[real_size] < state->state_dist->threshold_discard) \
			real_size++; \
	} else { \
		real_size = state->num_dim_compute; \
	} \
	double p = state->state_dist->order; \
	double sum = 0; \
	if (p == 1) { \
		for (int64_t i = 0; i < real_size; ++i) \
			sum += diffs[i]; \
	} else if (p == 2) { \
		for (int64_t i = 0; i < real_size; ++i) \
			sum += ((double) diffs[i]) * ((double) diffs[i]); \
		sum = sqrt(sum); \
	} else if (p == 0.5) { \
		for (int64_t i = 0; i < real_size; ++i) \
			sum += sqrt(diffs[i]); \
		sum = sum * sum; \
	} else { \
		for (int64_t i = 0; i < real_size; ++i) \
			sum += pow(diffs[i], p); \
		sum = pow(sum, 1 / p); \
	} \
	return sum; \
}

COMPARE_FUNCTIONS(int32_t)
COMPARE_FUNCTIONS(int64_t)
COMPARE_FUNCTIONS(float)
COMPARE_FUNCTIONS(double)

GENERATE_DOUBLE_DATATYPE_WITH_DIFF_ABS(DPF_FUNCTIONS)

static void dpf_distanceEval_release(void *state_distEval) {
	struct State_DPF_Eval *state = state_distEval;
	MY_FREE(state->diffs_buffer);
	MY_FREE(state);
}
static struct MknnDistEvalInstance dpf_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	int64_t dims1 = mknn_domain_vector_getNumDimensions(domain_left);
	int64_t dims2 = mknn_domain_vector_getNumDimensions(domain_right);
	if (dims1 != dims2)
		my_log_error(
				"distance does not support different number of dimensions (%"PRIi64"!=%"PRIi64")\n",
				dims1, dims2);
	struct State_DPF_Eval *state_eval = MY_MALLOC(1, struct State_DPF_Eval);
	state_eval->state_dist = state_distance;
	state_eval->num_dim_vectors = MIN(dims1, dims2);
	if (state_eval->state_dist->dims_discard > 0) {
		state_eval->num_dim_compute = state_eval->num_dim_vectors
				- state_eval->state_dist->dims_discard;
	} else if (state_eval->state_dist->pct_discard > 0) {
		state_eval->num_dim_compute = my_math_round_int(
				(1 - state_eval->state_dist->pct_discard)
						* state_eval->num_dim_vectors);
	} else {
		state_eval->num_dim_compute = state_eval->num_dim_vectors;
	}
	state_eval->num_dim_compute = MAX(0,
			MIN(state_eval->num_dim_vectors, state_eval->num_dim_compute));
	if (state_eval->num_dim_compute == state_eval->num_dim_vectors
			&& state_eval->state_dist->threshold_discard <= 0)
		my_log_info(
				"Warning: DPF is not discarding dimensions. Adjust parameters.\n");
	state_eval->diffs_buffer = MY_MALLOC(state_eval->num_dim_vectors, double);
	MknnDatatype datatype1 = mknn_domain_vector_getDimensionDataType(domain_left);
	MknnDatatype datatype2 = mknn_domain_vector_getDimensionDataType(
			domain_right);
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state_eval;
	ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype1, datatype2,
			dpf_distanceEval_)
	di.func_distanceEval_release = dpf_distanceEval_release;
	return di;
}

static struct MknnDistanceInstance dpf_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_DPF_Dist *state_dist = MY_MALLOC(1, struct State_DPF_Dist);
	state_dist->order = mknn_distanceParams_getDouble(params_distance, "order");
	if (state_dist->order <= 0) {
		my_log_info("Parameter 'order' must be greater than 0\n");
		mknn_predefDistance_helpPrintDistance(id_dist);
	}
	state_dist->dims_discard = mknn_distanceParams_getInt(params_distance,
			"dims_discard");
	state_dist->pct_discard = mknn_distanceParams_getDouble(params_distance,
			"pct_discard");
	state_dist->threshold_discard = mknn_distanceParams_getDouble(
			params_distance, "threshold_discard");
	struct MknnDistanceInstance df = { 0 };
	df.state_distance = state_dist;
	df.func_distance_release = free;
	df.func_distanceEval_new = dpf_distanceEval_new;
	return df;
}
static void dpf_printHelp(const char *id_dist) {
	my_log_info(
			"  order=[float]              Required. The order p of the Lp distance.\n");
	my_log_info(
			"  dims_discard=[int]         Absolute number of dimensions to discard."
					" e.g. dims_discard=2, discards two dimensions with highest difference.\n");
	my_log_info(
			"  pct_discard=[float]        Percentage of dimensions to discard."
					" e.g. pct_discard=0.05 if the vector dimension is 100,"
					" discards 5 dimensions with highest difference, if the vector dimension is 200 discards 10 dimensions.\n");
	my_log_info(
			"  threshold_discard=[float]  Threshold to discard a dimension. All the dimensions whose difference is higher"
					" than threshold_discard are discarded.\n");
}
void register_distance_dpf() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "DPF",
			"order=[float],dims_discard=[int],pct_discard=[float],threshold_discard=[float]",
			dpf_printHelp, dpf_dist_new);
}

