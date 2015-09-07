/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnDataset {
	void *data_pointer;
	mknn_function_dataset_getNumObjects func_getNumObjects;
	mknn_function_dataset_getObject func_getObject;
	mknn_function_dataset_pushObject func_pushObject;
	mknn_function_dataset_releaseDataPointer func_releaseDataPointer;
	MknnDomain *domain;
	bool free_domain_on_dataset_release;
	//auto-conversion to compact vectors
	void *compactVectors_pointer;
	bool free_compactVectors_on_release;
};
/* ************************************** */
MknnDataset *mknn_datasetLoader_Custom(void *data_pointer,
		mknn_function_dataset_getNumObjects func_getNumObjects,
		mknn_function_dataset_getObject func_getObject,
		mknn_function_dataset_pushObject func_pushObject,
		mknn_function_dataset_releaseDataPointer func_releaseDataPointer,
		MknnDomain *domain, bool free_domain_on_dataset_release) {
	MknnDataset *dataset = MY_MALLOC(1, MknnDataset);
	dataset->data_pointer = data_pointer;
	dataset->func_getNumObjects = func_getNumObjects;
	dataset->func_getObject = func_getObject;
	dataset->func_pushObject = func_pushObject;
	dataset->func_releaseDataPointer = func_releaseDataPointer;
	dataset->domain = domain;
	dataset->free_domain_on_dataset_release = free_domain_on_dataset_release;
	return dataset;
}
void *mknn_dataset_custom_getDataPointer(MknnDataset *custom_dataset) {
	if (custom_dataset == NULL)
		return NULL;
	return custom_dataset->data_pointer;
}

/* ************************************** */
MknnDomain *mknn_dataset_getDomain(MknnDataset *dataset) {
	if (dataset == NULL)
		return NULL;
	return dataset->domain;
}
void *mknn_dataset_getObject(MknnDataset *dataset, int64_t pos) {
	return dataset->func_getObject(dataset->data_pointer, pos);
}
int64_t mknn_dataset_getNumObjects(MknnDataset *dataset) {
	if (dataset == NULL)
		return 0;
	return dataset->func_getNumObjects(dataset->data_pointer);
}
void mknn_dataset_pushObject(MknnDataset *dataset, void *object) {
	if (dataset->func_pushObject == NULL)
		my_log_error("this dataset does not support adding objects\n");
	dataset->func_pushObject(dataset->data_pointer, object);
	//adds the object to the release list
	//releases the cache of getCompactVectors
	if (dataset->compactVectors_pointer != NULL) {
		free(dataset->compactVectors_pointer);
		dataset->compactVectors_pointer = NULL;
	}
}
void mknn_dataset_set_free_domain_on_dataset_release(MknnDataset *dataset,
bool free_domain_on_dataset_release) {
	dataset->free_domain_on_dataset_release = free_domain_on_dataset_release;
}
bool mknn_dataset_get_free_domain_on_dataset_release(MknnDataset *dataset) {
	return dataset->free_domain_on_dataset_release;
}
void mknn_dataset_release(MknnDataset *dataset) {
	if (dataset == NULL)
		return;
	if (dataset->free_compactVectors_on_release
			&& dataset->compactVectors_pointer != NULL)
		free(dataset->compactVectors_pointer);
	if (dataset->func_releaseDataPointer != NULL
			&& dataset->data_pointer != NULL)
		dataset->func_releaseDataPointer(dataset->data_pointer);
	if (dataset->free_domain_on_dataset_release && dataset->domain != NULL)
		mknn_domain_release(dataset->domain);
	free(dataset);
}
/* ************************************** */
static void *copy_vectors_to_single_array(MknnDataset *dataset) {
	if (dataset == NULL)
		return NULL;
	int64_t length_in_bytes = mknn_domain_vector_getVectorLengthInBytes(
			dataset->domain);
	if (length_in_bytes == 0)
		my_log_error("dataset must contain vectors in order to copy them\n");
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int64_t total_length_in_bytes = num_objects * length_in_bytes;
	char *data_array = MY_MALLOC_NOINIT(total_length_in_bytes, char);
	if (dataset->compactVectors_pointer != NULL) {
		memcpy(data_array, dataset->compactVectors_pointer,
				total_length_in_bytes);
	} else {
		char *ptr = data_array;
		for (int64_t i = 0; i < num_objects; ++i) {
			void *object = mknn_dataset_getObject(dataset, i);
			memcpy(ptr, object, length_in_bytes);
			ptr += length_in_bytes;
		}
	}
	return data_array;
}
MknnDataset *mknn_dataset_clone(MknnDataset *dataset) {
	void *data_array = copy_vectors_to_single_array(dataset);
	return mknn_datasetLoader_PointerCompactVectors_alt(data_array, true,
			mknn_dataset_getNumObjects(dataset),
			mknn_domain_newClone(mknn_dataset_getDomain(dataset)), true);
}
void *mknn_dataset_getCompactVectors(MknnDataset *dataset) {
	if (dataset == NULL)
		return NULL;
	if (dataset->compactVectors_pointer != NULL)
		return dataset->compactVectors_pointer;
	void *data_array = copy_vectors_to_single_array(dataset);
	dataset->compactVectors_pointer = data_array;
	dataset->free_compactVectors_on_release = true;
	return data_array;
}
void mknn_dataset_setCompactVectors(MknnDataset *dataset,
		void *compactVectors_pointer, bool free_compactVectors_on_release) {
	dataset->compactVectors_pointer = compactVectors_pointer;
	dataset->free_compactVectors_on_release = free_compactVectors_on_release;
}

/* ************************************** */
void mknn_dataset_computeStatsVectors(MknnDataset *dataset,
		struct MyDataStatsCompute *stats) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	my_assert_isTrue("is vector", mknn_domain_isGeneralDomainVector(domain));
	int64_t numdim = mknn_domain_vector_getNumDimensions(domain);
	my_assert_equalInt("num dimensions", numdim,
			my_math_computeStats_getNumDimensions(stats));
	MyDatatype mytype = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(domain));
	MyProgress *lt = NULL;
	if (mknn_dataset_getNumObjects(dataset) > 100000)
		lt = my_progress_new("statistics", mknn_dataset_getNumObjects(dataset),
				1);
	my_math_computeStats_addSample funcAddSample =
			my_math_computeStats_getAddSampleFunction(mytype);
	for (int64_t i = 0; i < mknn_dataset_getNumObjects(dataset); ++i) {
		if (lt != NULL && i % 10000 == 0)
			my_progress_setN(lt, i);
		void *vector = mknn_dataset_getObject(dataset, i);
		funcAddSample(stats, vector);
	}
	my_progress_setN(lt, mknn_dataset_getNumObjects(dataset));
	my_progress_release(lt);
}
struct MyDataStatsCompute *mknn_dataset_computeDataStats(MknnDataset *dataset) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	int64_t numdim = mknn_domain_vector_getNumDimensions(domain);
	struct MyDataStatsCompute *stats = my_math_computeStats_new(numdim);
	mknn_dataset_computeStatsVectors(dataset, stats);
	return stats;
}
