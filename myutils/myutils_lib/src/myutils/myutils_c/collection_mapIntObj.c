/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct Node {
	int64_t key;
	void *object;
	struct Node *left, *right;
};
struct MyMapIntObj {
	bool is_array, is_ABB;
	int64_t size;
//array
	int64_t *array_keys;
	void **array_objects;
	int64_t buffer_size;
//ABB i2o
	struct Node *abb_root;
};

MyMapIntObj *my_mapIntObj_new_array() {
	MyMapIntObj *map = MY_MALLOC(1, MyMapIntObj);
	map->is_array = true;
	return map;
}
MyMapIntObj *my_mapIntObj_new_ABB() {
	MyMapIntObj *map = MY_MALLOC(1, MyMapIntObj);
	map->is_ABB = true;
	return map;
}
static struct Node *newNode(int64_t key, void *object) {
	struct Node *n = MY_MALLOC(1, struct Node);
	n->key = key;
	n->object = object;
	return n;
}
//retorna el que objeto que estaba o NULL si no existe la llave
void *my_mapIntObj_add(MyMapIntObj *map, int64_t key, void *object) {
	if (map->is_array) {
		int64_t pos = -1;
		if (my_binsearch_intArr(key, map->array_keys, map->size, &pos)) {
			void *old = map->array_objects[pos];
			map->array_objects[pos] = object;
			return old;
		}
		if (map->size == map->buffer_size) {
			map->buffer_size = MAX(2 * map->buffer_size, 4);
			MY_REALLOC(map->array_keys, map->buffer_size, int64_t);
			MY_REALLOC(map->array_objects, map->buffer_size, void*);
		}
		for (int64_t i = map->size; i > pos; --i) {
			map->array_keys[i] = map->array_keys[i - 1];
			map->array_objects[i] = map->array_objects[i - 1];
		}
		map->array_keys[pos] = key;
		map->array_objects[pos] = object;
		map->size++;
		return NULL;
	} else if (map->is_ABB) {
		if (map->abb_root == NULL) {
			map->abb_root = newNode(key, object);
			map->size++;
			return NULL;
		}
		struct Node *nodo = map->abb_root;
		for (;;) {
			if (key < nodo->key) {
				if (nodo->left == NULL) {
					nodo->left = newNode(key, object);
					map->size++;
					return NULL;
				}
				nodo = nodo->left;
			} else if (key > nodo->key) {
				if (nodo->right == NULL) {
					nodo->right = newNode(key, object);
					map->size++;
					return NULL;
				}
				nodo = nodo->right;
			} else {
				void *old = nodo->object;
				nodo->object = object;
				return old;
			}
		}
		return NULL;
	}
	my_log_error("todo addObj_mapInt\n");
	return NULL;
}
//retorna NULL si no esta la llave
void *my_mapIntObj_get(MyMapIntObj *map, int64_t key) {
	if (map->is_array) {
		int64_t pos = -1;
		if (my_binsearch_intArr(key, map->array_keys, map->size, &pos))
			return map->array_objects[pos];
		return NULL;
	} else if (map->is_ABB) {
		struct Node *nodo = map->abb_root;
		while (nodo != NULL) {
			if (key < nodo->key) {
				nodo = nodo->left;
			} else if (key > nodo->key) {
				nodo = nodo->right;
			} else {
				return nodo->object;
			}
		}
	}
	return NULL;
}
int64_t my_mapIntObj_size(MyMapIntObj *map) {
	return map->size;
}
MyVectorObj *my_mapIntObj_valuesVector(MyMapIntObj *map) {
	return my_vectorObj_new_wrapper(map->size, map->array_objects, false);
}
int64_t my_mapIntObj_getKeyAt(MyMapIntObj *map, int64_t position) {
	if (map->is_array) {
		my_assert_indexRangeInt("array index", position, map->size);
		return map->array_keys[position];
	}
	my_log_error("todo\n");
	return -1;
}
void *my_mapIntObj_getObjAt(MyMapIntObj *map, int64_t position) {
	if (map->is_array) {
		my_assert_indexRangeInt("array index", position, map->size);
		return map->array_objects[position];
	}
	my_log_error("todo\n");
	return NULL;
}
static void my_map_releaseABB_i2o_rec(struct Node *nodo,
bool freeEachObject) {
	if (nodo == NULL)
		return;
	my_map_releaseABB_i2o_rec(nodo->left, freeEachObject);
	my_map_releaseABB_i2o_rec(nodo->right, freeEachObject);
	if (freeEachObject)
		MY_FREE(nodo->object);
	MY_FREE(nodo);
}
void my_mapIntObj_release(MyMapIntObj *map, bool freeEachObject) {
	if (map->is_array) {
		if (freeEachObject)
			for (int64_t i = 0; i < map->size; ++i)
				MY_FREE(map->array_objects[i]);
		MY_FREE(map->array_keys);
		MY_FREE(map->array_objects);
	} else if (map->is_ABB) {
		my_map_releaseABB_i2o_rec(map->abb_root, freeEachObject);
	}
	MY_FREE(map);
}
