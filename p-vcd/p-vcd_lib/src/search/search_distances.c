/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

static DescriptorType validateProfileSingleDtype(struct SearchProfile *profile,
		int64_t dtype) {
	my_assert_equalInt("numModalities", profile->colReference->numModalities,
			1);
	my_assert_equalInt("numModalities", profile->colQuery->numModalities, 1);
	DescriptorType td1 = profile->colReference->modalities[0];
	DescriptorType td2 = profile->colQuery->modalities[0];
	my_assert_equalInt("descriptor dtype", td1.dtype, dtype);
	my_assert_equalInt("descriptor dtype", td2.dtype, dtype);
	my_assert_equalInt("descriptor length", td1.array_length, td2.array_length);
	return td1;
}

struct SparseGeneral_Eval {
	int64_t array_length;
	float *array_left;
	float *array_right;
	MknnDistanceEval *array_distanceEval;
};
struct SparseGeneral_Factory {
	MknnDomain *array_domain;
	MknnDistance *array_distance;
};

static double sparseGeneral_eval(void *state_distEval, void *object_left,
		void *object_right, double current_threshold) {
	struct SparseGeneral_Eval *state = state_distEval;
	MySparseArray *sparse_left = object_left;
	MySparseArray *sparse_right = object_right;
	my_sparseArray_restoreArrayFloat(sparse_left, state->array_left,
			state->array_length);
	my_sparseArray_restoreArrayFloat(sparse_right, state->array_right,
			state->array_length);
	return mknn_distanceEval_evalTh(state->array_distanceEval,
			state->array_left, state->array_right, current_threshold);
}
static void sparseGeneral_releaseState(void *state_distEval) {
	struct SparseGeneral_Eval *state = state_distEval;
	mknn_distanceEval_release(state->array_distanceEval);
	free(state->array_left);
	free(state->array_right);
	free(state);
}
static void sparseGeneral_createDistEval(void *state_factory,
		MknnDomain *domain_left, MknnDomain *domain_right,
		void **out_state_distEval,
		mknn_function_distanceEval_eval *out_func_eval,
		mknn_function_distanceEval_releaseState *out_func_releaseState) {
	struct SparseGeneral_Factory *factory = state_factory;
	struct SparseGeneral_Eval *state = MY_MALLOC(1, struct SparseGeneral_Eval);
	state->array_length = mknn_domain_vector_getNumDimensions(
			factory->array_domain);
	state->array_distanceEval = mknn_distance_newDistanceEval(
			factory->array_distance, factory->array_domain,
			factory->array_domain);
	state->array_left = MY_MALLOC(state->array_length, float);
	state->array_right = MY_MALLOC(state->array_length, float);
	*out_state_distEval = state;
	*out_func_eval = sparseGeneral_eval;
	*out_func_releaseState = sparseGeneral_releaseState;
}
static void *sparseGeneral_newStateFactory(struct SearchProfile *profile) {
	DescriptorType td = validateProfileSingleDtype(profile, DTYPE_SPARSE_ARRAY);
	struct SparseGeneral_Factory *factory = MY_MALLOC(1,
			struct SparseGeneral_Factory);
	factory->array_domain = mknn_domain_newVector(td.array_length,
			MKNN_DATATYPE_FLOATING_POINT_32bits);
	factory->array_distance = mknn_distance_newPredefined(
			mknn_distanceParams_newParseString(profile->id_dist), true);
	return factory;
}
static void sparseGeneral_releaseStateFactory(void *state_factory) {
	struct SparseGeneral_Factory *factory = state_factory;
	mknn_distance_release(factory->array_distance);
	free(factory);
}

static double sparseCosine_eval(void *state_distEval, void *object_left,
		void *object_right, double current_threshold) {
	MySparseArray *sparse_left = object_left;
	MySparseArray *sparse_right = object_right;
	double cosine = my_sparseArray_multiplyWeightsEqualId(sparse_left,
			sparse_right);
	if (cosine == 0)
		return M_SQRT2;
	else
		return sqrt(2 * fabs(1 - cosine));
}

MknnDistance *profile_get_mdistance(struct SearchProfile *profile) {
	if (my_string_equals_ignorecase(profile->id_dist_custom, "SparseGeneral")) {
		void *state_factory = sparseGeneral_newStateFactory(profile);
		MknnDistance *distance = mknn_distance_newCustomFactory(state_factory,
				sparseGeneral_createDistEval,
				sparseGeneral_releaseStateFactory);
		return distance;
	} else if (my_string_equals_ignorecase(profile->id_dist_custom,
			"SparseCosine")) {
		validateProfileSingleDtype(profile, DTYPE_SPARSE_ARRAY);
		MknnDistance *distance = mknn_distance_newCustom(NULL,
				sparseCosine_eval, NULL);
		return distance;
	} else {
		MknnDistance *distance = mknn_distance_newPredefined(
				mknn_distanceParams_newParseString(profile->id_dist), true);
		return distance;
	}
}
