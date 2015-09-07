/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MultiObjectDataset {
	int64_t num_subdatasets;
	bool free_subdatasets_on_release;
	MknnDataset **subdatasets;
	int64_t num_objects;
	void ***object_array;
};
static int64_t func_getNumObjects_multiobject(void *data_pointer) {
	struct MultiObjectDataset *data = data_pointer;
	return data->num_objects;
}
static void *func_getObject_multiobject(void *data_pointer, int64_t pos) {
	struct MultiObjectDataset *data = data_pointer;
	my_assert_indexRangeInt("pos", pos, data->num_objects);
	return data->object_array[pos];
}
static void func_releaseDataPointer_multiobject(void *data_pointer) {
	struct MultiObjectDataset *data = data_pointer;
	for (int64_t pos = 0; pos < data->num_objects; ++pos)
		MY_FREE(data->object_array[pos]);
	MY_FREE(data->object_array);
	if (data->free_subdatasets_on_release) {
		for (int64_t i = 0; i < data->num_subdatasets; ++i) {
			mknn_dataset_release(data->subdatasets[i]);
		}
	}
	MY_FREE(data->subdatasets);
	MY_FREE(data);
}
MknnDataset *mknn_datasetLoader_MultiObject(int64_t num_subdatasets,
		MknnDataset **subdatasets, bool free_subdatasets_on_dataset_release) {
	struct MultiObjectDataset *data = MY_MALLOC(1, struct MultiObjectDataset);
	data->num_subdatasets = num_subdatasets;
	data->subdatasets = MY_MALLOC(num_subdatasets, MknnDataset*);
	data->free_subdatasets_on_release = free_subdatasets_on_dataset_release;
	data->num_objects = 0;
	MknnDomain **subdomains = MY_MALLOC(num_subdatasets, MknnDomain*);
	for (int64_t i = 0; i < num_subdatasets; ++i) {
		int64_t size = mknn_dataset_getNumObjects(subdatasets[i]);
		subdomains[i] = mknn_dataset_getDomain(subdatasets[i]);
		if (i == 0) {
			data->num_objects = size;
		} else if (data->num_objects != size) {
			my_log_error(
					"error generating the MULTIOBJECT dataset. Different lengths.\nAll the modalities must contain the same number of objects.\n");
		}
	}
	if (data->num_objects > 0)
		data->object_array = MY_MALLOC(data->num_objects, void**);
	for (int64_t i = 0; i < data->num_objects; ++i) {
		data->object_array[i] = MY_MALLOC(num_subdatasets, void*);
		for (int64_t j = 0; j < num_subdatasets; ++j) {
			data->object_array[i][j] = mknn_dataset_getObject(subdatasets[j],
					i);
		}
	}
	MknnDomain *multi_domain = mknn_domain_newMultiobject(num_subdatasets,
			subdomains, false);
	MknnDataset *new_dataset = mknn_datasetLoader_Custom(data,
			func_getNumObjects_multiobject, func_getObject_multiobject, NULL,
			func_releaseDataPointer_multiobject, multi_domain, true);
	free(subdomains);
	return new_dataset;
}
int64_t mknn_dataset_multiobject_getNumSubDatasets(
		MknnDataset *multiobject_dataset) {
	if (!mknn_domain_isGeneralDomainMultiObject(
			mknn_dataset_getDomain(multiobject_dataset))) {
		my_log_error("dataset must be multiobject\n");
	}
	struct MultiObjectDataset *data = mknn_dataset_custom_getDataPointer(
			multiobject_dataset);
	return data->num_subdatasets;
}
MknnDataset *mknn_dataset_multiobject_getSubDataset(
		MknnDataset *multiobject_dataset, int64_t num_subdataset) {
	if (!mknn_domain_isGeneralDomainMultiObject(
			mknn_dataset_getDomain(multiobject_dataset))) {
		my_log_error("dataset must be multiobject\n");
	}
	struct MultiObjectDataset *data = mknn_dataset_custom_getDataPointer(
			multiobject_dataset);
	my_assert_indexRangeInt("num_subdataset", num_subdataset,
			data->num_subdatasets);
	return data->subdatasets[num_subdataset];
}
