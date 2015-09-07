/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_COLLECTION_UTIL_H
#define MY_COLLECTION_UTIL_H

#include "../myutils_c.h"

int my_compare_double(double da, double db);
int my_compare_int(int64_t da, int64_t db);

void my_qsort_double_array(double *array, int64_t length);
void my_qsort_double_arrayInv(double *array, int64_t length);
void my_qsort_int_array(int64_t *array, int64_t length);
void my_qsort_int_arrayInv(int64_t *array, int64_t length);
void my_qsort_uint8_array(uint8_t *array, int64_t length);

typedef int (*my_func_compareObj)(const void *search_object,
		const void *array_object);

bool my_binsearch_intArr(int64_t search_value, int64_t *sorted_array,
		int64_t array_size, int64_t *out_position);
bool my_binsearch_doubleArr(double search_value, double *sorted_array,
		int64_t array_size, int64_t *out_position);
/**
 * BINARY SEARCH.
 * if search_value is found, returns true and out_position is the position [0, array_size-1]
 * if search_value is not found, returns false and out_position is the position where it should be
 * @param search_value
 * @param sorted_array
 * @param array_size
 * @param func_compare
 * @param out_position current position or should be position
 * @return true/false
 */
bool my_binsearch_objArr(const void *search_value, void **sorted_array,
		int64_t array_size, my_func_compareObj func_compare,
		int64_t *out_position);

bool my_seqsearch_intArr(int64_t search_value, int64_t *unsorted_array,
		int64_t array_size, int64_t *out_position);
bool my_seqsearch_doubleArr(double search_value, double *unsorted_array,
		int64_t array_size, int64_t *out_position);
bool my_seqsearch_objArr(const void *search_value, void **unsorted_array,
		int64_t array_size, my_func_compareObj func_compare,
		int64_t *out_position);

MyVectorInt *my_vectorInt_new();
MyVectorInt *my_vectorInt_new_sorted(bool keepSorted, bool removeDuplicates);
MyVectorInt *my_vectorInt_new_wrapper(int64_t *array, int64_t length,
bool free_array_on_release);
int64_t my_vectorInt_get(MyVectorInt *al, int64_t position);
/**
 *
 * @param al
 * @param value
 * @return true if is was added, false if it was duplicated and the array fobidded it
 */
bool my_vectorInt_add(MyVectorInt *al, int64_t value);
/**
 *
 * @param al
 * @param values
 * @return true if any my_vectorInt_add was true
 */
bool my_vectorInt_addAll(MyVectorInt *al, MyVectorInt *values);
void my_vectorInt_remove(MyVectorInt *al, int64_t position);
int64_t my_vectorInt_size(MyVectorInt *al);
int64_t *my_vectorInt_array(MyVectorInt *al);
int64_t my_vectorInt_max(MyVectorInt *al);
void my_vectorInt_qsort(MyVectorInt *al);
/**
 * @param al
 * @param searchValue
 * @return if found returns the position (>=0), otherwise returns a number < 0
 */
int64_t my_vectorInt_indexOf(MyVectorInt *al, int64_t searchValue);
char *my_vectorInt_toString(MyVectorInt *al, char separator);
void my_vectorInt_release(MyVectorInt *al);
void my_vectorInt_releaseNoBuffer(MyVectorInt *al);

MyVectorDouble *my_vectorDouble_new();
MyVectorDouble *my_vectorDouble_new_sorted(bool keepSorted,
bool removeDuplicates);
double my_vectorDouble_get(MyVectorDouble *al, int64_t position);
bool my_vectorDouble_add(MyVectorDouble *al, double value);
int64_t my_vectorDouble_size(MyVectorDouble *al);
double *my_vectorDouble_array(MyVectorDouble *al);
void my_vectorDouble_qsort(MyVectorDouble *al);
char *my_vectorDouble_toString(MyVectorDouble *al, char separator);
void my_vectorDouble_release(MyVectorDouble *al);
void my_vectorDouble_releaseNoBuffer(MyVectorDouble *al);

int my_compare_filename(const void *object1, const void *object2);
int my_compare_string(const void *object1, const void *object2);
int my_compare_string_ignorecase(const void *objeto1, const void *objeto2);

MyVectorString *my_vectorString_new();
void my_vectorString_sortCaseSensitive(MyVectorString *al);
void my_vectorString_sortIgnoreCase(MyVectorString *al);
void my_vectorString_sortFilenames(MyVectorString *al);
void my_vectorString_keepSortedCaseSensitive(MyVectorString *al);
void my_vectorString_keepSortedIgnoreCase(MyVectorString *al);
void my_vectorString_keepSortedFilenames(MyVectorString *al);
char *my_vectorString_get(MyVectorString *al, int64_t position);
/**
 * returns true if it was added, false otherwise (the element already exists)
 * @param al
 * @param value
 * @return
 */
bool my_vectorString_add(MyVectorString *al, char* value);
/**
 *
 * @param al
 * @param value
 * @return
 */
bool my_vectorStringConst_add(MyVectorString *al, const char* value);
char *my_vectorString_remove(MyVectorString *al, int64_t position);
int64_t my_vectorString_removeDuplicates(MyVectorString *al);
int64_t my_vectorString_size(MyVectorString *al);
/**
 * add all values in @c values to @c al
 * @param al list which receives the values
 * @param values values to add
 */
void my_vectorString_addAll(MyVectorString *al, MyVectorString *values);
char **my_vectorString_getCurrentBuffer(MyVectorString *al);
/**
 * if found returns the position (>=0), otherwise returns a number < 0
 * @param al
 * @param search_object
 * @param func_compare
 * @return
 */
int64_t my_vectorString_indexOf(MyVectorString *al, const char* search_object,
		my_func_compareObj func_compare);
/**
 * array must be first sorted (either after a sort method or by keepsorted flag).
 * if found returns the position (>=0), otherwise returns a number < 0
 * @param al
 * @param search_object
 * @return
 */
int64_t my_vectorString_indexOf_binsearch(MyVectorString *al,
		const void* search_object);
char *my_vectorString_toString(MyVectorString *al, char separator);
void my_vectorString_release(MyVectorString *al, bool freeEachString);
char **my_vectorString_releaseReturnBuffer(MyVectorString *al);

MyVectorObj *my_vectorObj_new();
void my_vectorObj_setArrayOptions(MyVectorObj *al, bool keepSorted,
bool removeDuplicates, my_func_compareObj func_compare);
MyVectorObj *my_vectorObj_new_wrapper(int64_t largo, void **array,
bool free_array_on_release);
void* my_vectorObj_get(MyVectorObj *al, int64_t position);
/**
 * @param al
 * @param value
 * @return true if it was added o false otherwise (e.g. is duplicated and the options forbidded it)
 */
bool my_vectorObj_add(MyVectorObj *al, void* value);
void my_vectorObj_set(MyVectorObj *al, int64_t position, void* value);
void* my_vectorObj_remove(MyVectorObj *al, int64_t position);
int64_t my_vectorObj_size(MyVectorObj *al);
void* my_vectorObj_max(MyVectorObj *al);
/**
 * add all values in @c values to @c al
 * @param al list which receives the values
 * @param values values to add
 * @return if any my_vectorObj_add returned true
 */
bool my_vectorObj_addAll(MyVectorObj *al, MyVectorObj *values);
void **my_vectorObj_array(MyVectorObj *al);
void my_vectorObj_qsort(MyVectorObj *al,
		int (*qsort_compar)(const void*, const void*));
int64_t my_vectorObj_indexOf(MyVectorObj *al, const void* search_object);
int64_t my_vectorObj_indexOf_binsearch(MyVectorObj *al,
		const void* search_object, my_func_compareObj func_compare);
void my_vectorObj_release(MyVectorObj *al, bool freeEachObject);

typedef struct MyHeapObj MyHeapObj;

MyHeapObj *my_heapObj_new(int64_t maxElementos,
		my_func_compareObj func_compare);
void my_heapObj_add(MyHeapObj *al, void* value);
void *my_heapObj_remove(MyHeapObj *al);
int64_t my_heapObj_size(MyHeapObj *al);
void my_heapObj_release(MyHeapObj *al, bool freeEachObject);

typedef struct MyMapIntObj MyMapIntObj;

MyMapIntObj *my_mapIntObj_new_array();
MyMapIntObj *my_mapIntObj_new_ABB();
void *my_mapIntObj_add(MyMapIntObj *map, int64_t key, void *object);
void *my_mapIntObj_get(MyMapIntObj *map, int64_t key);
int64_t my_mapIntObj_size(MyMapIntObj *map);
MyVectorObj *my_mapIntObj_valuesVector(MyMapIntObj *map);
void my_mapIntObj_release(MyMapIntObj *map, bool freeEachObject);
int64_t my_mapIntObj_getKeyAt(MyMapIntObj *map, int64_t position);
void *my_mapIntObj_getObjAt(MyMapIntObj *map, int64_t position);

MyMapObjObj *my_mapObjObj_new(my_func_compareObj func_compare_keys);
void *my_mapObjObj_add(MyMapObjObj *map, void *key, void *value);
void *my_mapObjObj_get(MyMapObjObj *map, void *key);
MyVectorObj *my_mapObjObj_valuesVector(MyMapObjObj *map);
void *my_mapObjObj_remove(MyMapObjObj *map, void *key);
int64_t my_mapObjObj_size(MyMapObjObj *map);
void* my_mapObjObj_getKeyAt(MyMapObjObj *map, int64_t position);
void* my_mapObjObj_getObjAt(MyMapObjObj *map, int64_t position);
/**
 * add all pairs in @c keys , @c values to @c map
 * @param map
 * @param keys
 * @param values
 * @param size
 */
void my_mapObjObj_addAll(MyMapObjObj *map, void **keys, void **values,
		int64_t size);
void my_mapObjObj_release(MyMapObjObj *map, bool freeEachKey,
bool freeEachValue);

MyMapStringObj *my_mapStringObj_newCaseSensitive();
MyMapStringObj *my_mapStringObj_newIgnoreCase();
void *my_mapStringObj_add(MyMapStringObj *map, const char *key, void *value);
void *my_mapStringObj_get(MyMapStringObj *map, const char *key);
void *my_mapStringObj_remove(MyMapStringObj *map, void *key);
MyVectorObj *my_mapStringObj_valuesVector(MyMapStringObj *map);
int64_t my_mapStringObj_size(MyMapStringObj *map);
const char *my_mapStringObj_getKeyAt(MyMapStringObj *map, int64_t position);
void *my_mapStringObj_getObjAt(MyMapStringObj *map, int64_t position);
void my_mapStringObj_addAll(MyMapStringObj *map, const char **keys,
		void **values, int64_t size);
void my_mapStringObj_release(MyMapStringObj *map, bool freeEachKey,
bool freeEachValue);

#endif
