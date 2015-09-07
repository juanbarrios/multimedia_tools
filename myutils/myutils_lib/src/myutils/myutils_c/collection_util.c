/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

int my_compare_double(double da, double db) {
	if (isnan(da))
		return isnan(db) ? 0 : 1;
	else if (isnan(db))
		return -1;
	else
		return (da == db) ? 0 : (da > db ? 1 : -1);
}
int my_compare_int(int64_t da, int64_t db) {
	return (da == db) ? 0 : (da > db ? 1 : -1);
}

static int compare_double_qsort(const void *a, const void *b) {
	double da = *(double*) a;
	double db = *(double*) b;
	return my_compare_double(da, db);
}
static int compare_double_qsortInv(const void *a, const void *b) {
	double da = *(double*) a;
	double db = *(double*) b;
	return my_compare_double(db, da);
}
static int compare_int_qsort(const void *a, const void *b) {
	int64_t da = *(int64_t*) a;
	int64_t db = *(int64_t*) b;
	return my_compare_int(da, db);
}
static int compare_int_qsortInv(const void *a, const void *b) {
	int64_t da = *(int64_t*) a;
	int64_t db = *(int64_t*) b;
	return my_compare_int(db, da);
}
static int compare_uint8_qsort(const void *a, const void *b) {
	uint8_t da = *(uint8_t*) a;
	uint8_t db = *(uint8_t*) b;
	return my_compare_int(da, db);
}

void my_qsort_double_array(double *array, int64_t length) {
	qsort(array, length, sizeof(double), compare_double_qsort);
}
void my_qsort_double_arrayInv(double *array, int64_t length) {
	qsort(array, length, sizeof(double), compare_double_qsortInv);
}
void my_qsort_int_array(int64_t *array, int64_t length) {
	qsort(array, length, sizeof(int64_t), compare_int_qsort);
}
void my_qsort_int_arrayInv(int64_t *array, int64_t length) {
	qsort(array, length, sizeof(int64_t), compare_int_qsortInv);
}
void my_qsort_uint8_array(uint8_t *array, int64_t length) {
	qsort(array, length, sizeof(uint8_t), compare_uint8_qsort);
}

//si se encuentra retorna la position donde esta [0, numElementos-1]
//si no se encuentra retorna  -(newpos - 1) con la position donde deberia estar
//es decir, sumar 1 y cambiar el signo para ver donde tiene que estar
bool my_binsearch_intArr(int64_t search_value, int64_t *sorted_array,
		int64_t array_size, int64_t *out_position) {
	int64_t desde = 0, hasta = array_size;
	while (desde < hasta) {
		int64_t medio = (desde + hasta) / 2;
		int64_t valorMed = sorted_array[medio];
		int cmp = my_compare_int(search_value, valorMed);
		if (cmp > 0) {
			desde = medio + 1;
		} else if (cmp < 0) {
			hasta = medio;
		} else {
			if (out_position != NULL)
				*out_position = medio;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = desde;
	return false;
}
//si se encuentra retorna la position donde esta [0, numElementos-1]
//si no se encuentra retorna  -(newpos - 1) con la position donde deberia estar
//es decir, sumar 1 y cambiar el signo para ver donde tiene que estar
bool my_binsearch_doubleArr(double search_value, double *sorted_array,
		int64_t array_size, int64_t *out_position) {
	int64_t desde = 0, hasta = array_size;
	while (desde < hasta) {
		int64_t medio = (desde + hasta) / 2;
		double valorMed = sorted_array[medio];
		int cmp = my_compare_double(search_value, valorMed);
		if (cmp > 0) {
			desde = medio + 1;
		} else if (cmp < 0) {
			hasta = medio;
		} else {
			if (out_position != NULL)
				*out_position = medio;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = desde;
	return false;
}
bool my_binsearch_objArr(const void *search_value, void **sorted_array,
		int64_t array_size, my_func_compareObj func_compare,
		int64_t *out_position) {
	if (func_compare == NULL)
		my_assert_notNull("func_compare", NULL);
	int64_t start = 0, end = array_size;
	while (start < end) {
		int64_t med = (start + end) / 2;
		int cmp = func_compare(search_value, sorted_array[med]);
		if (cmp > 0) {
			start = med + 1;
		} else if (cmp < 0) {
			end = med;
		} else {
			if (out_position != NULL)
				*out_position = med;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = start;
	return false;
}
//si se encuentra retorna la position donde esta [0, numElementos-1]
//si no se encuentra retorna false
bool my_seqsearch_intArr(int64_t search_value, int64_t *unsorted_array,
		int64_t array_size, int64_t *out_position) {
	for (int64_t i = 0; i < array_size; ++i) {
		if (my_compare_int(search_value, unsorted_array[i]) == 0) {
			if (out_position != NULL)
				*out_position = i;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = array_size;
	return false;
}
//si se encuentra retorna la position donde esta [0, numElementos-1]
//si no se encuentra retorna -1
bool my_seqsearch_doubleArr(double search_value, double *unsorted_array,
		int64_t array_size, int64_t *out_position) {
	for (int64_t i = 0; i < array_size; ++i) {
		if (my_compare_double(search_value, unsorted_array[i]) == 0) {
			if (out_position != NULL)
				*out_position = i;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = array_size;
	return false;
}
//si se encuentra retorna la position donde esta [0, numElementos-1]
//si no se encuentra retorna -1
bool my_seqsearch_objArr(const void *search_value, void **unsorted_array,
		int64_t array_size, my_func_compareObj func_compare,
		int64_t *out_position) {
	if (func_compare == NULL)
		my_assert_notNull("func_compare", NULL);
	for (int64_t i = 0; i < array_size; ++i) {
		if (func_compare(search_value, unsorted_array[i]) == 0) {
			if (out_position != NULL)
				*out_position = i;
			return true;
		}
	}
	if (out_position != NULL)
		*out_position = array_size;
	return false;
}
