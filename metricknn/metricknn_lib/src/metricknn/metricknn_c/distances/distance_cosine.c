/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct State_Cosine_Dist {
	bool normalize_vectors;
	bool is_similarity;
};
struct State_Cosine_Eval {
	struct State_Cosine_Dist *state_dist;
	void *last_object_left, *last_object_right;
	double last_norm1, last_norm2;
	int64_t dimensionsDiv4, dimensionsMod4;
	mknn_function_distanceEval_eval cosine_similarity_eval;
};

#define NORM_FUNCTIONS(typeVector) \
static double euclid_norm_##typeVector(void *object, int64_t dimensionsDiv4, int64_t dimensionsMod4) { \
	typeVector *array = (typeVector*) object; \
	int64_t numN = dimensionsDiv4; \
	double sum = 0; \
	while (numN > 0) { \
		sum += ((double) array[0]) * ((double) array[0]) \
				+ ((double) array[1]) * ((double) array[1]) \
				+ ((double) array[2]) * ((double) array[2]) \
				+ ((double) array[3]) * ((double) array[3]); \
		array += 4; \
		numN--; \
	} \
	numN = dimensionsMod4; \
	while (numN > 0) { \
		sum += ((double) array[0]) * ((double) array[0]); \
		array += 1; \
		numN--; \
	} \
	return sqrt(sum); \
}

GENERATE_SINGLE_DATATYPE(NORM_FUNCTIONS)

#define COS_FUNCTIONS(typeVector1, typeVector2, typeDiff, funAbs) \
static double cosine_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_Cosine_Eval *state = state_distEval; \
	double norm_left; \
	if(!state->state_dist->normalize_vectors) { \
		norm_left = 1; \
	} else if(object_left == state->last_object_left) { \
		norm_left = state->last_norm1; \
	} else { \
		norm_left = euclid_norm_##typeVector1(object_left, state->dimensionsDiv4, state->dimensionsMod4); \
		state->last_norm1 = norm_left; \
		state->last_object_left = object_left; \
	} \
	if(norm_left == 0) \
		return 0; \
	double norm_right; \
	if(!state->state_dist->normalize_vectors) { \
		norm_right = 1; \
	} else if(object_right == state->last_object_right) { \
		norm_right = state->last_norm2; \
	} else { \
		norm_right = euclid_norm_##typeVector2(object_right, state->dimensionsDiv4, state->dimensionsMod4); \
		state->last_norm2 = norm_right; \
		state->last_object_right = object_right; \
	} \
	if(norm_right == 0) \
		return 0; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	double sum = 0; \
	while (numN > 0) { \
		sum += ((double) array1[0]) * ((double) array2[0]) \
				+ ((double) array1[1]) * ((double) array2[1]) \
				+ ((double) array1[2]) * ((double) array2[2]) \
				+ ((double) array1[3]) * ((double) array2[3]); \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	int64_t numB = state->dimensionsMod4; \
	while (numB > 0) { \
		sum += ((double) array1[0]) * ((double) array2[0]); \
		array1 += 1; \
		array2 += 1; \
		numB--; \
	} \
	return (sum / norm_left) / norm_right; \
}

GENERATE_DOUBLE_DATATYPE_WITH_DIFF_ABS(COS_FUNCTIONS)

static double cosineDistance_eval(void *state_distEval, void *object_left,
		void *object_right, double current_threshold) {
	struct State_Cosine_Eval *state = state_distEval;
	double cosine = state->cosine_similarity_eval(state_distEval, object_left,
			object_right, DBL_MAX);
	if (cosine == 0)
		return M_SQRT2;
	else
		return sqrt(2 * fabs(1 - cosine));
}
static struct MknnDistEvalInstance cosine_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	int64_t dims1 = mknn_domain_vector_getNumDimensions(domain_left);
	int64_t dims2 = mknn_domain_vector_getNumDimensions(domain_right);
	if (dims1 != dims2)
		my_log_error(
				"distance does not support different number of dimensions (%"PRIi64"!=%"PRIi64")\n",
				dims1, dims2);
	struct State_Cosine_Eval *state_eval = MY_MALLOC(1,
			struct State_Cosine_Eval);
	state_eval->state_dist = state_distance;
	state_eval->dimensionsDiv4 = dims1 / 4;
	state_eval->dimensionsMod4 = dims1 % 4;
	MknnDatatype datatype1 = mknn_domain_vector_getDimensionDataType(
			domain_left);
	MknnDatatype datatype2 = mknn_domain_vector_getDimensionDataType(
			domain_right);
	ASSIGN_DOUBLE_DATATYPE(state_eval->cosine_similarity_eval, datatype1,
			datatype2, cosine_distanceEval_)
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state_eval;
	di.func_distanceEval_release = free;
	if (state_eval->state_dist->is_similarity)
		di.func_distanceEval_eval = state_eval->cosine_similarity_eval;
	else
		di.func_distanceEval_eval = cosineDistance_eval;
	return di;
}
struct MknnDistanceInstance cosine_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_Cosine_Dist *state_dist = MY_MALLOC(1,
			struct State_Cosine_Dist);
	state_dist->normalize_vectors = mknn_distanceParams_getBool(params_distance,
			"normalize_vectors");
	state_dist->is_similarity =
			(strcmp("COSINE_SIMILARITY", id_dist) == 0) ? true : false;
	struct MknnDistanceInstance df = { 0 };
	df.state_distance = state_dist;
	df.func_distanceEval_new = cosine_distanceEval_new;
	return df;
}
static void cosine_printHelp(const char *id_dist) {
	my_log_info(
			"    normalize_vectors=[true|false]    Optional. Normalizes to euclidean norm 1 prior to each computation.\n");
}
void register_distance_cosine() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "COSINE_SIMILARITY",
			"normalize_vectors=[true|false]", cosine_printHelp,
			cosine_dist_new);
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "COSINE_DISTANCE",
			"normalize_vectors=[true|false]", cosine_printHelp,
			cosine_dist_new);
}
