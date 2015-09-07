/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

//Puzicha et al. 1997. Nonparametric Similarity Measures for Unsupervised Texture Segmentation and Image Retrieval.
//Rubner et al. 2001. Empirical Evaluation of Dissimilarity Measures for Color and Texture.

#define AVG(val1, val2) ( ( ((double) (val1)) + ((double) (val2)) ) / 2.0 )
#define SQUARED(x) ( ((double) (x)) * ((double) (x)) )
#define IF_SUM(val, avg, var) if(avg > 0){ var += SQUARED((val) - (avg)) / ((double) avg); }

#define CHI2_FUNCTIONS(typeVector1, typeVector2) \
static double chi2_distanceEval_##typeVector1##_##typeVector2(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	MknnDomain *domain_object = state_distEval; \
	typeVector1 *array1 = (typeVector1*) object_left; \
	typeVector2 *array2 = (typeVector2*) object_right; \
	int64_t numN = mknn_domain_vector_getNumDimensions(domain_object) / 4; \
	double sum = 0; \
	while (numN > 0) { \
		double avg0 = AVG(array1[0], array2[0]); \
		double avg1 = AVG(array1[1], array2[1]); \
		double avg2 = AVG(array1[2], array2[2]); \
		double avg3 = AVG(array1[3], array2[3]); \
		IF_SUM(array1[0], avg0, sum) \
		IF_SUM(array1[1], avg1, sum) \
		IF_SUM(array1[2], avg2, sum) \
		IF_SUM(array1[3], avg3, sum) \
		if (sum > current_threshold) \
			return sum; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = mknn_domain_vector_getNumDimensions(domain_object) % 4; \
	while (numN > 0) { \
		double avg0 = AVG(array1[0], array2[0]); \
		IF_SUM(array1[0], avg0, sum) \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum; \
}

GENERATE_DOUBLE_DATATYPE(CHI2_FUNCTIONS)

static struct MknnDistEvalInstance chi2_distanceEval_new(void *state_distance,
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
			chi2_distanceEval_)
	return di;
}
static struct MknnDistanceInstance chi2_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct MknnDistanceInstance df = { 0 };
	df.func_distanceEval_new = chi2_distanceEval_new;
	return df;
}

void register_distance_chi2() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "CHI2", NULL, NULL,
			chi2_dist_new);
}
