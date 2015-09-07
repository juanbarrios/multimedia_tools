/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#define SUM_NOT_EQUAL(val1, val2, var, typeDiff) if(((typeDiff)val1) != ((typeDiff)val2)){ var++; }

#define HAM_FUNCTIONS(typeVector1, typeVector2, typeDiff, funAbs) \
static double hamming_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	MknnDomain *domain_object = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = mknn_domain_vector_getNumDimensions(domain_object) / 4; \
	int64_t sum = 0; \
	while (numN > 0) { \
		SUM_NOT_EQUAL(array1[0], array2[0], sum, typeDiff) \
		SUM_NOT_EQUAL(array1[1], array2[1], sum, typeDiff) \
		SUM_NOT_EQUAL(array1[2], array2[2], sum, typeDiff) \
		SUM_NOT_EQUAL(array1[3], array2[3], sum, typeDiff) \
		if (sum > current_threshold) \
			return sum; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = mknn_domain_vector_getNumDimensions(domain_object) % 4; \
	while (numN > 0) { \
		SUM_NOT_EQUAL(array1[0], array2[0], sum, typeDiff) \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum; \
}

GENERATE_DOUBLE_DATATYPE_WITH_DIFF_ABS(HAM_FUNCTIONS)

static struct MknnDistEvalInstance hamming_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	int64_t dims1 = mknn_domain_vector_getNumDimensions(domain_left);
	int64_t dims2 = mknn_domain_vector_getNumDimensions(domain_right);
	if (dims1 != dims2)
		my_log_error(
				"distance does not support different number of dimensions (%"PRIi64"!=%"PRIi64")\n",
				dims1, dims2);
	MknnDatatype datatype1 = mknn_domain_vector_getDimensionDataType(domain_left);
	MknnDatatype datatype2 = mknn_domain_vector_getDimensionDataType(
			domain_right);
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = domain_left;
	ASSIGN_DOUBLE_DATATYPE(di.func_distanceEval_eval, datatype1, datatype2,
			hamming_distanceEval_)
	return di;
}
static struct MknnDistanceInstance hamming_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct MknnDistanceInstance df = { 0 };
	df.func_distanceEval_new = hamming_distanceEval_new;
	return df;
}

void register_distance_hamming() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "HAMMING", NULL,
	NULL, hamming_dist_new);
}
