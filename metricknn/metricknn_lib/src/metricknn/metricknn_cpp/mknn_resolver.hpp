/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_RESOLVER_HPP_
#define MKNN_RESOLVER_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

class Result;

/**
 * A Resolver represents the parameters of a similarity search.
 * The constructor needs the search parameters and the index.
 *
 * A similarity search is performed by calling method Resolver::search.
 * The results of a search is stored in the object Result.
 *
 */

class Resolver {
public:

	/**
	 * Return the parameters used to create the resolver.
	 * @return the parameters used to create the resolver.
	 */
	ResolverParams &getParameters();

	/**
	 * Returns the index used to create the resolver.
	 * @return the index used to create the resolver
	 */
	Index &getIndex();

	/**
	 * Performs the configured similarity search.
	 *
	 * @note This method make take long time.
	 *
	 * @param delete_resolver_on_result_release binds the lifetime of current @p resolver to the new result.
	 * @param query_dataset the set of query objects to resolve
	 * @param delete_query_dataset_on_result_release binds the lifetime of @p query_dataset to the new result.
	 * @return a new search result (it must be released with delete).
	 */
	Result search(Dataset &query_dataset);

	/**
	 * Default constructor.
	 */
	Resolver();
	/**
	 * Default destructor.
	 */
	virtual ~Resolver();
	/**
	 * Copy constructor.
	 */
	Resolver(const Resolver &other);
	/**
	 * Assignment operator.
	 */
	Resolver &operator=(const Resolver &other);

protected:
	/**
	 * Internal opaque class
	 */
	class Impl;
	/**
	 * Internal opaque class
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Index;
};

}
#endif
