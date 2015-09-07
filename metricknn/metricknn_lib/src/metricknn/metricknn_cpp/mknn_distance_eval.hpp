/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DISTANCE_EVAL_HPP_
#define MKNN_DISTANCE_EVAL_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 * This class computes the distance between two objects from a given Domain
 */
class DistanceEval {
public:
	/**
	 * Evaluates the distance between @p object1 and @p object2.
	 *
	 * @remark <b>Note about Multi-threading</b>: Each instance of DistanceEval will be called sequentially by a single thread.
	 *
	 * @param object_left left object to be evaluated.
	 * @param object_right right object to be evaluated.
	 * @return a number @>= 0 with the distance value between @p object1 and @p object2.
	 */
	double eval(void *object_left, void *object_right);

	/**
	 * Evaluates the distance between @p object1 and @p object2.
	 *
	 * The parameter @p current_threshold can be used to save some computation: if during the computation it is known that the value
	 * of the distance will surpass @p current_threshold, the computation is terminated and return some value greater than  @p current_threshold.
	 * Note that this can only be fulfilled by distances whose computation produces increasing partial results (as the euclidean distance).
	 *
	 * @remark <b>Note about Multi-threading</b>: Each instance of DistanceEval will be called sequentially by a single thread.
	 *
	 * @param object_left left object to be evaluated.
	 * @param object_right right object to be evaluated.
	 * @param current_threshold current threshold to perform an early termination.
	 * @return a number @>= 0 with the distance value between @p object1 and @p object2.
	 */
	double evalTh(void *object_left, void *object_right,
			double current_threshold);

	/**
	 * returns the distance declared for the object.
	 * @return the distance declared for the object.
	 */
	Distance &getDistance();

	/**
	 * returns the domain declared for the object at the left.
	 * @return the domain declared for the object at the left.
	 */
	Domain &getDomainLeft();

	/**
	 * returns the domain declared for the object at the right.
	 * @return the domain declared for the object at the right.
	 */
	Domain &getDomainRight();

	/**
	 * @}
	 */

	/**
	 * Default constructor.
	 */
	DistanceEval();
	/**
	 * Default destructor.
	 */
	virtual ~DistanceEval();
	/**
	 * Copy constructor.
	 */
	DistanceEval(const DistanceEval &other);
	/**
	 * Assignment operator.
	 */
	DistanceEval &operator=(const DistanceEval &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Distance;
};

}

#endif
