/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct SubsetDataset {
	int64_t num_objects, offset;
	MknnDataset *superdataset;
	bool free_superdataset_on_release;
};
static int64_t func_getNumObjects_subset(void *data_pointer) {
	struct SubsetDataset *subset = data_pointer;
	return subset->num_objects;
}
static void *func_getObject_subset(void *data_pointer, int64_t pos) {
	struct SubsetDataset *subset = data_pointer;
	my_assert_indexRangeInt("pos", pos, subset->num_objects);
	return mknn_dataset_getObject(subset->superdataset, pos + subset->offset);
}
static void func_releaseDataPointer_subset(void *data_pointer) {
	struct SubsetDataset *subset = data_pointer;
	if (subset->free_superdataset_on_release)
		mknn_dataset_release(subset->superdataset);
	free(subset);
}
MknnDataset *mknn_datasetLoader_SubsetSegment(MknnDataset *superdataset,
		int64_t position_start, int64_t length,
		bool free_superdataset_on_release) {
	struct SubsetDataset *subset = MY_MALLOC(1, struct SubsetDataset);
	subset->num_objects = MIN(length,
			mknn_dataset_getNumObjects(superdataset) - position_start);
	subset->offset = position_start;
	subset->superdataset = superdataset;
	return mknn_datasetLoader_Custom(subset, func_getNumObjects_subset,
			func_getObject_subset, NULL, func_releaseDataPointer_subset,
			mknn_dataset_getDomain(superdataset),
			false);
}
