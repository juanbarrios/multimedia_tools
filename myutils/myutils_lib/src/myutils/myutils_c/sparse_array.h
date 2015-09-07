/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_SPARSE_ARRAY_H
#define MY_SPARSE_ARRAY_H

#include "../myutils_c.h"

typedef struct MySparseArray MySparseArray;

MySparseArray *my_sparseArray_new();
void my_sparseArray_storeArrayDouble(MySparseArray *sparseArray, double *array,
		int32_t array_size);
void my_sparseArray_restoreArrayDouble(MySparseArray *sparseArray,
		double *array, int32_t array_size);
void my_sparseArray_restoreArrayFloat(MySparseArray *sparseArray, float *array,
		int32_t array_size);
void my_sparseArray_release(MySparseArray *sparseArray);
char *my_sparseArray_toString(MySparseArray *sparseArray);
MySparseArray *my_sparseArray_clone(MySparseArray *sparseArray);
size_t my_sparseArray_serializePredictSizeBytes(MySparseArray *sparseArray);
size_t my_sparseArray_serialize(MySparseArray *sparseArray, void *data_buffer);
size_t my_sparseArray_deserializePredictReadBytes(void *data_buffer);
size_t my_sparseArray_deserialize(void *data_buffer,
		MySparseArray *sparseArray);
double my_sparseArray_multiplyWeightsEqualId(MySparseArray *sparseArray1,
		MySparseArray *sparseArray2);

#endif
