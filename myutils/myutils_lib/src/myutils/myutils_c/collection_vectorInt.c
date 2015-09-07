/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct MyVectorInt {
	int64_t *array;
	int64_t length;
	int64_t buffer_size;
	bool keepSorted, removeDuplicates;
	bool free_array_on_release;
};
MyVectorInt *my_vectorInt_new() {
	return my_vectorInt_new_sorted(false, false);
}
MyVectorInt *my_vectorInt_new_sorted(bool keepSorted, bool removeDuplicates) {
	MyVectorInt *al = MY_MALLOC(1, MyVectorInt);
	al->keepSorted = keepSorted;
	al->removeDuplicates = removeDuplicates;
	al->free_array_on_release = true;
	return al;
}
MyVectorInt *my_vectorInt_new_wrapper(int64_t *array, int64_t length,
bool free_array_on_release) {
	MyVectorInt *al = MY_MALLOC(1, MyVectorInt);
	al->array = array;
	al->length = length;
	al->buffer_size = length;
	al->free_array_on_release = free_array_on_release;
	return al;
}
int64_t my_vectorInt_get(MyVectorInt *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	return al->array[position];
}
bool my_vectorInt_add(MyVectorInt *al, int64_t value) {
	int64_t pos = al->length;
	if (al->keepSorted || al->removeDuplicates) {
		bool exists;
		if (al->keepSorted) {
			exists = my_binsearch_intArr(value, al->array, al->length, &pos);
		} else {
			exists = my_seqsearch_intArr(value, al->array, al->length, &pos);
		}
		if (exists && al->removeDuplicates)
			return false;
	}
	if (al->length == al->buffer_size) {
		al->buffer_size = MAX(2 * al->buffer_size, 4);
		MY_REALLOC(al->array, al->buffer_size, int64_t);
	}
	for (int64_t i = al->length; i > pos; --i) {
		al->array[i] = al->array[i - 1];
	}
	al->array[pos] = value;
	al->length++;
	return true;
}
bool my_vectorInt_addAll(MyVectorInt *al, MyVectorInt *values) {
	if (values == NULL || al == NULL)
		return false;
	bool ok = false;
	for (int64_t i = 0; i < values->length; ++i)
		ok |= my_vectorInt_add(al, values->array[i]);
	return ok;
}
void my_vectorInt_remove(MyVectorInt *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	for (int64_t i = position + 1; i < al->length; ++i)
		al->array[i - 1] = al->array[i];
	al->length--;
}
int64_t my_vectorInt_size(MyVectorInt *al) {
	if (al == NULL)
		return 0;
	return al->length;
}
int64_t *my_vectorInt_array(MyVectorInt *al) {
	if (al == NULL)
		return NULL;
	return al->array;
}

int64_t my_vectorInt_max(MyVectorInt *al) {
	my_assert_indexRangeInt("array index", 0, al->length);
	if (al->keepSorted)
		return al->length - 1;
	int64_t val_max = al->array[0];
	for (int64_t i = 1; i < al->length; ++i) {
		if (al->array[i] > val_max)
			val_max = al->array[i];
	}
	return val_max;
}
void my_vectorInt_qsort(MyVectorInt *al) {
	my_qsort_int_array(al->array, al->length);
}

int64_t my_vectorInt_indexOf(MyVectorInt *al, int64_t searchValue) {
	bool exists = false;
	int64_t pos = -1;
	if (al->keepSorted)
		exists = my_binsearch_intArr(searchValue, al->array, al->length, &pos);
	else
		exists = my_seqsearch_intArr(searchValue, al->array, al->length, &pos);
	if (exists)
		return pos;
	return -1;
}
char *my_vectorInt_toString(MyVectorInt *al, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < my_vectorInt_size(al); ++i) {
		int64_t n = my_vectorInt_get(al, i);
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendInt(sb, n);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
void my_vectorInt_release(MyVectorInt *al) {
	if (al == NULL)
		return;
	if (al->free_array_on_release && al->array != NULL)
		free(al->array);
	free(al);
}
void my_vectorInt_releaseNoBuffer(MyVectorInt *al) {
	if (al == NULL)
		return;
	MY_FREE(al);
}
