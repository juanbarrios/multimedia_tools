/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

MknnDataset *mknn_datasetLoader_reorderRandomPermutation(
		MknnDataset *superdataset,
		bool free_superdataset_on_release) {
	int64_t size = mknn_dataset_getNumObjects(superdataset);
	int64_t *permutation = my_random_newPermutation(0, size);
	MknnDataset *subset = mknn_datasetLoader_SubsetPositions(superdataset,
			permutation, size, free_superdataset_on_release);
	free(permutation);
	return subset;
}

static int64_t getPositionCloserToZero(MknnDataset *superdataset,
		MknnDistance *distance) {
	MknnDomain *dom = mknn_dataset_getDomain(superdataset);
	MknnDataset *query_subset = NULL;
	if (mknn_domain_isGeneralDomainVector(dom)) {
		void *vector = mknn_domain_vector_createNewEmptyVectors(dom, 1);
		query_subset = mknn_datasetLoader_PointerCompactVectors_alt(vector,
		true, 1, dom, false);
	} else {
		return 0;
	}
	MknnIndex *index = mknn_index_newPredefined(
			mknn_predefIndex_LinearScan_indexParams(), true, superdataset,
			false, distance, false);
	MknnResolver *resolver = mknn_index_newResolver(index,
			mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(1, 0, 1),
			true);
	MknnResult *result = mknn_resolver_search(resolver, true, query_subset,
	true);
	MknnResultQuery *res = mknn_result_getResultQuery(result, 0);
	my_assert_equalInt("num_nns", res->num_nns, 1);
	int64_t nn_position = res->nn_position[0];
	mknn_result_release(result);
	mknn_index_release(index);
	return nn_position;
}
MknnDataset *mknn_datasetLoader_reorderNearestNeighbor(
		MknnDataset *superdataset, MknnDistance *distance,
		int64_t start_position,
		bool free_superdataset_on_release) {
	int64_t max_size = mknn_dataset_getNumObjects(superdataset);
	int64_t *sorted_array = MY_MALLOC(max_size, int64_t);
	for (int64_t i = 0; i < max_size; ++i)
		sorted_array[i] = i;
	if (start_position < 0) {
		start_position = getPositionCloserToZero(superdataset, distance);
	}
	my_assert_indexRangeInt("start_position", start_position, max_size);
	if (start_position != 0) {
		int64_t tmp = sorted_array[0];
		sorted_array[0] = sorted_array[start_position];
		sorted_array[start_position] = tmp;
	}
	int64_t sorted_size = 1;
	while (sorted_size < max_size) {
		if (false) {
			char *st1 = my_newString_arrayInt(sorted_array, sorted_size, ' ');
			char *st2 = my_newString_arrayInt(sorted_array + sorted_size,
					max_size - sorted_size, ' ');
			my_log_info("sorted=%s\n    left=%s\n", st1, st2);
			free(st1);
			free(st2);
		}
		MknnDataset *query_subset = mknn_datasetLoader_SubsetSegment(
				superdataset, sorted_array[sorted_size - 1], 1, false);
		MknnDataset *ref_subset = mknn_datasetLoader_SubsetPositions(
				superdataset, sorted_array + sorted_size,
				max_size - sorted_size, false);
		MknnIndex *index = mknn_index_newPredefined(
				mknn_predefIndex_LinearScan_indexParams(), true, ref_subset,
				true, distance, false);
		MknnResolver *resolver = mknn_index_newResolver(index,
				mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(1, 0,
						1), true);
		MknnResult *result = mknn_resolver_search(resolver, true, query_subset,
		true);
		MknnResultQuery *res = mknn_result_getResultQuery(result, 0);
		my_assert_equalInt("num_nns", res->num_nns, 1);
		int64_t nn_position = sorted_size + res->nn_position[0];
		mknn_result_release(result);
		mknn_index_release(index);
		if (nn_position != sorted_size) {
			int64_t tmp = sorted_array[sorted_size];
			sorted_array[sorted_size] = sorted_array[nn_position];
			sorted_array[nn_position] = tmp;
		}
		sorted_size++;
	}
	MknnDataset *sortedset = mknn_datasetLoader_SubsetPositions(superdataset,
			sorted_array, max_size, free_superdataset_on_release);
	free(sorted_array);
	return sortedset;
}
