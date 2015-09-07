/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct State_LP_Dist {
	double order;
	bool isL2Squared;
};
struct State_LP_Eval {
	int64_t dimensionsDiv4, dimensionsMod4;
	struct State_LP_Dist *state_dist;
};

#define LP_FUNCTIONS(typeVector1, typeVector2, typeDiff, funAbs) \
static double L1_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	typeDiff sum = 0; \
	while (numN > 0) { \
		sum += ((typeDiff) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))) \
				+ ((typeDiff) funAbs(((typeDiff)array1[1]) - ((typeDiff)array2[1]))) \
				+ ((typeDiff) funAbs(((typeDiff)array1[2]) - ((typeDiff)array2[2]))) \
				+ ((typeDiff) funAbs(((typeDiff)array1[3]) - ((typeDiff)array2[3]))); \
		if (sum > current_threshold) \
			return sum; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		sum += ((typeDiff) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))); \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum; \
} \
static double L2_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	double current_max_squared = current_threshold * current_threshold; \
	typeDiff sum = 0; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff)array1[0]) - ((typeDiff)array2[0]); \
		typeDiff d1 = ((typeDiff)array1[1]) - ((typeDiff)array2[1]); \
		typeDiff d2 = ((typeDiff)array1[2]) - ((typeDiff)array2[2]); \
		typeDiff d3 = ((typeDiff)array1[3]) - ((typeDiff)array2[3]); \
		sum += d0 * d0 + d1 * d1 + d2 * d2 + d3 * d3; \
		if (sum > current_max_squared) \
			return current_threshold * 2; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff)array1[0]) - ((typeDiff)array2[0]); \
		sum += d0 * d0; \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sqrt(sum); \
} \
static double L2squared_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	typeDiff sum = 0; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff)array1[0]) - ((typeDiff)array2[0]); \
		typeDiff d1 = ((typeDiff)array1[1]) - ((typeDiff)array2[1]); \
		typeDiff d2 = ((typeDiff)array1[2]) - ((typeDiff)array2[2]); \
		typeDiff d3 = ((typeDiff)array1[3]) - ((typeDiff)array2[3]); \
		sum += d0 * d0 + d1 * d1 + d2 * d2 + d3 * d3; \
		if (sum > current_threshold) \
			return sum; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff)array1[0]) - ((typeDiff)array2[0]); \
		sum += d0 * d0; \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum; \
} \
static double Lmax_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	typeDiff maxDiff = 0; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))); \
		typeDiff d1 = ((typeDiff) funAbs(((typeDiff)array1[1]) - ((typeDiff)array2[1]))); \
		typeDiff d2 = ((typeDiff) funAbs(((typeDiff)array1[2]) - ((typeDiff)array2[2]))); \
		typeDiff d3 = ((typeDiff) funAbs(((typeDiff)array1[3]) - ((typeDiff)array2[3]))); \
		if (d0 > maxDiff) \
			maxDiff = d0; \
		if (d1 > maxDiff) \
			maxDiff = d1; \
		if (d2 > maxDiff) \
			maxDiff = d2; \
		if (d3 > maxDiff) \
			maxDiff = d3; \
		if (maxDiff > current_threshold) \
			return maxDiff; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		typeDiff d0 = ((typeDiff) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))); \
		if (d0 > maxDiff) \
			maxDiff = d0; \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return maxDiff; \
} \
static double L05_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	double sum = 0; \
	while (numN > 0) { \
		double d0 = funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0])); \
		double d1 = funAbs(((typeDiff)array1[1]) - ((typeDiff)array2[1])); \
		double d2 = funAbs(((typeDiff)array1[2]) - ((typeDiff)array2[2])); \
		double d3 = funAbs(((typeDiff)array1[3]) - ((typeDiff)array2[3])); \
		sum += sqrt(d0) + sqrt(d1) + sqrt(d2) + sqrt(d3); \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		double d0 = funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0])); \
		sum += sqrt(d0); \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum * sum; \
} \
static double LP_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	struct State_LP_Eval *state = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = state->dimensionsDiv4; \
	double sum = 0; \
	double p = state->state_dist->order; \
	double current_max_p = pow(current_threshold, p); \
	while (numN > 0) { \
		double d0 = ((double) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))); \
		double d1 = ((double) funAbs(((typeDiff)array1[1]) - ((typeDiff)array2[1]))); \
		double d2 = ((double) funAbs(((typeDiff)array1[2]) - ((typeDiff)array2[2]))); \
		double d3 = ((double) funAbs(((typeDiff)array1[3]) - ((typeDiff)array2[3]))); \
		sum += pow(d0, p) + pow(d1, p) + pow(d2, p) + pow(d3, p); \
		if (sum > current_max_p) \
			return current_threshold * 2; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = state->dimensionsMod4; \
	while (numN > 0) { \
		double d0 = ((double) funAbs(((typeDiff)array1[0]) - ((typeDiff)array2[0]))); \
		sum += pow(d0, p); \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return pow(sum, 1 / p); \
}

GENERATE_DOUBLE_DATATYPE_WITH_DIFF_ABS(LP_FUNCTIONS)

static struct MknnDistEvalInstance LP_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	int64_t dims1 = mknn_domain_vector_getNumDimensions(domain_left);
	int64_t dims2 = mknn_domain_vector_getNumDimensions(domain_right);
	if (dims1 != dims2)
		my_log_error(
				"distance does not support different number of dimensions (%"PRIi64"!=%"PRIi64")\n",
				dims1, dims2);
	struct State_LP_Dist *state_dist = state_distance;
	struct State_LP_Eval *state = MY_MALLOC(1, struct State_LP_Eval);
	state->state_dist = state_dist;
	state->dimensionsDiv4 = dims1 / 4;
	state->dimensionsMod4 = dims1 % 4;
	MknnDatatype datatype_l = mknn_domain_vector_getDimensionDataType(
			domain_left);
	MknnDatatype datatype_r = mknn_domain_vector_getDimensionDataType(
			domain_right);
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state;
	di.func_distanceEval_release = free;
	if (state_dist->isL2Squared) {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, L2squared_distanceEval_)
	} else if (state_dist->order == 1) {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, L1_distanceEval_)
	} else if (state_dist->order == 2) {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, L2_distanceEval_)
	} else if (state_dist->order == DBL_MAX) {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, Lmax_distanceEval_)
	} else if (state_dist->order == 0.5) {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, L05_distanceEval_)
	} else {
		ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype_l,
				datatype_r, LP_distanceEval_)
	}
	return di;
}
static struct MknnDistanceInstance LP_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_LP_Dist *state_dist = MY_MALLOC(1, struct State_LP_Dist);
	state_dist->order = mknn_distanceParams_getDouble(params_distance, "order");
	if (my_string_equals(id_dist, "L1")) {
		state_dist->order = 1;
	} else if (my_string_equals(id_dist, "L2")) {
		state_dist->order = 2;
	} else if (my_string_equals(id_dist, "L2SQUARED")) {
		state_dist->isL2Squared = true;
		state_dist->order = 2;
	} else if (my_string_equals(id_dist,
			"LMAX") || isinf(state_dist->order) || state_dist->order == DBL_MAX) {
		state_dist->order = DBL_MAX;
	}
	if (state_dist->order <= 0) {
		my_log_info("Parameter 'order' must be greater than 0\n");
		mknn_predefDistance_helpPrintDistance(id_dist);
	}
	struct MknnDistanceInstance df = { 0 };
	df.state_distance = state_dist;
	df.func_distanceEval_new = LP_distanceEval_new;
	return df;
}
static void LP_printHelp(const char *id_dist) {
	my_log_info(
			"  order=[float]              Required. The order p of the Lp distance.\n");
}
void register_distance_lp() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "L1", NULL, NULL,
			LP_dist_new);
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "L2", NULL, NULL,
			LP_dist_new);
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "LMAX", NULL, NULL,
			LP_dist_new);
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "LP", "order=[float]",
			LP_printHelp, LP_dist_new);
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "L2SQUARED", NULL, NULL,
			LP_dist_new);
}

