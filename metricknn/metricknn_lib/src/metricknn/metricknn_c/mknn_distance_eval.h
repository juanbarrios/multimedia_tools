/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DISTANCE_EVAL_H_
#define MKNN_DISTANCE_EVAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * MknnDistanceEval is the function used for comparing objects.
 *
 * In order to compare objects, a #MknnDistance is used to instantiate one or more #MknnDistanceEval objects (one for
 * each parallel thread). The #MknnDistanceEval object computes the distance between two objects of a given domain.
 *
 * The #MknnDistanceEval is created by the method #mknn_distance_newDistanceEval, which requires the actual domain of the
 * objects to be compared.
 *
 * A search method uses the #MknnDistance to create as many #MknnDistanceEval as threads will be used to solve the search.
 * During the search, each thread uses a single #MknnDistanceEval.
 *
 *  @file
 */


/**
 * Evaluates the distance between @p object1 and @p object2.
 *
 * The parameter @p current_threshold can be used to save some computation: if during the computation it is known that the value
 * of the distance will surpass @p current_threshold, the computation is terminated and return some value greater than  @p current_threshold.
 * Note that this can only be fulfilled by distances whose computation produces increasing partial results (as the euclidean distance).
 *
 * @remark <b>Note about Multi-threading</b>: this method can be called in parallel by several threads as long as
 * each thread owns a different @p distance_eval object, i.e. a  @p distance_eval object cannot be used at the same time by different threads.
 *
 * @param distance_eval distance object.
 * @param object_left left object to be evaluated.
 * @param object_right right object to be evaluated.
 * @param current_threshold is the value of the current k-th candidate in a search. This value may be used to trigger an early
 * termination of the distance computation.
 * @return the distance value between @p object_left and @p object_right which must be a number @>= 0.
 */
double mknn_distanceEval_evalTh(MknnDistanceEval *distance_eval, void *object_left,
		void *object_right, double current_threshold);

/**
 * Evaluates the distance between @p object1 and @p object2.
 *
 * @remark <b>Note about Multi-threading</b>: this method can be called in parallel by several threads as long as
 * each thread owns a different @p distance_eval object, i.e. a  @p distance_eval object cannot be used at the same time by different threads.
 *
 * @param distance_eval distance object.
 * @param object_left left object to be evaluated.
 * @param object_right right object to be evaluated.
 * @return the distance value between @p object_left and @p object_right which must be a number @>= 0.
 */
double mknn_distanceEval_eval(MknnDistanceEval *distance_eval, void *object_left,
		void *object_right);

/**
 * returns the distance declared for the object.
 * @param distance_eval
 * @return the distance declared for the object.
 */
MknnDistance *mknn_distanceEval_getDistance(MknnDistanceEval *distance_eval);

/**
 * returns the domain declared for the object at the left.
 * @param distance_eval
 * @return the domain declared for the object at the left.
 */
MknnDomain *mknn_distanceEval_getDomainLeft(MknnDistanceEval *distance_eval);

/**
 * returns the domain declared for the object at the right.
 * @param distance_eval
 * @return the domain declared for the object at the right.
 */
MknnDomain *mknn_distanceEval_getDomainRight(MknnDistanceEval *distance_eval);

/**
 * Releases the distance eval.
 * @param distance_eval
 */
void mknn_distanceEval_release(MknnDistanceEval *distance_eval);


#ifdef __cplusplus
}
#endif

#endif
