/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct PointersArray {
	void **object_array;
	int64_t num_objects;
	bool free_each_object_on_release;
	bool free_object_array_on_release;
};
static int64_t func_getNumObjects_pointersArray(void *data_pointer) {
	struct PointersArray *data = data_pointer;
	return data->num_objects;
}
static void *func_getObject_pointersArray(void *data_pointer, int64_t pos) {
	struct PointersArray *data = data_pointer;
	my_assert_indexRangeInt("pos", pos, data->num_objects);
	return data->object_array[pos];
}
static void func_releaseDataPointer_pointersArray(void *data_pointer) {
	struct PointersArray *data = data_pointer;
	if (data->free_each_object_on_release) {
		for (int64_t pos = 0; pos < data->num_objects; ++pos) {
			if (data->object_array[pos] != NULL)
				free(data->object_array[pos]);
		}
	}
	if (data->free_object_array_on_release && data->object_array != NULL) {
		free(data->object_array);
	}
	free(data);
}
MknnDataset *mknn_datasetLoader_PointerArray(void **object_array,
		int64_t num_objects, MknnDomain *domain,
		bool free_each_object_on_dataset_release,
		bool free_object_array_on_dataset_release,
		bool free_domain_on_dataset_release) {
	struct PointersArray *data = MY_MALLOC(1, struct PointersArray);
	data->object_array = object_array;
	data->num_objects = num_objects;
	data->free_each_object_on_release = free_each_object_on_dataset_release;
	data->free_object_array_on_release = free_object_array_on_dataset_release;
	MknnDataset *dataset = mknn_datasetLoader_Custom(data,
			func_getNumObjects_pointersArray, func_getObject_pointersArray,
			NULL, func_releaseDataPointer_pointersArray, domain,
			free_domain_on_dataset_release);
	return dataset;
}
