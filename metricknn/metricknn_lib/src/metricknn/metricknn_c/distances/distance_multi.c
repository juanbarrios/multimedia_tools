/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct State_Multi_Dist {
	int64_t num_distances;
	MknnDistance **distances;
	double *weights_combination;
	double *weights_normalize;
};
struct State_Multi_Eval {
	MknnDistanceEval **subdist_evals;
	struct State_Multi_Dist *state_dist;
};

static double multi_distanceEval_eval(void *state_distEval, void *object_left,
		void *object_right, double current_threshold) {
	struct State_Multi_Eval *state = state_distEval;
	void **array_left = object_left;
	void **array_right = object_right;
	int64_t num_distances = state->state_dist->num_distances;
	double *weights_c = state->state_dist->weights_combination;
	double *weights_n = state->state_dist->weights_normalize;
	double dist = 0;
	for (int64_t i = 0; i < num_distances; ++i) {
		if (weights_c[i] == 0)
			continue;
		double d = mknn_distanceEval_eval(state->subdist_evals[i], array_left[i],
				array_right[i]);
		dist += (d / weights_n[i]) * weights_c[i];
	}
	return dist;
}

static void multi_distanceEval_release(void *state_distEval) {
	struct State_Multi_Eval *state = state_distEval;
	for (int64_t i = 0; i < state->state_dist->num_distances; ++i) {
		mknn_distanceEval_release(state->subdist_evals[i]);
	}
	free(state->subdist_evals);
	free(state);
}
static struct MknnDistEvalInstance multi_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	struct State_Multi_Dist *state_dist = state_distance;
	int64_t length1 = mknn_domain_multiobject_getLength(domain_left);
	int64_t length2 = mknn_domain_multiobject_getLength(domain_right);
	if (length1 != state_dist->num_distances)
		my_log_error(
				"domain_left is multi-object length %"PRIi64" but multi-distance contains %"PRIi64" distance%s\n",
				length1, state_dist->num_distances,
				(state_dist->num_distances == 1) ? "" : "s");
	if (length2 != state_dist->num_distances)
		my_log_error(
				"domain_right is multi-object length %"PRIi64" but multi-distance contains %"PRIi64" distance%s\n",
				length2, state_dist->num_distances,
				(state_dist->num_distances == 1) ? "" : "s");
	struct State_Multi_Eval *state = MY_MALLOC(1, struct State_Multi_Eval);
	state->state_dist = state_dist;
	state->subdist_evals = MY_MALLOC(state_dist->num_distances, MknnDistanceEval*);
	for (int64_t i = 0; i < state_dist->num_distances; ++i) {
		MknnDomain *subLeft = mknn_domain_multiobject_getSubDomain(domain_left,
				i);
		MknnDomain *subRight = mknn_domain_multiobject_getSubDomain(domain_right,
				i);
		state->subdist_evals[i] = mknn_distance_newDistanceEval(
				state_dist->distances[i], subLeft, subRight);
	}
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state;
	di.func_distanceEval_eval = multi_distanceEval_eval;
	di.func_distanceEval_release = multi_distanceEval_release;
	return di;
}
static double getAlphaNormalizationValue(MknnDataset *dataset,
		MknnDistance *distance, double alpha) {
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int64_t num_samples_hist = my_math_round_int(
			log(num_objects) * num_objects / 100.0);
	num_samples_hist = MAX(num_samples_hist, 100);
	int64_t max_threads = my_parallel_getNumberOfCores();
	MknnHistogram *hist_distObjs = mknn_computeDistHistogram(dataset,
			num_samples_hist, distance, max_threads);
	double value = mknn_histogram_getValueQuantile(hist_distObjs, alpha);
	mknn_histogram_release(hist_distObjs);
	return value;
}
static void auto_normalization(struct State_Multi_Dist *state_dist,
		double alpha, MknnDataset *dataset) {
	for (int64_t i = 0; i < state_dist->num_distances; ++i) {
		MknnDataset *subdataset = mknn_dataset_multiobject_getSubDataset(
				dataset, i);
		MknnDistance *dist = state_dist->distances[i];
		double value = getAlphaNormalizationValue(subdataset, dist, alpha);
		state_dist->weights_normalize[i] = (value == 0) ? 1 : value;
	}
}
static void printDetails(struct State_Multi_Dist *state_dist) {
	MyVectorString *arr = my_vectorString_new();
	for (int64_t i = 0; i < state_dist->num_distances; ++i) {
		const char *name = mknn_distance_getIdPredefinedDistance(
				state_dist->distances[i]);
		my_vectorString_add(arr, (char*) name);
	}
	char *st1 = my_vectorString_toString(arr, ';');
	char *st2 = my_newString_arrayDouble(state_dist->weights_normalize,
			state_dist->num_distances, ';');
	char *st3 = my_newString_arrayDouble(state_dist->weights_combination,
			state_dist->num_distances, ';');
	my_log_info("distance MULTI uses distances=%s normalize=%s weights=%s\n",
			st1, st2, st3);
	my_vectorString_release(arr, false);
	free(st1);
	free(st2);
	free(st3);
}
static void multi_dist_build(void *state_distance, const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_Multi_Dist *state_dist = state_distance;
	if (mknn_distanceParams_getString(params_distance,
			"normalization_alpha") != NULL) {
		double alpha = mknn_distanceParams_getDouble(params_distance,
				"normalization_alpha");
		MknnDataset *dataset = mknn_distanceParams_getObject(params_distance,
				"normalization_dataset");
		if (dataset == NULL)
			my_log_error(
					"the dataset must be provided to autonormalize distances\n");
		auto_normalization(state_dist, alpha, dataset);
	}
	printDetails(state_dist);
}
static void multi_dist_release(void *state_distance) {
	struct State_Multi_Dist *state_dist = state_distance;
	for (int64_t i = 0; i < state_dist->num_distances; ++i) {
		mknn_distance_release(state_dist->distances[i]);
	}
	MY_FREE_MULTI(state_dist->weights_combination,
			state_dist->weights_normalize, state_dist->distances);
	free(state_dist);
}
static MyVectorString *parseTerms(const char* line, char token_delimiter) {
	MyTokenizer *tk = my_tokenizer_new(line, token_delimiter);
	my_tokenizer_useBrackets(tk, '(', ')');
	MyVectorString *arr = my_vectorString_new();
	while (my_tokenizer_hasNext(tk)) {
		const char *token = my_tokenizer_nextToken(tk);
		my_vectorString_add(arr, my_newString_string(token));
	}
	my_tokenizer_releaseValidateEnd(tk);
	return arr;
}

static struct MknnDistanceInstance multi_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_Multi_Dist *state_dist = MY_MALLOC(1, struct State_Multi_Dist);
	if (mknn_distanceParams_getString(params_distance, "distances") != NULL) {
		MyVectorString *distnames = parseTerms(
				mknn_distanceParams_getString(params_distance, "distances"),
				';');
		state_dist->num_distances = my_vectorString_size(distnames);
		if (state_dist->num_distances < 1) {
			my_log_info(
					"Parameter 'distances' must contain at least one distance\n");
			mknn_predefDistance_helpPrintDistance(id_dist);
		}
		state_dist->distances = MY_MALLOC(state_dist->num_distances,
				MknnDistance*);
		state_dist->weights_combination = MY_MALLOC(state_dist->num_distances,
				double);
		state_dist->weights_normalize = MY_MALLOC(state_dist->num_distances,
				double);
		for (int64_t i = 0; i < state_dist->num_distances; ++i) {
			char *term = my_vectorString_get(distnames, i);
			state_dist->distances[i] = mknn_distance_newPredefined(
					mknn_distanceParams_newParseString(term), true);
			state_dist->weights_combination[i] = 1;
			state_dist->weights_normalize[i] = 1;
		}
		my_vectorString_release(distnames, true);
	}
	if (state_dist->num_distances < 1)
		mknn_predefDistance_helpPrintDistance(id_dist);
	if (mknn_distanceParams_getString(params_distance, "normalize") != NULL) {
		MyVectorDouble *vector = my_tokenizer_splitLineDouble(
				mknn_distanceParams_getString(params_distance, "normalize"),
				';');
		if (my_vectorDouble_size(vector) != state_dist->num_distances)
			my_log_error(
					"the length of normalize must coincide with the number of distances\n");
		for (int64_t i = 0; i < state_dist->num_distances; ++i) {
			state_dist->weights_normalize[i] *= my_vectorDouble_get(vector, i);
		}
		my_vectorDouble_release(vector);
	}
	if (mknn_distanceParams_getString(params_distance, "weights") != NULL) {
		MyVectorDouble *vector = my_tokenizer_splitLineDouble(
				mknn_distanceParams_getString(params_distance, "weights"), ';');
		if (my_vectorDouble_size(vector) != state_dist->num_distances)
			my_log_error(
					"the number of weights must coincide with the number of distances\n");
		for (int64_t i = 0; i < state_dist->num_distances; ++i) {
			state_dist->weights_combination[i] *= my_vectorDouble_get(vector,
					i);
		}
		my_vectorDouble_release(vector);
	}
	struct MknnDistanceInstance df = { 0 };
	df.state_distance = state_dist;
	df.func_distance_build = multi_dist_build;
	df.func_distance_release = multi_dist_release;
	df.func_distanceEval_new = multi_distanceEval_new;
	return df;
}

static void multi_printHelp(const char *id_dist) {
	my_log_info(
			"  distances=dist1;dist2;...;distN     Required. The distances to combine. The same number of distance as the length of the multi-object.\n");
	my_log_info(
			"  normalize=value1;...;valueN         Optional. The value to normalize each distance. The distances will be scaled by 1/value.\n");
	my_log_info(
			"  weights=weight1;...;weightN         Optional. The weight to apply to each distance.\n");
	my_log_info(
			"  normalization_alpha=[float]         Optional. Computes automatic values for normalization using the distance with cumulative probability alpha. alpha in (0,1], where 1 means maximum distance.\n");
	my_log_info(
			"  normalization_dataset=[dataset]     Optional. The dataset used for computing automatic normalization values.\n");
}
void register_distance_multiDistance() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_MULTIOBJECT, "MULTIDISTANCE",
			"distances=dist1;...,normalization=val1;...,weights=weight1;...,normalization_alpha=[float]",
			multi_printHelp, multi_dist_new);
}
