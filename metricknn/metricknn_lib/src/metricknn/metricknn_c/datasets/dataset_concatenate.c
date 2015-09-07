/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct ListOfDatasets {
	int64_t num_objects, num_subdatasets;
	int64_t *pos_to_numSubdataset, *pos_to_posInSubdataset;
	bool free_subdatasets_on_release;
	MknnDataset **subdatasets;
};
static int64_t func_getNumObjects_concatenate(void *data_pointer) {
	struct ListOfDatasets *list = data_pointer;
	return list->num_objects;
}
static void *func_getObject_concatenate(void *data_pointer, int64_t pos) {
	struct ListOfDatasets *list = data_pointer;
	my_assert_indexRangeInt("pos", pos, list->num_objects);
	int64_t i = list->pos_to_numSubdataset[pos];
	int64_t j = list->pos_to_posInSubdataset[pos];
	return mknn_dataset_getObject(list->subdatasets[i], j);
}
static void func_releaseDataPointer_concatenate(void *data_pointer) {
	struct ListOfDatasets *list = data_pointer;
	if (list->free_subdatasets_on_release) {
		for (int64_t i = 0; i < list->num_subdatasets; ++i) {
			mknn_dataset_release(list->subdatasets[i]);
		}
	}
	free(list->subdatasets);
	free(list->pos_to_numSubdataset);
	free(list->pos_to_posInSubdataset);
	free(list);
}
MknnDataset *mknn_datasetLoader_Concatenate(int64_t num_subdatasets,
		MknnDataset **subdatasets, bool free_subdatasets_on_dataset_release) {
	my_assert_greaterInt("num_datasets", num_subdatasets, 0);
	MknnDomain *dom = NULL;
	for (int64_t i = 0; i < num_subdatasets; ++i) {
		MknnDomain *dom_i = mknn_dataset_getDomain(subdatasets[i]);
		if (mknn_dataset_getNumObjects(subdatasets[i]) == 0)
			continue;
		else if (dom == NULL)
			dom = dom_i;
		else if (!mknn_domain_testEqual(dom_i, dom)) {
			my_log_error(
					"error generating the CONCATENATE dataset. Different domains.\nAll the subdatasets must be the same domain.\n");
		}
	}
	if (dom == NULL)
		dom = mknn_dataset_getDomain(subdatasets[0]);
	struct ListOfDatasets *list = MY_MALLOC(1, struct ListOfDatasets);
	list->num_subdatasets = num_subdatasets;
	list->subdatasets = MY_MALLOC(num_subdatasets, MknnDataset*);
	list->free_subdatasets_on_release = free_subdatasets_on_dataset_release;
	list->num_objects = 0;
	for (int64_t i = 0; i < num_subdatasets; ++i) {
		list->subdatasets[i] = subdatasets[i];
		list->num_objects += mknn_dataset_getNumObjects(list->subdatasets[i]);
	}
	list->pos_to_numSubdataset = MY_MALLOC(list->num_objects, int64_t);
	list->pos_to_posInSubdataset = MY_MALLOC(list->num_objects, int64_t);
	int64_t pos = 0;
	for (int64_t i = 0; i < num_subdatasets; ++i) {
		int64_t size = mknn_dataset_getNumObjects(list->subdatasets[i]);
		for (int64_t j = 0; j < size; ++j) {
			list->pos_to_numSubdataset[pos] = i;
			list->pos_to_posInSubdataset[pos] = j;
			pos++;
		}
	}
	my_assert_equalInt("size", pos, list->num_objects);
	return mknn_datasetLoader_Custom(list, func_getNumObjects_concatenate,
			func_getObject_concatenate, NULL,
			func_releaseDataPointer_concatenate, mknn_domain_newClone(dom),
			true);
}
int64_t mknn_dataset_concatenate_getNumSubDatasets(
		MknnDataset *concatenate_dataset) {
	struct ListOfDatasets *list = mknn_dataset_custom_getDataPointer(
			concatenate_dataset);
	return list->num_subdatasets;
}
MknnDataset *mknn_dataset_concatenate_getSubDataset(
		MknnDataset *concatenate_dataset, int64_t num_subdataset) {
	struct ListOfDatasets *list = mknn_dataset_custom_getDataPointer(
			concatenate_dataset);
	my_assert_indexRangeInt("num_subdataset", num_subdataset,
			list->num_subdatasets);
	return list->subdatasets[num_subdataset];
}
void mknn_dataset_concatenate_getDatasetObject(MknnDataset *concatenate_dataset,
		int64_t posObject, int64_t *out_numSubdataset,
		int64_t *out_posObjectInSubdataset) {
	struct ListOfDatasets *list = mknn_dataset_custom_getDataPointer(
			concatenate_dataset);
	my_assert_indexRangeInt("posObject", posObject, list->num_objects);
	if (out_numSubdataset != NULL)
		*out_numSubdataset = list->pos_to_numSubdataset[posObject];
	if (out_posObjectInSubdataset != NULL)
		*out_posObjectInSubdataset = list->pos_to_posInSubdataset[posObject];
}
