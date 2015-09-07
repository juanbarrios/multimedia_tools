/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnResolver {
	MknnIndex *index;
	MknnResolverParams *parameters;
	bool free_parameters_on_release;
	struct MknnResolverInstance resolverInstance;
};

MknnResolver *mknn_resolver_newInternal(
		struct MknnResolverInstance resolverInstance, MknnIndex *index,
		MknnResolverParams *parameters,
		bool free_parameters_on_resolver_release) {
	MknnResolver *resolver = MY_MALLOC(1, MknnResolver);
	resolver->index = index;
	resolver->parameters = parameters;
	resolver->free_parameters_on_release = free_parameters_on_resolver_release;
	resolver->resolverInstance = resolverInstance;
	return resolver;
}
MknnResolverParams *mknn_resolver_getParameters(MknnResolver *resolver) {
	if (resolver == NULL)
		return NULL;
	return resolver->parameters;
}
MknnIndex *mknn_resolver_getIndex(MknnResolver *resolver) {
	if (resolver == NULL)
		return NULL;
	return resolver->index;
}
MknnResult *mknn_resolver_search(MknnResolver *resolver,
bool free_resolver_on_release, MknnDataset *query_dataset,
bool free_query_dataset_on_release) {
	if (resolver == NULL || query_dataset == NULL)
		return NULL;
	MyTimer *search_timer = my_timer_new();
	MknnResult *result = resolver->resolverInstance.func_resolver_search(
			resolver->resolverInstance.state_resolver, query_dataset);
	mknn_result_setResolverQueryDataset(result, resolver, query_dataset,
			free_resolver_on_release, free_query_dataset_on_release);
	mknn_result_updateTotalDistanceEvaluations(result);
	mknn_result_setTotalSearchTime(result, my_timer_getSeconds(search_timer));
	my_timer_release(search_timer);
	return result;
}
void mknn_resolver_release(MknnResolver *resolver) {
	if (resolver->free_parameters_on_release)
		mknn_resolverParams_release(resolver->parameters);
	free(resolver);
}

