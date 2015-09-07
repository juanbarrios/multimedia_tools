/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_RESOLVER_H_
#define MKNN_RESOLVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * A MknnResolver represents the parameters of a similarity search.
 * The resolver is created by the method #mknn_index_newResolver which needs the search parameters and the index.
 *
 * A similarity search is performed by calling method #mknn_resolver_search.
 * The results of a search is stored in the object #MknnResult.
 *
 *  @file
 */

/**
 * Performs the configured similarity search.
 *
 * @note This method make take long time.
 *
 * @param resolver the resolver containing the search parameters
 * @param free_resolver_on_result_release binds the lifetime of @p resolver to the new result.
 * @param query_dataset the set of query objects to resolve
 * @param free_query_dataset_on_result_release binds the lifetime of @p query_dataset to the new result.
 * @return a new search result (it must be released with #mknn_result_release).
 */
MknnResult *mknn_resolver_search(MknnResolver *resolver,
		bool free_resolver_on_result_release, MknnDataset *query_dataset,
		bool free_query_dataset_on_result_release);

/**
 * Return the parameters used to create the resolver.
 * @param resolver the resolver
 * @return the parameters used to create the resolver.
 */
MknnResolverParams *mknn_resolver_getParameters(MknnResolver *resolver);

/**
 * Returns the index used to create the resolver.
 * @param resolver the resolver
 * @return the index used to create the resolver
 */
MknnIndex *mknn_resolver_getIndex(MknnResolver *resolver);

/**
 * Releases the resolver.
 *
 * @param resolver a resolver
 */
void mknn_resolver_release(MknnResolver *resolver);

#ifdef __cplusplus
}
#endif

#endif
