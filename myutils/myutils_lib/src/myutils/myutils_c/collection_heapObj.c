/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "collection_util.h"

struct MyHeapObj {
	void **array;
	int64_t length, maxElementos;
	my_func_compareObj func_compare;
};
MyHeapObj *my_heapObj_new(int64_t maxElementos, my_func_compareObj func_compare) {
	MyHeapObj *al = MY_MALLOC(1, MyHeapObj);
	al->array = MY_MALLOC(maxElementos, void*);
	al->maxElementos = maxElementos;
	al->func_compare = func_compare;
	return al;
}

#define SWAP(var1, var2, swap_var) (swap_var)=(var1);(var1)=(var2);(var2)=(swap_var)

void my_heapObj_add(MyHeapObj *al, void* value) {
	if (al->length == al->maxElementos)
		my_log_error("largo maximo del heap %"PRIi64" sobrepasado\n",
				al->maxElementos);
	al->array[al->length] = value;
	void *aux;
	int64_t j = al->length, padre;
	al->length++;
	while (j > 0) {
		padre = (j - 1) / 2;
		if (al->func_compare(al->array[padre], al->array[j]) >= 0)
			break;
		SWAP(al->array[j], al->array[padre], aux);
		j = padre;
	}
}
void *my_heapObj_remove(MyHeapObj *al) {
	my_assert_indexRangeInt("array index", 0, al->length);
	void *val_max = al->array[0];
	al->length--;
	al->array[0] = al->array[al->length];
	void *aux;
	int64_t j = 0, izq, der, mayor;
	for (;;) {
		izq = (2 * j) + 1;
		der = izq + 1;
		mayor = j;
		if (izq < al->length
				&& al->func_compare(al->array[izq], al->array[j]) > 0)
			mayor = izq;
		if (der < al->length
				&& al->func_compare(al->array[der], al->array[mayor]) > 0)
			mayor = der;
		if (mayor == j)
			break;
		SWAP(al->array[j], al->array[mayor], aux);
		j = mayor;
	}
	return val_max;
}
int64_t my_heapObj_size(MyHeapObj *al) {
	return al->length;
}
void my_heapObj_release(MyHeapObj *al, bool freeEachObject) {
	if (freeEachObject) {
		for (int64_t i = 0; i < al->length; ++i)
			MY_FREE(al->array[i]);
	}
	MY_FREE(al->array);
	MY_FREE(al);
}
