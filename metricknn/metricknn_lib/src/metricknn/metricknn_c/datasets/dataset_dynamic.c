/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct DynamicArray {
	void **object_array;
	int64_t num_objects;
};
static int64_t func_getNumObjects_dynamicDataset(void *data_pointer) {
	struct DynamicArray *data = data_pointer;
	return data->num_objects;
}
static void *func_getObject_dynamicDataset(void *data_pointer, int64_t pos) {
	struct DynamicArray *data = data_pointer;
	my_assert_indexRangeInt("pos", pos, data->num_objects);
	return data->object_array[pos];
}
static void func_pushObject_dynamicDataset(void *data_pointer, void *object) {
	struct DynamicArray *data = data_pointer;
	MY_REALLOC(data->object_array, data->num_objects + 1, void*);
	data->object_array[data->num_objects] = object;
	data->num_objects++;
}
static void func_releaseDataPointer_dynamicDataset(void *data_pointer) {
	struct DynamicArray *data = data_pointer;
	free(data->object_array);
	free(data);
}
MknnDataset *mknn_datasetLoader_Empty(MknnDomain *domain,
bool free_domain_on_dataset_release) {
	struct DynamicArray *data = MY_MALLOC(1, struct DynamicArray);
	data->object_array = NULL;
	data->num_objects = 0;
	return mknn_datasetLoader_Custom(data, func_getNumObjects_dynamicDataset,
			func_getObject_dynamicDataset, func_pushObject_dynamicDataset,
			func_releaseDataPointer_dynamicDataset, domain,
			free_domain_on_dataset_release);
}
