/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct DataArray {
	char *vectors_header;
	int64_t num_vectors;
	int64_t array_length_in_bytes;
	bool free_vectors_header_on_release;
};
static int64_t func_getNumObjects_dataArray(void *data_pointer) {
	struct DataArray *data = data_pointer;
	return data->num_vectors;
}
static void *func_getObject_dataArray(void *data_pointer, int64_t pos) {
	struct DataArray *data = data_pointer;
	my_assert_indexRangeInt("pos", pos, data->num_vectors);
	if (data->array_length_in_bytes == 0)
		my_log_error("the dataset needs a vector domain\n");
	return data->vectors_header + pos * data->array_length_in_bytes;
}
static void func_releaseDataPointer_dataArray(void *data_pointer) {
	struct DataArray *data = data_pointer;
	if (data->free_vectors_header_on_release && data->vectors_header != NULL)
		free(data->vectors_header);
	free(data);
}
MknnDataset *mknn_datasetLoader_PointerCompactVectors_alt(void *vectors_header,
bool free_vectors_header_on_dataset_release, int64_t num_vectors,
		MknnDomain *domain,
		bool free_domain_on_dataset_release) {
	if (!mknn_domain_isGeneralDomainVector(domain))
		my_log_error("the dataset needs a vector domain\n");
	struct DataArray *data = MY_MALLOC(1, struct DataArray);
	data->vectors_header = vectors_header;
	data->num_vectors = num_vectors;
	data->array_length_in_bytes = mknn_domain_vector_getVectorLengthInBytes(
			domain);
	data->free_vectors_header_on_release =
			free_vectors_header_on_dataset_release;
	MknnDataset *dataset = mknn_datasetLoader_Custom(data,
			func_getNumObjects_dataArray, func_getObject_dataArray, NULL,
			func_releaseDataPointer_dataArray, domain,
			free_domain_on_dataset_release);
	mknn_dataset_setCompactVectors(dataset, vectors_header, false);
	return dataset;
}
MknnDataset *mknn_datasetLoader_PointerCompactVectors(void *vectors_header,
bool free_vectors_header_on_dataset_release, int64_t num_vectors,
		int64_t vector_dimensions, MknnDatatype vector_dimension_datatype) {
	MknnDomain *domain = mknn_domain_newVector(vector_dimensions,
			vector_dimension_datatype);
	return mknn_datasetLoader_PointerCompactVectors_alt(vectors_header,
			free_vectors_header_on_dataset_release, num_vectors, domain, true);
}
