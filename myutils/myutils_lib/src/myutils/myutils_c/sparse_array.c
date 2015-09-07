/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "sparse_array.h"

struct ArraySlot {
	int32_t id_dimension;
	float weight_dimension;
};

struct MySparseArray {
	int32_t filled_slots;
	struct ArraySlot *slots_buffer;
	int32_t slots_buffer_size;
};

MySparseArray *my_sparseArray_new() {
	MySparseArray *sparseArray = MY_MALLOC(1, MySparseArray);
	return sparseArray;
}
void my_sparseArray_storeArrayDouble(MySparseArray *sparseArray, double *array,
		int32_t array_size) {
	sparseArray->filled_slots = 0;
	for (int32_t i = 0; i < array_size; ++i) {
		if (array[i] == 0)
			continue;
		if (sparseArray->filled_slots >= sparseArray->slots_buffer_size) {
			sparseArray->slots_buffer_size = MAX(
					2 * sparseArray->slots_buffer_size, 64);
			MY_REALLOC(sparseArray->slots_buffer,
					sparseArray->slots_buffer_size, struct ArraySlot);
		}
		struct ArraySlot *slot = sparseArray->slots_buffer
				+ sparseArray->filled_slots;
		slot->id_dimension = i;
		slot->weight_dimension = array[i];
		sparseArray->filled_slots++;
	}
}
void my_sparseArray_restoreArrayDouble(MySparseArray *sparseArray,
		double *array, int32_t array_size) {
	MY_SETZERO(array, array_size, double);
	for (int32_t i = 0; i < sparseArray->filled_slots; ++i) {
		struct ArraySlot *slot = sparseArray->slots_buffer + i;
		array[slot->id_dimension] = slot->weight_dimension;
	}
}
void my_sparseArray_restoreArrayFloat(MySparseArray *sparseArray, float *array,
		int32_t array_size) {
	MY_SETZERO(array, array_size, float);
	for (int32_t i = 0; i < sparseArray->filled_slots; ++i) {
		struct ArraySlot *slot = sparseArray->slots_buffer + i;
		array[slot->id_dimension] = slot->weight_dimension;
	}
}
double my_sparseArray_multiplyWeightsEqualId(MySparseArray *sparseArray1,
		MySparseArray *sparseArray2) {
	if (sparseArray1->filled_slots == 0 || sparseArray2->filled_slots == 0)
		return 0;
	double sum = 0;
	int32_t j = 0;
	for (int32_t i = 0; i < sparseArray1->filled_slots; ++i) {
		struct ArraySlot *slot1 = sparseArray1->slots_buffer + i;
		for (;;) {
			if (j >= sparseArray2->filled_slots)
				break;
			struct ArraySlot *slot2 = sparseArray2->slots_buffer + j;
			if (slot2->id_dimension < slot1->id_dimension) {
				j++;
			} else if (slot2->id_dimension == slot1->id_dimension) {
				sum += slot2->weight_dimension * slot1->weight_dimension;
				j++;
				break;
			} else {
				break;
			}
		}
	}
	return sum;
}
void my_sparseArray_release(MySparseArray *sparseArray) {
	if (sparseArray == NULL)
		return;
	if (sparseArray->slots_buffer != NULL)
		free(sparseArray->slots_buffer);
	free(sparseArray);
}
char *my_sparseArray_toString(MySparseArray *sparseArray) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int32_t i = 0; i < sparseArray->filled_slots; ++i) {
		struct ArraySlot *slot = sparseArray->slots_buffer + i;
		char *st_1 = my_newString_int(slot->id_dimension);
		char *st_2 = my_newString_float(slot->weight_dimension);
		char *st_3 = my_newString_format("%s%s->%s", (i == 0 ? "" : "\t"), st_1,
				st_2);
		my_stringbuf_appendString(sb, st_3);
		free(st_1);
		free(st_2);
		free(st_3);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
MySparseArray *my_sparseArray_clone(MySparseArray *sparseArray) {
	MySparseArray *newSp = my_sparseArray_new();
	if (sparseArray->filled_slots > 0) {
		newSp->slots_buffer_size = newSp->filled_slots =
				sparseArray->filled_slots;
		MY_REALLOC(newSp->slots_buffer, newSp->slots_buffer_size,
				struct ArraySlot);
		memcpy(newSp->slots_buffer, sparseArray->slots_buffer,
				sparseArray->filled_slots * sizeof(struct ArraySlot));
	}
	return newSp;
}
size_t my_sparseArray_serializePredictSizeBytes(MySparseArray *sparseArray) {
	return sizeof(int32_t)
			+ sparseArray->filled_slots * sizeof(struct ArraySlot);
}
size_t my_sparseArray_serialize(MySparseArray *sparseArray, void *data_buffer) {
	int32_t *header = data_buffer;
	header[0] = sparseArray->filled_slots;
	if (sparseArray->filled_slots > 0) {
		void *start_slots = (header + 1);
		memcpy(start_slots, sparseArray->slots_buffer,
				sparseArray->filled_slots * sizeof(struct ArraySlot));
	}
	return sizeof(int32_t)
			+ sparseArray->filled_slots * sizeof(struct ArraySlot);
}
size_t my_sparseArray_deserializePredictReadBytes(void *data_buffer) {
	if (data_buffer == NULL)
		return 0;
	int32_t filled_slots = ((int32_t*) data_buffer)[0];
	return sizeof(int32_t) + filled_slots * sizeof(struct ArraySlot);
}
size_t my_sparseArray_deserialize(void *data_buffer, MySparseArray *sparseArray) {
	if (data_buffer == NULL)
		return 0;
	int32_t *header = data_buffer;
	sparseArray->filled_slots = header[0];
	if (sparseArray->filled_slots > 0) {
		sparseArray->slots_buffer_size = sparseArray->filled_slots;
		MY_REALLOC(sparseArray->slots_buffer, sparseArray->slots_buffer_size,
				struct ArraySlot);
		void *start_slots = (header + 1);
		memcpy(sparseArray->slots_buffer, start_slots,
				sparseArray->filled_slots * sizeof(struct ArraySlot));
	}
	return sizeof(int32_t)
			+ sparseArray->filled_slots * sizeof(struct ArraySlot);
}
