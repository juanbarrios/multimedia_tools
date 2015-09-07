/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DISTANCE_HPP_
#define MKNN_DISTANCE_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

class DistanceEval;
class DistanceCustomFactory;

/**
 * The Distance is the object with the definition of the method for comparing objects.
 *
 * A distance object can be instantiated in three different ways:
 *
 * 1) Using one of the constructors for predefined distances in PredefinedDistance class.
 *
 * 2) Using function pointers and define a custom distance, see Distance::newCustom.
 *
 * 3) Using the generic method #newPredefined, which requires a DistanceParams object.
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
 * calling PredefDistance::helpListDistances. The parameters supported by each distance can be printed by calling PredefDistance::helpPrintDistance.
 *
 * Depending on the distance, the constructor method may take some time to complete. In order
 * to reduce the build time, a distance can be saved and then loaded.
 *
 * Each predefined distance requires the objects to compare be in a given general domain, i.e., vectors, strings or multi object.
 *
 * In order to compute distance values, the Distance object must be used to instantiate one or more DistanceEval objects (one for
 * each parallel thread). The DistanceEval object is used to compute the distance value between two objects.
 *
 * The DistanceEval is created by the method mknn::Distance::newDistanceEval, which requires the actual domain of the
 * objects to be compared.
 *
 * A search method uses the Distance to create as many DistanceEval as threads will be used to solve the search.
 * During the search, each thread uses a single DistanceEval.
 *
 */
class Distance {
public:
	/**
	 * Creates a new distance for the given parameters.
	 *
	 * The list of pre-defined distances can be listed by invoking PredefDistance::helpListDistances.
	 *
	 * @param parameters the parameters to create the distance.
	 * @param delete_parameters_on_distance_release binds the lifetime of @p parameters to the new distance.
	 * @return a new distance (must be released)
	 */
	static Distance newPredefined(DistanceParams &parameters);

	/**
	 * Creates a new custom function.
	 *
	 * When resolving a search using @c N parallel threads, the factory
	 * is invoked @c N times (thus creating a state for each thread), then each thread
	 * invokes mknn::DistanceCustomInstance::evalDistance.
	 *
	 * @param factory the object that will be used to obtain instances of mknn::DistanceCustomInstance
	 * @param delete_factory_on_distance_release binds the lifetime of @c factory to the new distance object
	 * @return a new distance object
	 */
	static Distance newCustom(DistanceCustomFactory *factory,
			bool delete_factory_on_distance_release);

	/**
	 * Return the parameters used to create the distance.
	 * @return the parameters used to create the distance.
	 */
	DistanceParams &getParameters();

	/**
	 * The id of the predefined distance.
	 * @return a string with an id or NULL if it is a custom distance.
	 */
	std::string getIdPredefinedDistance();

	/**
	 * Creates a distance for computing distances between objects of the given domains.
	 *
	 * <tt>domain_object1</tt> and <tt>domain_object2</tt> are the domains of the objects that will be
	 * used in DistanceEval::eval. These two domains must be compatible with the domain
	 * of <tt>distance</tt>, i.e., they may differ only in the datatype (see Domain::testEqualExceptDatatype).
	 *
	 * @param domain_left the domain of the left object
	 * @param domain_right the domain of the right object
	 * @return a new distance (it must be released with delete).
	 */
	DistanceEval newDistanceEval(Domain domain_left, Domain domain_right);

	/**
	 * @name Load/Save distances
	 * @{
	 */
	/**
	 * The distance is saved to a file.
	 * It may create other files using @p filename_write as prefix.
	 * The created files may be in binary format for fast loading.
	 *
	 * @remark A custom distance cannot be saved.
	 *
	 * @param filename_write File to create. If the file already exists it is overwritten.
	 */

	void save(std::string filename_write);

	/**
	 * Loads a distance from a file.
	 * The file must have been created by save.
	 *
	 * @param filename_read File to read. If the file does not exists an error is raised.
	 * @param more_parameters any parameter that may override the stored parameters or new parameters that
	 * could not be saved.
	 * @param delete_parameters_on_distance_release binds the lifetime of @p more_parameters to the new distance.
	 * @return a new distance (it must be released with delete).
	 */
	static Distance restore(std::string filename_read,
			DistanceParams &more_parameters);

	/**
	 * @}
	 */

	/**
	 * Default constructor.
	 */
	Distance();
	/**
	 * Default destructor.
	 */
	virtual ~Distance();
	/**
	 * Copy constructor.
	 */
	Distance(const Distance &other);
	/**
	 * Assignment operator.
	 */
	Distance &operator=(const Distance &other);

protected:
	/**
	 * Internal opaque class
	 */
	class Impl;
	/**
	 * Internal opaque class
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Domain;
	friend class Index;
};

/**
 * Abstract class that must be inherited to define a custom distance.
 */
class DistanceCustomInstance {
public:
	/**
	 * The function of a custom distance that computes the distance between two objects.
	 * The parameters @p state_dist is the value returned by @p mknn_function_distanceEval_start.
	 * The parameters @p domain_left and @p domain_right are the same objects
	 * given to DistanceCustomFactory::newInstance
	 *
	 * @param object_left is the left object of the distance.
	 * @param object_right is the right object of the distance.
	 * @param current_threshold is the value of the current k-th candidate in a search. This value may be used to trigger an early
	 * termination of the computation. See DistanceEval::eval.
	 * @return the distance value between @p object_left and @p object_right which must be a number @>= 0.
	 */
	virtual double evalDistance(void *object_left, void *object_right,
			double current_threshold) = 0;

	virtual ~DistanceCustomInstance() = 0;
};

/**
 * Abstract class that must be inherited to define a custom distance.
 */
class DistanceCustomFactory {
public:
	/**
	 * The function of a custom distance that creates a new state.
	 *
	 * This method is internally invoked by Distance::newDistanceEval to create one state
	 * for each thread resolving the search.
	 *
	 * @param domain_left domain of the left object of the distance.
	 * @param domain_right domain of the right object of the distance.
	 * @return a new instance of the distance for the given domains
	 */
	virtual DistanceCustomInstance *newInstance(Domain *domain_left,
			Domain *domain_right) = 0;

	virtual ~DistanceCustomFactory() = 0;

};

}

#endif
