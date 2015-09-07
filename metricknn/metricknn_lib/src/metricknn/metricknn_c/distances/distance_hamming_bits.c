/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

static int bits_cont(uint8_t c) {
	int cont = 0;
	for (int i = 0; i < 8; ++i) {
		if (c & 1)
			cont++;
		c >>= 1;
	}
	return cont;
}
static void *compute_bits_table() {
	int64_t *table = MY_MALLOC(256, int64_t);
	for (int i = 0; i < 256; ++i)
		table[i] = bits_cont(i);
	return table;
}
static int64_t bits_cont_uint8_t(uint8_t c1, uint8_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint8_t c = (c1 ^ c2);
	return table[c];
}
static int64_t bits_cont_int8_t(int8_t c1, int8_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint8_t c = (uint8_t) (c1 ^ c2);
	return table[c];
}
static int64_t bits_cont_uint16_t(uint16_t c1, uint16_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint16_t c = (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF];
}
static int64_t bits_cont_int16_t(int16_t c1, int16_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint16_t c = (uint16_t) (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF];
}
static int64_t bits_cont_uint32_t(uint32_t c1, uint32_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint32_t c = (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF] + table[(c >> 16) & 0xFF]
			+ table[(c >> 24) & 0xFF];
}
static int64_t bits_cont_int32_t(int32_t c1, int32_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint32_t c = (uint32_t) (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF] + table[(c >> 16) & 0xFF]
			+ table[(c >> 24) & 0xFF];
}
static int64_t bits_cont_uint64_t(uint64_t c1, uint64_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint64_t c = (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF] + table[(c >> 16) & 0xFF]
			+ table[(c >> 24) & 0xFF] + table[(c >> 32) & 0xFF]
			+ table[(c >> 40) & 0xFF] + table[(c >> 48) & 0xFF]
			+ table[(c >> 56) & 0xFF];
}
static int64_t bits_cont_int64_t(int64_t c1, int64_t c2, void *table_ptr) {
	int64_t *table = table_ptr;
	uint64_t c = (uint64_t) (c1 ^ c2);
	return table[c & 0xFF] + table[(c >> 8) & 0xFF] + table[(c >> 16) & 0xFF]
			+ table[(c >> 24) & 0xFF] + table[(c >> 32) & 0xFF]
			+ table[(c >> 40) & 0xFF] + table[(c >> 48) & 0xFF]
			+ table[(c >> 56) & 0xFF];
}
static int64_t bits_cont_float(float c1, float c2, void *table_ptr) {
	return bits_cont_int32_t((int32_t) c1, (int32_t) c2, table_ptr);
}
static int64_t bits_cont_double(double c1, double c2, void *table_ptr) {
	return bits_cont_int64_t((int64_t) c1, (int64_t) c2, table_ptr);
}

#define CONT_BITS_FUNCTIONS(typeVector) \
static double bits_distanceEval_##typeVector(void *state_distEval, void *object_left, void *object_right, double current_threshold) { \
	MknnDomain *domain_object = state_distEval; \
	typeVector *array1 = (typeVector*) object_left; \
	typeVector *array2 = (typeVector*) object_right; \
	int64_t numN = mknn_domain_vector_getNumDimensions(domain_object) / 4; \
	int64_t sum = 0; \
	while (numN > 0) { \
		sum += bits_cont_##typeVector(array1[0], array2[0], state_distEval); \
		sum += bits_cont_##typeVector(array1[1], array2[1], state_distEval); \
		sum += bits_cont_##typeVector(array1[2], array2[2], state_distEval); \
		sum += bits_cont_##typeVector(array1[3], array2[3], state_distEval); \
		if (sum > current_threshold) \
			return sum; \
		array1 += 4; \
		array2 += 4; \
		numN--; \
	} \
	numN = mknn_domain_vector_getNumDimensions(domain_object) % 4; \
	while (numN > 0) { \
		sum += bits_cont_##typeVector(array1[0], array2[0], state_distEval); \
		array1 += 1; \
		array2 += 1; \
		numN--; \
	} \
	return sum; \
}

CONT_BITS_FUNCTIONS(int8_t)
CONT_BITS_FUNCTIONS(int16_t)
CONT_BITS_FUNCTIONS(int32_t)
CONT_BITS_FUNCTIONS(int64_t)
CONT_BITS_FUNCTIONS(uint8_t)
CONT_BITS_FUNCTIONS(uint16_t)
CONT_BITS_FUNCTIONS(uint32_t)
CONT_BITS_FUNCTIONS(uint64_t)
CONT_BITS_FUNCTIONS(float)
CONT_BITS_FUNCTIONS(double)

static struct MknnDistEvalInstance bits_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	int64_t dims1 = mknn_domain_vector_getNumDimensions(domain_left);
	int64_t dims2 = mknn_domain_vector_getNumDimensions(domain_right);
	if (dims1 != dims2)
		my_log_error(
				"distance does not support different number of dimensions (%"PRIi64"!=%"PRIi64")\n",
				dims1, dims2);
	MknnDatatype datatype1 = mknn_domain_vector_getDimensionDataType(
			domain_left);
	MknnDatatype datatype2 = mknn_domain_vector_getDimensionDataType(
			domain_right);
	if (!mknn_datatype_areEqual(datatype1, datatype2))
		my_log_error("distance does not support different datatypes\n");
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state_distance;
	ASSIGN_SINGLE_DATATYPE(di.func_distanceEval_eval, datatype1,
			bits_distanceEval_)
	return di;
}
static struct MknnDistanceInstance bits_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct MknnDistanceInstance df = { 0 };
	df.func_distanceEval_new = bits_distanceEval_new;
	df.state_distance = compute_bits_table();
	df.func_distance_release = free;
	return df;
}

void register_distance_bits() {
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "BITS", NULL,
	NULL, bits_dist_new);
}
