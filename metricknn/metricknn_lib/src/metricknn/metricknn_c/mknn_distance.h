/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DISTANCE_H_
#define MKNN_DISTANCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * MknnDistance represents a method for comparing objects.
 *
 * A MknnDistance object can be instantiated in three different ways:
 *
 * 1) Using one of the constructors for predefined distances. For example, #mknn_predefDistance_L2.
 *
 * 2) Using function pointers and define a custom distance, see #mknn_distance_newCustom.
 *
 * 3) Using the generic method #mknn_distance_newPredefined, which requires a #MknnDistanceParams object.
 * The parameters can be created by parsing a string of parameters (e.g. a command line parameter), by setting each required parameter,
 * or by loading a configuration file. Each pre-defined distance is identified by a unique distance identifier. Some valid IDs are:
 *
 *  @li @c L1 The Manhattan or taxi-cab distance.
 *  @li @c L2 The Euclidean distance.
 *  @li @c LMAX The maximum distance.
 *  @li @c LP Any Minkowsky distance.
 *  @li @c EMD The Earth Mover's distance.
 *  @li @c DPF The Dynamic Partial Function.
 *  @li @c CHI2 The Chi-squared distance.
 *  @li @c HAMMING The hamming distance.
 *
 * The complete list of pre-defined distances provided by MetricKnn can be printed by
 * calling #mknn_predefDistance_helpListDistances. The parameters supported by each distance can be printed by calling #mknn_predefDistance_helpPrintDistance.
 *
 * Depending on the distance, the constructor method may take some time to complete. In order
 * to reduce the build time, a distance can be saved and then loaded.
 *
 * Each predefined distance requires the objects to compare be in a given general domain, i.e., vectors, strings or multi object.
 *
 * In order to compare objects, the #MknnDistance must be used to instantiate one or more #MknnDistanceEval objects.
 *
 *  @file
 */
/**
 * The function of a custom distance that creates a new state.
 *
 * This method is internally invoked by #mknn_distance_newDistanceEval to create one state
 * for each thread resolving the search.
 *
 * @param domain_left domain of the left object of the distance.
 * @param domain_right domain of the right object of the distance.
 * @return a new state that will be given to #mknn_function_distanceEval_eval
 */
typedef void* (*mknn_function_distanceEval_createState)(MknnDomain *domain_left,
		MknnDomain *domain_right);

/**
 * Computes the distance between two objects.
 * The parameters @p state_dist is the value returned by @p mknn_function_distanceEval_createState.
 * The parameters @p domain_left and @p domain_right are the same objects
 * given to #mknn_function_distanceEval_createState.
 *
 * This method is internally invoked by #mknn_distanceEval_eval.
 *
 * @param state_distEval an object that was previously created by #mknn_function_distanceEval_createState.
 * @param object_left is the left object of the distance.
 * @param object_right is the right object of the distance.
 * @param current_threshold is the value of the current k-th candidate in a search. This value may be used to trigger an early
 * termination of the computation. See #mknn_distanceEval_eval.
 * @return the distance value between @p object_left and @p object_right which must be a number @>= 0.
 */
typedef double (*mknn_function_distanceEval_eval)(void *state_distEval,
		void *object_left, void *object_right, double current_threshold);

/**
 * The function of a custom distance that releases a state.
 * The state is an object that was returned by #mknn_function_distanceEval_createState.
 *
 * This method is internally invoked by #mknn_distanceEval_release for each state
 * previously created by #mknn_function_distanceEval_createState.
 *
 * @param state_eval the value to be released.
 */
typedef void (*mknn_function_distanceEval_releaseState)(void *state_distEval);

/**
 * The function of a custom distance factory.
 *
 * @param state_factory the state of the factory
 * @param domain_left domain of the left object of the distance.
 * @param domain_right domain of the right object of the distance.
 * @param out_state_distEval the state of the created distance
 * @param out_func_eval the function to eval the distance
 * @param out_func_releaseState the function to release @c out_state_distEval.
 */
typedef void (*mknn_function_distanceFactory_createDistEval)(
		void *state_factory, MknnDomain *domain_left, MknnDomain *domain_right,
		void **out_state_distEval,
		mknn_function_distanceEval_eval *out_func_eval,
		mknn_function_distanceEval_releaseState *out_func_releaseState);
/**
 * The function to release the state of the distance factory. Called by #mknn_distance_release.
 *
 * @param state_factory the state of the factory to release
 */
typedef void (*mknn_function_distanceFactory_releaseFactory)(
		void *state_factory);

/**
 * Creates a new distance for the given parameters.
 *
 * The list of pre-defined distances can be listed by invoking #mknn_predefDistance_helpListDistances.
 *
 * @param parameters the parameters to create the distance.
 * @param free_parameters_on_distance_release bind the lifetime of @p parameters to the new distance.
 * @return a new distance (must be released)
 */
MknnDistance *mknn_distance_newPredefined(MknnDistanceParams *parameters,
bool free_parameters_on_distance_release);

/**
 * Creates a new custom distance.
 *
 * When resolving a search using @c N parallel threads, the function @p func_createState
 * is invoked @c N times (creating a state for each thread), then each thread invokes @p func_eval passing its
 * corresponding state, and at the end @c func_releaseState is invoked @c N times to release the @c N states.
 *
 * @remark @c func_createState, @c func_eval and @c func_releaseState may be called in parallel by different threads. Therefore if they
 * write to some global variable, some synchronization method is required (e.g. @c pthread_mutex_lock and @c pthread_mutex_unlock).
 * Despite @c func_eval may be called in parallel, it does not need any synchronization
 * if it only writes to the @c state_dist object (which is created by @c func_createState).
 *
 * If @c func_createState needs some common state (e.g. to save time at invoking for all threads) or if
 * the definition of @c func_eval depends on the domains, the #mknn_distance_newCustomFactory can use a
 * common state and can vary the @c func_eval.
 *
 * @param func_createState The function that creates a new state for computing the distance or NULL if it is not required.
 * @param func_eval The function that computes the distance between two objects.
 * @param func_releaseState The function that releases the state created by @c func_createState.
 * @return the new distance
 */
MknnDistance *mknn_distance_newCustom(
		mknn_function_distanceEval_createState func_createState,
		mknn_function_distanceEval_eval func_eval,
		mknn_function_distanceEval_releaseState func_releaseState);

/**
 * Creates a new custom distance, using a factory method.
 * @param state_factory
 * @param func_factory
 * @param func_release
 * @return the new distance
 */
MknnDistance *mknn_distance_newCustomFactory(void *state_factory,
		mknn_function_distanceFactory_createDistEval func_factory,
		mknn_function_distanceFactory_releaseFactory func_release);

/**
 * Return the parameters used to create the predefined distance.
 * @param distance a predefined distance.
 * @return the parameters used to create the distance.
 */
MknnDistanceParams *mknn_distance_getParameters(MknnDistance *distance);

/**
 * The id of the predefined distance.
 * @param distance a predefined distance.
 * @return a string with an id or NULL if it is a custom distance.
 */
const char *mknn_distance_getIdPredefinedDistance(MknnDistance *distance);

/**
 * The distance is saved to a file.
 * It may create other files using @p filename_write as prefix.
 * The created files may be in binary format for fast loading.
 *
 * @remark A custom distance cannot be saved.
 *
 * @param distance the distance to save
 * @param filename_write File to create. If the file already exists it is overwritten.
 */

void mknn_distance_save(MknnDistance *distance, const char *filename_write);

/**
 * Loads a distance from a file.
 * The file must have been created by #mknn_distance_save.
 *
 * @param filename_read File to read. If the file does not exists an error is raised.
 * @param more_parameters any parameter that may override the stored parameters or new parameters that
 * could not be saved.
 * @param free_parameters_on_distance_release to release @p more_parameters during the release of the new distance.
 * @return a new distance (it must be released with #mknn_distance_release).
 */
MknnDistance *mknn_distance_restore(const char *filename_read,
		MknnDistanceParams *more_parameters, bool free_parameters_on_distance_release);

/**
 * Creates a distance for computing distances between objects of the given domains.
 *
 * <tt>domain_object1</tt> and <tt>domain_object2</tt> are the domains of the objects that will be
 * used in #mknn_distanceEval_eval. These two domains must be compatible with the domain
 * of <tt>distance</tt>, i.e., they may differ only in the datatype (see #mknn_domain_testEqualExceptDatatype).
 *
 * @param distance the distance
 * @param domain_left the domain of the left object
 * @param domain_right the domain of the right object
 * @return a new distance (it must be released with #mknn_distanceEval_release).
 */
MknnDistanceEval *mknn_distance_newDistanceEval(MknnDistance *distance,
		MknnDomain *domain_left, MknnDomain *domain_right);

/**
 * Releases the distance.
 *
 * @param distance the distance to be released.
 */
void mknn_distance_release(MknnDistance *distance);

#ifdef __cplusplus
}
#endif

#endif
