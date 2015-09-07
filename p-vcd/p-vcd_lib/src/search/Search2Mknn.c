/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

MknnDomain *getMknnDomainDescriptorArray(DescriptorType td) {
	if (td.dtype == DTYPE_ARRAY_UCHAR) {
		return mknn_domain_newVector(td.array_length,
				MKNN_DATATYPE_UNSIGNED_INTEGER_8bits);
	} else if (td.dtype == DTYPE_ARRAY_FLOAT) {
		return mknn_domain_newVector(td.array_length,
				MKNN_DATATYPE_FLOATING_POINT_32bits);
	} else if (td.dtype == DTYPE_ARRAY_DOUBLE) {
		return mknn_domain_newVector(td.array_length,
				MKNN_DATATYPE_FLOATING_POINT_64bits);
	}
	return NULL;
}
struct GlobalDataset {
	int64_t num_segments;
	struct SearchSegment **segments;
};
static int64_t gd_getNumObjects(void *data_pointer) {
	struct GlobalDataset *data = data_pointer;
	return data->num_segments;
}
static void *gd_getObject(void *data_pointer, int64_t pos) {
	struct GlobalDataset *data = data_pointer;
	my_assert_indexRangeInt("pos", pos, data->num_segments);
	return data->segments[pos]->descriptor;
}

MknnDataset *get_dataset_global_descriptors(struct SearchCollection *col,
		struct SearchSegment **segments, int64_t num_segments) {
	MknnDomain *domain = NULL;
	if (col->numModalities == 1) {
		DescriptorType td = col->modalities[0];
		domain = getMknnDomainDescriptorArray(td);
	}
	struct GlobalDataset *data = MY_MALLOC(1, struct GlobalDataset);
	data->num_segments = num_segments;
	data->segments = segments;
	MknnDataset *dataset = mknn_datasetLoader_Custom(data, gd_getNumObjects,
			gd_getObject, NULL, free, domain, true);
	return dataset;
}
int64_t getNumLocalVectors(struct SearchSegment **segments,
		int64_t num_segments) {
	int64_t totalVectors = 0;
	for (int64_t i = 0; i < num_segments; ++i) {
		MyLocalDescriptors *des = segments[i]->descriptor;
		totalVectors += my_localDescriptors_getNumDescriptors(des);
	}
	return totalVectors;
}
MknnDataset *get_dataset_local_descriptors(struct SearchCollection *col,
		struct SearchSegment **segments, int64_t num_segments) {
	if (col->numModalities != 1)
		my_log_error("can't combine multiple local descriptors\n");
	DescriptorType td = col->modalities[0];
	if (td.dtype != DTYPE_LOCAL_VECTORS)
		my_log_error("dataset must contain local descriptors\n");
	MknnDataset **subsets = MY_MALLOC(num_segments, MknnDataset*);
	for (int64_t i = 0; i < num_segments; ++i) {
		subsets[i] = my_localDescriptors_createMknnDataset_vectors(
				segments[i]->descriptor, false);
	}
	MknnDataset *dataset = mknn_datasetLoader_Concatenate(num_segments, subsets,
	true);
	free(subsets);
	return dataset;
}
MknnDataset *profile_get_reference_globalDescriptors(
		struct SearchProfile *profile) {
	return get_dataset_global_descriptors(profile->colReference,
			profile->colReference->allSegments,
			profile->colReference->totalSegments);
}
MknnDataset *profile_get_query_globalDescriptors(struct SearchProfile *profile) {
	return get_dataset_global_descriptors(profile->colQuery,
			profile->colQuery->allSegments, profile->colQuery->totalSegments);
}
