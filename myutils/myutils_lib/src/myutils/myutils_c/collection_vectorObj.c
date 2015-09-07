/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct MyVectorObj {
	void **array;
	int64_t length;
	int64_t buffer_size;
	bool keepSorted, removeDuplicates;
	my_func_compareObj func_compare;
	bool free_array_on_release;
};
MyVectorObj *my_vectorObj_new() {
	MyVectorObj *al = MY_MALLOC(1, MyVectorObj);
	al->free_array_on_release = true;
	return al;
}
void my_vectorObj_setArrayOptions(MyVectorObj *al, bool keepSorted,
bool removeDuplicates, my_func_compareObj func_compare) {
	al->keepSorted = keepSorted;
	al->removeDuplicates = removeDuplicates;
	al->func_compare = func_compare;
}
MyVectorObj *my_vectorObj_new_wrapper(int64_t length, void **array,
bool free_array_on_release) {
	MyVectorObj *al = MY_MALLOC(1, MyVectorObj);
	al->array = array;
	al->length = length;
	al->buffer_size = length;
	al->free_array_on_release = free_array_on_release;
	return al;
}
void* my_vectorObj_get(MyVectorObj *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	return al->array[position];
}

bool my_vectorObj_add(MyVectorObj *al, void* value) {
	int64_t pos = al->length;
	if (al->keepSorted || al->removeDuplicates) {
		bool exists;
		if (al->keepSorted) {
			exists = my_binsearch_objArr(value, al->array, al->length,
					al->func_compare, &pos);
		} else {
			exists = my_seqsearch_objArr(value, al->array, al->length,
					al->func_compare, &pos);
		}
		if (exists && al->removeDuplicates)
			return false;
	}
	if (al->length == al->buffer_size) {
		al->buffer_size = MAX(2 * al->buffer_size, 4);
		MY_REALLOC(al->array, al->buffer_size, void*);
	}
	for (int64_t i = al->length; i > pos; --i) {
		al->array[i] = al->array[i - 1];
	}
	al->array[pos] = value;
	al->length++;
	return true;
}
void my_vectorObj_set(MyVectorObj *al, int64_t position, void* value) {
	if (al->keepSorted || al->removeDuplicates)
		my_log_error("cannot set an object in this list\n");
	my_assert_indexRangeInt("array index", position, al->length);
	al->array[position] = value;
}
void* my_vectorObj_remove(MyVectorObj *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	void *obj = al->array[position];
	for (int64_t i = position + 1; i < al->length; ++i)
		al->array[i - 1] = al->array[i];
	al->length--;
	return obj;
}
int64_t my_vectorObj_size(MyVectorObj *al) {
	if (al == NULL)
		return 0;
	return al->length;
}
void* my_vectorObj_max(MyVectorObj *al) {
	my_assert_indexRangeInt("array index", 0, al->length);
	if (al->keepSorted)
		return al->array[al->length - 1];
	void *val_max = al->array[0];
	for (int64_t i = 1; i < al->length; ++i) {
		if (al->func_compare(al->array[i], val_max) > 0)
			val_max = al->array[i];
	}
	return val_max;
}
bool my_vectorObj_addAll(MyVectorObj *al, MyVectorObj *valores) {
	if (valores == NULL || al == NULL)
		return false;
	bool ok = false;
	for (int64_t i = 0; i < valores->length; ++i)
		ok |= my_vectorObj_add(al, valores->array[i]);
	return ok;
}
void **my_vectorObj_array(MyVectorObj *al) {
	if (al == NULL)
		return NULL;
	return al->array;
}
void my_vectorObj_qsort(MyVectorObj *al,
		int (*qsort_compar)(const void*, const void*)) {
	if (al->length > 1)
		qsort(al->array, al->length, sizeof(void*), qsort_compar);
}
//if found returns the position (>=0), otherwise returns a number < 0
int64_t my_vectorObj_indexOf(MyVectorObj *al, const void* search_object) {
	int64_t pos = -1;
	bool exists;
	if (al->keepSorted)
		exists = my_binsearch_objArr(search_object, al->array, al->length,
				al->func_compare, &pos);
	else
		exists = my_seqsearch_objArr(search_object, al->array, al->length,
				al->func_compare, &pos);
	if (exists)
		return pos;
	return -1;
}
//array must be first sorted by func_compare
//if found returns the position (>=0), otherwise returns a number < 0
int64_t my_vectorObj_indexOf_binsearch(MyVectorObj *al,
		const void* search_object, my_func_compareObj func_compare) {
	int64_t pos = -1;
	bool exists = my_binsearch_objArr(search_object, al->array, al->length,
			func_compare, &pos);
	return exists ? pos : -1;
}

void my_vectorObj_release(MyVectorObj *al, bool freeEachObject) {
	if (al == NULL)
		return;
	if (freeEachObject) {
		for (int64_t i = 0; i < al->length; ++i)
			MY_FREE(al->array[i]);
	}
	if (al->free_array_on_release)
		MY_FREE(al->array);
	MY_FREE(al);
}
