/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

static void* get_random_double(int64_t size, double minValueIncluded,
		double maxValueNotIncluded) {
	double *double_array = MY_MALLOC(size, double);
	my_random_doubleList(minValueIncluded, maxValueNotIncluded, double_array,
			size);
	return double_array;
}
static void* get_random_int64(int64_t size, double minValueIncluded,
		double maxValueNotIncluded) {
	int64_t *int64_array = MY_MALLOC(size, int64_t);
	my_random_intList(minValueIncluded, maxValueNotIncluded, int64_array, size);
	return int64_array;
}
static void* get_random_float(int64_t size, double minValueIncluded,
		double maxValueNotIncluded) {
	float *array_float = MY_MALLOC(size, float);
	//random generation of double and converted to float
	int64_t buffer_size = MIN(10000, size);
	double *buffer_double = MY_MALLOC(buffer_size, double);
	int64_t current_pos = 0;
	while (current_pos < size) {
		int64_t left = MIN(buffer_size, size - current_pos);
		my_random_doubleList(minValueIncluded, maxValueNotIncluded,
				buffer_double, left);
		for (int64_t i = 0; i < left; ++i) {
			array_float[current_pos + i] = (float) buffer_double[i];
		}
		current_pos += left;
	}
	free(buffer_double);
	return array_float;
}
static void* get_random_genericInt(int64_t size, double minValueIncluded,
		double maxValueNotIncluded, MknnDatatype datatype) {
	size_t bytes_dim = mknn_datatype_sizeof(datatype);
	char *data_array = MY_MALLOC(size * bytes_dim, char);
	//random generation of int64_t and converted to desired type
	int64_t buffer_size = MIN(10000, size);
	int64_t *buffer_int64 = MY_MALLOC(buffer_size, int64_t);
	my_function_copy_vector func_copy = my_datatype_getFunctionCopyVector(
			MY_DATATYPE_INT64, mknn_datatype_convertMknn2My(datatype));
	int64_t current_pos = 0;
	while (current_pos < size) {
		int64_t left = MIN(buffer_size, size - current_pos);
		my_random_intList(minValueIncluded, maxValueNotIncluded, buffer_int64,
				left);
		func_copy(buffer_int64, data_array + current_pos, left);
		current_pos += left;
	}
	free(buffer_int64);
	return data_array;
}
static void* get_random_numbers(int64_t size, double minValueIncluded,
		double maxValueNotIncluded, MknnDatatype datatype) {
	//my_random_testDouble(minValueIncluded, maxValueNotIncluded, size);
	//my_random_testInt(minValueIncluded, maxValueNotIncluded, size);
	if (mknn_datatype_isDouble(datatype)) {
		return get_random_double(size, minValueIncluded, maxValueNotIncluded);
	} else if (mknn_datatype_isFloat(datatype)) {
		return get_random_float(size, minValueIncluded, maxValueNotIncluded);
	} else if (mknn_datatype_isInt64(datatype)) {
		return get_random_int64(size, minValueIncluded, maxValueNotIncluded);
	} else {
		return get_random_genericInt(size, minValueIncluded,
				maxValueNotIncluded, datatype);
	}
}

MknnDataset *mknn_datasetLoader_UniformRandomVectors(int64_t num_objects,
		int64_t dimension, double dimension_minValueIncluded,
		double dimension_maxValueNotIncluded,
		MknnDatatype vectors_dimension_datatype) {
	void *data_array = get_random_numbers(num_objects * dimension,
			dimension_minValueIncluded, dimension_maxValueNotIncluded,
			vectors_dimension_datatype);
	return mknn_datasetLoader_PointerCompactVectors(data_array, true,
			num_objects, dimension, vectors_dimension_datatype);
}
