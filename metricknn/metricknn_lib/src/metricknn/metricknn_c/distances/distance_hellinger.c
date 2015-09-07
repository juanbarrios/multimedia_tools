/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#define SQUARED(x) ( ((double) (x)) * ((double) (x)) )

#define HELLINGER_FUNCTIONS(typeVector1, typeVector2) \
static double hellinger_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	MknnDomain *domain_object = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = mknn_domain_vector_getNumDimensions(domain_object) / 4; \
	double current_max_squared = (current_threshold * current_threshold) * 2; \
	double sum = 0; \
	while (numN > 0) { \
		double dif0 = sqrt((double) array1[0]) - sqrt((double) array2[0]); \
		double dif1 = sqrt((double) array1[1]) - sqrt((double) array2[1]); \
		double dif2 = sqrt((double) array1[2]) - sqrt((double) array2[2]); \
		double dif3 = sqrt((double) array1[3]) - sqrt((double) array2[3]); \
		sum += SQUARED(dif0) + SQUARED(dif1) + SQUARED(dif2) + SQUARED(dif3); \
		if (sum > current_max_squared) \
			return current_threshold * 2; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = mknn_domain_vector_getNumDimensions(domain_object) % 4; \
	while (numN > 0) { \
		double dif0 = sqrt((double) array1[0]) - sqrt((double) array2[0]); \
		sum += SQUARED(dif0) ; \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sqrt(sum / 2); \
}

GENERATE_DOUBLE_DATATYPE(HELLINGER_FUNCTIONS)

static struct MknnDistEvalInstance hellinger_distanceEval_new(void *state_distance,
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
			hellinger_distanceEval_)
	return di;
}
static struct MknnDistanceInstance hellinger_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct MknnDistanceInstance df = { 0 };
	df.func_distanceEval_new = hellinger_distanceEval_new;
	return df;
}

void register_distance_hellinger() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "HELLINGER", NULL,
	NULL, hellinger_dist_new);
}
