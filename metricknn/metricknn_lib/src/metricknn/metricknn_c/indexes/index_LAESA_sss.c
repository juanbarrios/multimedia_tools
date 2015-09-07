/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct P_PivotSet {
	MyVectorInt *pivots_pos;
	MyVectorObj *pivots_obj;
	double dist_threshold;
	struct MyDataStats stats_lbs;
	struct MyDataStats stats_diffs;
	int64_t* contLBPivot;
};
struct EvalPair {
	void *object1, *object2;
	double distance;
};
struct EvalSubset {
	int64_t num_samples_eval;
	struct EvalPair *samples;
};

struct P_EvaluarParamThread {
	MknnDataset *dataset;
	MknnDistanceEval **dist_evals;
	MknnHistogram *hist_distObjs;
	struct P_PivotSet **sets;
	int64_t num_pivots;
	struct P_Conjunto *conj;
	double *evalActualDist;
	double alpha;
	int64_t triesCurrentAlpha;
	struct EvalSubset *evset;
	double ALPHA_START;
	double ALPHA_REDUCTION_FACTOR;
	double ALPHA_TRIES_BEFORE_REDUCTION;
	pthread_mutex_t mutex;
};
static bool is_eligible_pivot(void *object, MyVectorObj *pivots_obj,
		MknnDataset *dataset, MknnDistanceEval *distance, double dist_threshold) {
	for (int64_t i = 0; i < my_vectorObj_size(pivots_obj); ++i) {
		void *piv = my_vectorObj_get(pivots_obj, i);
		double d = mknn_distanceEval_eval(distance, object, piv);
		if (d <= dist_threshold)
			return false;
	}
	return true;
}
static void selectPivotsUsingSSS(struct P_PivotSet *set, MknnDataset *dataset,
		int64_t num_pivots, MknnDistanceEval *distance) {
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int64_t *ids = my_random_newPermutation(0, num_objects);
	for (int64_t i = 0; i < num_objects; ++i) {
		int64_t pos = ids[i];
		void *object = mknn_dataset_getObject(dataset, pos);
		if (is_eligible_pivot(object, set->pivots_obj, dataset, distance,
				set->dist_threshold)) {
			my_vectorObj_add(set->pivots_obj, object);
			my_vectorInt_add(set->pivots_pos, pos);
			if (my_vectorObj_size(set->pivots_obj) >= num_pivots)
				break;
		}
	}
	free(ids);
}

static double getCurrentAlpha(struct P_EvaluarParamThread *param,
		double alphaLastTry, double *out_threshold) {
	MY_MUTEX_LOCK(param->mutex);
	if (param->alpha == 0)
		param->alpha = param->ALPHA_START;
	if (alphaLastTry == param->alpha) {
		param->triesCurrentAlpha++;
		if (param->triesCurrentAlpha >= param->ALPHA_TRIES_BEFORE_REDUCTION) {
			param->alpha *= param->ALPHA_REDUCTION_FACTOR;
			param->triesCurrentAlpha = 0;
			char *st1 = my_newString_double(param->alpha);
			char *st2 = my_newString_double(
					mknn_histogram_getValueQuantile(param->hist_distObjs,
							param->alpha));
			my_log_info("decreasing sparse threshold to alpha=%s distance=%s\n",
					st1, st2);
			MY_FREE_MULTI(st1, st2);
		}
	}
	double alpha = param->alpha;
	*out_threshold = mknn_histogram_getValueQuantile(param->hist_distObjs,
			alpha);
	MY_MUTEX_UNLOCK(param->mutex);
	return alpha;
}

static struct P_PivotSet *generate_new_pivotset(
		struct P_EvaluarParamThread *param, MknnDistanceEval *distance) {
	double alpha = 0;
	for (;;) {
		double threshold = 0;
		alpha = getCurrentAlpha(param, alpha, &threshold);
		struct P_PivotSet *set = MY_MALLOC(1, struct P_PivotSet);
		set->pivots_pos = my_vectorInt_new();
		set->pivots_obj = my_vectorObj_new();
		set->dist_threshold = threshold;
		selectPivotsUsingSSS(set, param->dataset, param->num_pivots, distance);
		if (my_vectorObj_size(set->pivots_obj) == param->num_pivots)
			return set;
		my_vectorInt_release(set->pivots_pos);
		my_vectorObj_release(set->pivots_obj, 0);
		free(set);
	}
	my_log_error("error in sss\n");
	return NULL;
}
static double computeMaxLB(void *obj1, void *obj2, struct P_PivotSet *set,
		MknnDistanceEval *distance, int64_t *out_id_piv_lb) {
	int64_t i, id_piv_lb = -1;
	double max_lb = -1;
	for (i = 0; i < my_vectorObj_size(set->pivots_obj); ++i) {
		void *piv = my_vectorObj_get(set->pivots_obj, i);
		double dist_qp = mknn_distanceEval_eval(distance, obj1, piv);
		double dist_rp = mknn_distanceEval_eval(distance, obj2, piv);
		double lowerBoundDistance = fabs(dist_qp - dist_rp);
		if (lowerBoundDistance > max_lb) {
			max_lb = lowerBoundDistance;
			id_piv_lb = i;
		}
	}
	*out_id_piv_lb = id_piv_lb;
	return max_lb;
}
static void evaluatePivotSet(struct P_PivotSet *set, struct EvalSubset *evset,
		MknnDistanceEval *distance) {
	int64_t num_pivots = my_vectorObj_size(set->pivots_obj);
	double *lbs = MY_MALLOC(evset->num_samples_eval, double);
	double *diffs = MY_MALLOC(evset->num_samples_eval, double);
	set->contLBPivot = MY_MALLOC(num_pivots, int64_t);
	int64_t i;
	for (i = 0; i < evset->num_samples_eval; ++i) {
		void *obj1 = evset->samples[i].object1;
		void *obj2 = evset->samples[i].object2;
		double actual_dist = evset->samples[i].distance;
		int64_t id_piv_lb = -1;
		double max_lb = computeMaxLB(obj1, obj2, set, distance, &id_piv_lb);
		set->contLBPivot[id_piv_lb]++;
		lbs[i] = max_lb;
		diffs[i] = actual_dist - max_lb;
	}
	set->stats_lbs = my_math_computeStats(evset->num_samples_eval, lbs);
	set->stats_diffs = my_math_computeStats(evset->num_samples_eval, diffs);
	free(lbs);
	free(diffs);
}
static void process_pivotsets_thread(int64_t current_process,
		void *state_object, int64_t current_thread) {
	struct P_EvaluarParamThread *param = state_object;
	MknnDistanceEval *distance_eval = param->dist_evals[current_thread];
	struct P_PivotSet *set = generate_new_pivotset(param, distance_eval);
	if (param->evset != NULL)
		evaluatePivotSet(set, param->evset, distance_eval);
	param->sets[current_process] = set;
}
static MknnHistogram *getHistogram(MknnDataset *dataset, MknnDistance *distance,
		int64_t max_threads) {
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int64_t num_samples_hist = my_math_round_int(
			log(num_objects) * num_objects / 100.0);
	num_samples_hist = MAX(num_samples_hist, 100);
	MknnHistogram *hist_distObjs = mknn_computeDistHistogram(dataset,
			num_samples_hist, distance, max_threads);
	return hist_distObjs;
}
static struct EvalSubset *getEvalSubset(MknnDataset *dataset,
		MknnDistance *distance, int64_t max_threads) {
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int64_t num_samples_eval = my_math_round_int(log(num_objects) * 10000);
	num_samples_eval = MAX(num_samples_eval, 100);
	my_log_info("computing evaluation subset (%"PRIi64" samples)\n",
			num_samples_eval);
	int64_t *pos_src = MY_MALLOC(num_samples_eval, int64_t);
	int64_t *pos_dst = MY_MALLOC(num_samples_eval, int64_t);
	double *distances = MY_MALLOC(num_samples_eval, double);
	mknn_sample_distances_multithread(dataset, dataset, num_samples_eval,
			distance, max_threads, distances, pos_src, pos_dst);
	struct EvalSubset *evset = MY_MALLOC(1, struct EvalSubset);
	evset->num_samples_eval = num_samples_eval;
	evset->samples = MY_MALLOC(num_samples_eval, struct EvalPair);
	for (int64_t i = 0; i < num_samples_eval; ++i) {
		evset->samples[i].object1 = mknn_dataset_getObject(dataset, pos_src[i]);
		evset->samples[i].object2 = mknn_dataset_getObject(dataset, pos_dst[i]);
		evset->samples[i].distance = distances[i];
	}
	return evset;
}
static void releaseEvalSubset(struct EvalSubset *evset) {
	MY_FREE(evset->samples);
	MY_FREE(evset);
}
static struct P_PivotSet** compute_sets(MknnDataset *dataset,
		MknnDistance *distance, int64_t num_pivots, int64_t num_sets_eval,
		int64_t max_threads, MknnHistogram *hist_distObjs,
		struct EvalSubset *evset) {
	struct P_EvaluarParamThread param = { 0 };
	param.dataset = dataset;
	param.dist_evals = mknn_distance_createDistEvalArray(distance, max_threads,
			mknn_dataset_getDomain(dataset), mknn_dataset_getDomain(dataset));
	param.num_pivots = num_pivots;
	param.sets = MY_MALLOC(num_sets_eval, struct P_PivotSet*);
	param.hist_distObjs = hist_distObjs;
	param.evset = evset;
	MY_MUTEX_INIT(param.mutex);
	if (num_pivots < 10) {
		param.ALPHA_START = 0.999;
		param.ALPHA_REDUCTION_FACTOR = 0.99;
		param.ALPHA_TRIES_BEFORE_REDUCTION = 2;
	} else if (num_pivots < 100) {
		param.ALPHA_START = 0.8;
		param.ALPHA_REDUCTION_FACTOR = 0.8;
		param.ALPHA_TRIES_BEFORE_REDUCTION = 2;
	} else if (num_pivots < 200) {
		param.ALPHA_START = 0.5;
		param.ALPHA_REDUCTION_FACTOR = 0.5;
		param.ALPHA_TRIES_BEFORE_REDUCTION = 1;
	} else {
		param.ALPHA_START = 0.2;
		param.ALPHA_REDUCTION_FACTOR = 0.5;
		param.ALPHA_TRIES_BEFORE_REDUCTION = 1;
	}
	if (num_sets_eval > 1)
		my_log_info_time("selecting %"PRIi64" sets\n", num_sets_eval);
	my_parallel_incremental(num_sets_eval, &param, process_pivotsets_thread,
	NULL, max_threads);
	MY_MUTEX_DESTROY(param.mutex);
	mknn_distanceEval_releaseArray(param.dist_evals, max_threads);
	return param.sets;
}
static struct P_PivotSet *select_best_set(int64_t num_sets_eval,
		struct P_PivotSet **pivot_sets) {
	if (num_sets_eval == 1)
		return pivot_sets[0];
	my_log_info("evaluation %"PRIi64" sets:\n", num_sets_eval);
	int64_t selected = -1;
	struct P_PivotSet *selSet = NULL;
	for (int64_t i = 0; i < num_sets_eval; ++i) {
		struct P_PivotSet *set = pivot_sets[i];
		char *st1 = my_newString_doubleDec(set->stats_lbs.average, 2);
		char *st2 = my_newString_doubleDec(set->stats_diffs.average, 2);
		char *st3 = my_vectorInt_toString(set->pivots_pos, ',');
		my_log_info(
				"set %2"PRIi64") pivs=%-3"PRIi64" avgLB=%s  avgDiff=%s  pivotsId=%s\n",
				i, my_vectorObj_size(set->pivots_obj), st1, st2, st3);
		MY_FREE_MULTI(st1, st2, st3);
		if (i == 0 || set->stats_lbs.average > selSet->stats_lbs.average) {
			selected = i;
			selSet = set;
		}
	}
	char *stAvg = my_newString_doubleDec(selSet->stats_lbs.average, 2);
	my_log_info("selected set %"PRIi64" with avgLB=%s\n", selected, stAvg);
	MY_FREE(stAvg);
	return selSet;
}
static void releasePivotSet(struct P_PivotSet *set) {
	my_vectorObj_release(set->pivots_obj, 0);
	my_vectorInt_release(set->pivots_pos);
	MY_FREE(set->contLBPivot);
	MY_FREE(set);
}
void mknn_laesa_select_pivots_sss(MknnDataset *dataset, MknnDistance *distance,
		int64_t num_pivots, int64_t num_sets_eval, int64_t max_threads,
		int64_t *selected_positions) {
	if (num_pivots <= 0)
		num_pivots = 1;
	if (num_sets_eval <= 0)
		num_sets_eval = 1;
	if (max_threads <= 0)
		max_threads = 1;
	my_log_info(
			"selecting %"PRIi64" sets of %"PRIi64" pivots using %"PRIi64" threads\n",
			num_sets_eval, num_pivots, max_threads);
	MknnHistogram *hist_distObjs = getHistogram(dataset, distance, max_threads);
	struct EvalSubset *evset = NULL;
	if (num_sets_eval > 1)
		evset = getEvalSubset(dataset, distance, max_threads);
	struct P_PivotSet **sets = compute_sets(dataset, distance, num_pivots,
			num_sets_eval, max_threads, hist_distObjs, evset);
	mknn_histogram_release(hist_distObjs);
	if (evset != NULL)
		releaseEvalSubset(evset);
	struct P_PivotSet *set = select_best_set(num_sets_eval, sets);
	my_assert_equalInt("num_pivots", my_vectorInt_size(set->pivots_pos),
			num_pivots);
	for (int64_t i = 0; i < num_pivots; ++i) {
		int64_t pos = my_vectorInt_get(set->pivots_pos, i);
		selected_positions[i] = pos;
	}
	for (int64_t i = 0; i < num_sets_eval; ++i)
		releasePivotSet(sets[i]);
	MY_FREE(sets);
}
