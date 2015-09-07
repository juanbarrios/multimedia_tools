/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct MyVectorDouble {
	double *array;
	int64_t length;
	int64_t buffer_size;
	bool keepSorted, removeDuplicates;
};
MyVectorDouble *my_vectorDouble_new() {
	return my_vectorDouble_new_sorted(false, false);
}
MyVectorDouble *my_vectorDouble_new_sorted(bool keepSorted,
bool removeDuplicates) {
	MyVectorDouble *al = MY_MALLOC(1, MyVectorDouble);
	al->keepSorted = keepSorted;
	al->removeDuplicates = removeDuplicates;
	return al;
}
double my_vectorDouble_get(MyVectorDouble *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	return al->array[position];
}
bool my_vectorDouble_add(MyVectorDouble *al, double value) {
	int64_t pos = al->length;
	if (al->keepSorted || al->removeDuplicates) {
		bool exists;
		if (al->keepSorted) {
			exists = my_binsearch_doubleArr(value, al->array, al->length, &pos);
		} else {
			exists = my_seqsearch_doubleArr(value, al->array, al->length, &pos);
		}
		if (exists && al->removeDuplicates)
			return false;
	}
	if (al->length == al->buffer_size) {
		al->buffer_size = MAX(2 * al->buffer_size, 4);
		MY_REALLOC(al->array, al->buffer_size, double);
	}
	for (int64_t i = al->length; i > pos; --i) {
		al->array[i] = al->array[i - 1];
	}
	al->array[pos] = value;
	al->length++;
	return true;
}
int64_t my_vectorDouble_size(MyVectorDouble *al) {
	if (al == NULL)
		return 0;
	return al->length;
}
double *my_vectorDouble_array(MyVectorDouble *al) {
	if (al == NULL)
		return NULL;
	return al->array;
}
void my_vectorDouble_qsort(MyVectorDouble *al) {
	my_qsort_double_array(al->array, al->length);
}
char *my_vectorDouble_toString(MyVectorDouble *al, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < my_vectorDouble_size(al); ++i) {
		double n = my_vectorDouble_get(al, i);
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendDouble(sb, n);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}

void my_vectorDouble_release(MyVectorDouble *al) {
	if (al == NULL)
		return;
	MY_FREE(al->array);
	MY_FREE(al);
}
void my_vectorDouble_releaseNoBuffer(MyVectorDouble *al) {
	if (al == NULL)
		return;
	MY_FREE(al);
}
