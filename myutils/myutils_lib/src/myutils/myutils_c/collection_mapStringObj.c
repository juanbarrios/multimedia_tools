/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct MyMapStringObj {
	my_func_compareObj func_compare_keys;
	int64_t size;
	int64_t buffer_size;
	const char**array_keys;
	void **array_objects;
};

MyMapStringObj *my_mapStringObj_newCaseSensitive() {
	MyMapStringObj *map = MY_MALLOC(1, MyMapStringObj);
	map->func_compare_keys = my_compare_string;
	return map;
}
MyMapStringObj *my_mapStringObj_newIgnoreCase() {
	MyMapStringObj *map = MY_MALLOC(1, MyMapStringObj);
	map->func_compare_keys = my_compare_string_ignorecase;
	return map;
}
//retorna el que objeto que estaba o NULL si no existe la llave
void *my_mapStringObj_add(MyMapStringObj *map, const char *key, void *value) {
	int64_t pos = -1;
	if (my_binsearch_objArr((void*) key, (void**) map->array_keys, map->size,
			map->func_compare_keys, &pos)) {
		void *old = map->array_objects[pos];
		map->array_objects[pos] = value;
		return old;
	}
	if (map->size == map->buffer_size) {
		map->buffer_size = MAX(2 * map->buffer_size, 4);
		MY_REALLOC(map->array_keys, map->buffer_size, const char*);
		MY_REALLOC(map->array_objects, map->buffer_size, void*);
	}
	for (int64_t i = map->size; i > pos; --i) {
		map->array_keys[i] = map->array_keys[i - 1];
		map->array_objects[i] = map->array_objects[i - 1];
	}
	map->array_keys[pos] = key;
	map->array_objects[pos] = value;
	map->size++;
	return NULL;
}

//retorna NULL si no esta la llave
void *my_mapStringObj_get(MyMapStringObj *map, const char *key) {
	if (map == NULL)
		return NULL;
	int64_t pos = 0;
	if (my_binsearch_objArr((void*) key, (void**) map->array_keys, map->size,
			map->func_compare_keys, &pos))
		return map->array_objects[pos];
	return NULL;
}
void *my_mapStringObj_remove(MyMapStringObj *map, void *key) {
	int64_t pos = -1;
	if (!my_binsearch_objArr(key, (void**) map->array_keys, map->size,
			map->func_compare_keys, &pos))
		return NULL;
	void *old = map->array_objects[pos];
	for (int64_t k = pos + 1; k < map->size; ++k) {
		map->array_keys[k - 1] = map->array_keys[k];
		map->array_objects[k - 1] = map->array_objects[k];
	}
	map->size--;
	return old;
}
MyVectorObj *my_mapStringObj_valuesVector(MyMapStringObj *map) {
	return my_vectorObj_new_wrapper(map->size, map->array_objects, false);
}

int64_t my_mapStringObj_size(MyMapStringObj *map) {
	if (map == NULL)
		return 0;
	return map->size;
}
const char *my_mapStringObj_getKeyAt(MyMapStringObj *map, int64_t position) {
	my_assert_indexRangeInt("array index", position, map->size);
	return map->array_keys[position];
}
void *my_mapStringObj_getObjAt(MyMapStringObj *map, int64_t position) {
	my_assert_indexRangeInt("array index", position, map->size);
	return map->array_objects[position];
}
void my_mapStringObj_addAll(MyMapStringObj *map, const char **keys,
		void **values, int64_t size) {
	for (int64_t i = 0; i < size; ++i)
		my_mapStringObj_add(map, keys[i], values[i]);
}
void my_mapStringObj_release(MyMapStringObj *map, bool freeEachKey,
bool freeEachValue) {
	if (freeEachKey)
		for (int64_t i = 0; i < map->size; ++i)
			MY_FREE(map->array_keys[i]);
	if (freeEachValue)
		for (int64_t i = 0; i < map->size; ++i)
			MY_FREE(map->array_objects[i]);
	MY_FREE(map->array_keys);
	MY_FREE(map->array_objects);
	MY_FREE(map);
}
