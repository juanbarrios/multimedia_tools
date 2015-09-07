/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct PositionsDataset {
	int64_t num_objects;
	int64_t *positions;
	MknnDataset *superdataset;
	bool free_superdataset_on_release;
};
static int64_t func_getNumObjects_Pos(void *data_pointer) {
	struct PositionsDataset *subset = data_pointer;
	return subset->num_objects;
}
static void *func_getObject_Pos(void *data_pointer, int64_t pos) {
	struct PositionsDataset *subset = data_pointer;
	my_assert_indexRangeInt("pos", pos, subset->num_objects);
	return mknn_dataset_getObject(subset->superdataset, subset->positions[pos]);
}
static void func_releaseDataPointer_Pos(void *data_pointer) {
	struct PositionsDataset *subset = data_pointer;
	free(subset->positions);
	if (subset->free_superdataset_on_release)
		mknn_dataset_release(subset->superdataset);
	free(subset);
}
MknnDataset *mknn_datasetLoader_SubsetPositions(MknnDataset *superdataset,
		int64_t *positions, int64_t num_positions,
		bool free_superdataset_on_release) {
	struct PositionsDataset *subset = MY_MALLOC(1, struct PositionsDataset);
	subset->num_objects = num_positions;
	subset->positions = MY_MALLOC(num_positions, int64_t);
	for (int64_t i = 0; i < num_positions; ++i)
		subset->positions[i] = positions[i];
	subset->superdataset = superdataset;
	subset->free_superdataset_on_release = free_superdataset_on_release;
	return mknn_datasetLoader_Custom(subset, func_getNumObjects_Pos,
			func_getObject_Pos, NULL, func_releaseDataPointer_Pos,
			mknn_dataset_getDomain(superdataset),
			false);
}
MknnDataset *mknn_datasetLoader_SubsetRandomSample(MknnDataset *superdataset,
		double sample_size_or_fraction, bool free_superdataset_on_release) {
	MyVectorInt *positions = my_random_sampleNoRepetitionsSorted(0,
			mknn_dataset_getNumObjects(superdataset), sample_size_or_fraction);
	MknnDataset *subset = mknn_datasetLoader_SubsetPositions(superdataset,
			my_vectorInt_array(positions), my_vectorInt_size(positions),
			free_superdataset_on_release);
	my_vectorInt_release(positions);
	return subset;
}
