/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

int my_compare_filename(const void *objeto1, const void *objeto2) {
	const char *f1 = (const char*) objeto1;
	const char *f2 = (const char*) objeto2;
	return my_string_strcmpFilenames(f1, f2);
}
int my_compare_string(const void *objeto1, const void *objeto2) {
	const char *f1 = (const char*) objeto1;
	const char *f2 = (const char*) objeto2;
	return strcmp(f1, f2);
}
int my_compare_string_ignorecase(const void *objeto1, const void *objeto2) {
	const char *f1 = (const char*) objeto1;
	const char *f2 = (const char*) objeto2;
	return strcasecmp(f1, f2);
}

struct MyVectorString {
	char **array;
	int64_t length;
	int64_t buffer_size;
	//sort all objects
	bool keepSorted;
	my_func_compareObj func_compare;
	//was recently sorted
	bool is_sorted_data;
	my_func_compareObj func_last_sort;
};
MyVectorString *my_vectorString_new() {
	MyVectorString *al = MY_MALLOC(1, MyVectorString);
	return al;
}

static int func_qsort_strcmp(const void *a, const void *b) {
	return strcmp(*(char**) a, *(char**) b);
}
void my_vectorString_sortCaseSensitive(MyVectorString *al) {
	if (al->length > 1)
		qsort(al->array, al->length, sizeof(char*), func_qsort_strcmp);
	al->is_sorted_data = true;
	al->func_last_sort = my_compare_string;
}

static int func_qsort_strcasecmp(const void *a, const void *b) {
	return strcasecmp(*(char**) a, *(char**) b);
}
void my_vectorString_sortIgnoreCase(MyVectorString *al) {
	if (al->length > 1)
		qsort(al->array, al->length, sizeof(char*), func_qsort_strcasecmp);
	al->is_sorted_data = true;
	al->func_last_sort = my_compare_string_ignorecase;
}

static int func_qsort_filenames(const void *a, const void *b) {
	return my_string_strcmpFilenames(*(char**) a, *(char**) b);
}
void my_vectorString_sortFilenames(MyVectorString *al) {
	if (al->length > 1)
		qsort(al->array, al->length, sizeof(char*), func_qsort_filenames);
	al->is_sorted_data = true;
	al->func_last_sort = my_compare_filename;
}

void my_vectorString_keepSortedCaseSensitive(MyVectorString *al) {
	if (al->length > 0 && !al->is_sorted_data)
		my_vectorString_sortCaseSensitive(al);
	al->keepSorted = true;
	al->func_compare = my_compare_string;
}
void my_vectorString_keepSortedIgnoreCase(MyVectorString *al) {
	if (al->length > 0 && !al->is_sorted_data)
		my_vectorString_sortIgnoreCase(al);
	al->keepSorted = true;
	al->func_compare = my_compare_string_ignorecase;
}
void my_vectorString_keepSortedFilenames(MyVectorString *al) {
	if (al->length > 0 && !al->is_sorted_data)
		my_vectorString_sortFilenames(al);
	al->keepSorted = true;
	al->func_compare = my_compare_filename;
}

char *my_vectorString_get(MyVectorString *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	return al->array[position];
}

bool my_vectorString_add(MyVectorString *al, char* value) {
	int64_t pos = al->length;
	if (al->keepSorted) {
		if (my_binsearch_objArr(value, (void**) al->array, al->length,
				al->func_compare, &pos))
			return false;
	} else {
		al->is_sorted_data = false;
		al->func_last_sort = NULL;
	}
	if (al->length == al->buffer_size) {
		al->buffer_size = MAX(2 * al->buffer_size, 4);
		MY_REALLOC(al->array, al->buffer_size, char*);
	}
	for (int64_t i = al->length; i > pos; --i) {
		al->array[i] = al->array[i - 1];
	}
	al->array[pos] = value;
	al->length++;
	return true;
}
bool my_vectorStringConst_add(MyVectorString *al, const char* value) {
	return my_vectorString_add(al, (char*) value);
}
char *my_vectorString_remove(MyVectorString *al, int64_t position) {
	my_assert_indexRangeInt("array index", position, al->length);
	char *obj = al->array[position];
	for (int64_t i = position + 1; i < al->length; ++i)
		al->array[i - 1] = al->array[i];
	al->length--;
	return obj;
}
int64_t my_vectorString_removeDuplicates(MyVectorString *al) {
	if (!al->is_sorted_data)
		my_log_error(
				"MyVectorString must be first sorted before removing duplicates\n");
	int64_t cont = 0;
	int64_t pos = al->length - 1;
	while (pos > 0) {
		char *first = al->array[pos - 1];
		char *second = al->array[pos];
		int c = al->func_last_sort(first, second);
		if (c == 0) {
			my_vectorString_remove(al, pos);
			cont++;
		} else if (c > 0) {
			my_log_error("data is not sorted! (%s > %s)", first, second);
		}
		pos--;
	}
	return cont;
}

int64_t my_vectorString_size(MyVectorString *al) {
	if (al == NULL)
		return 0;
	return al->length;
}
void my_vectorString_addAll(MyVectorString *al, MyVectorString *values) {
	if (values == NULL)
		return;
	for (int64_t i = 0; i < values->length; ++i)
		my_vectorString_add(al, values->array[i]);
}
char **my_vectorString_getCurrentBuffer(MyVectorString *al) {
	return al->array;
}

int64_t my_vectorString_indexOf(MyVectorString *al, const char* search_object,
		my_func_compareObj func_compare) {
	int64_t pos = -1;
	if (my_seqsearch_objArr(search_object, (void**) al->array, al->length,
			func_compare, &pos))
		return pos;
	return -1;
}

int64_t my_vectorString_indexOf_binsearch(MyVectorString *al,
		const void* search_object) {
	if (!al->keepSorted && !al->is_sorted_data)
		my_log_error(
				"MyVectorString must be first sorted before searching data\n");
	int64_t pos = -1;
	bool exists = my_binsearch_objArr(search_object, (void**) al->array,
			al->length,
			(al->keepSorted) ? al->func_compare : al->func_last_sort, &pos);
	return (exists ? pos : -1);
}
char *my_vectorString_toString(MyVectorString *al, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < my_vectorString_size(al); ++i) {
		char *st = my_vectorString_get(al, i);
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendString(sb, st);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
void my_vectorString_release(MyVectorString *al, bool freeEachString) {
	if (al == NULL)
		return;
	if (freeEachString) {
		for (int64_t i = 0; i < al->length; ++i)
			MY_FREE(al->array[i]);
	}
	MY_FREE(al->array);
	MY_FREE(al);
}
char **my_vectorString_releaseReturnBuffer(MyVectorString *al) {
	if (al == NULL)
		return NULL;
	char **arr = al->array;
	MY_FREE(al);
	return arr;
}
