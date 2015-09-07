/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_MEM_UTIL_H
#define MY_MEM_UTIL_H

#include "../myutils_c.h"

#define MY_MALLOC(num, type) ((type*) my_memory_alloc((num), sizeof(type), true))
#define MY_MALLOC_NOINIT(num, type) ((type*) my_memory_alloc((num), sizeof(type), false))
#define MY_REALLOC(ptr, num, type) ((ptr)=(type *) my_memory_realloc((ptr),(num),sizeof(type)))
#define MY_MALLOC_MATRIX(dim1, dim2, type) ((type**) my_memory_alloc_matrix((dim1),(dim2),sizeof(type)))

void* my_memory_alloc(int64_t num_objs, size_t size_objs, bool initWithZeros);
void* my_memory_realloc(void *ptr, int64_t num_objs, size_t size_objs);
void** my_memory_alloc_matrix(int64_t dim1, int64_t dim2, size_t size_objs);

#define MY_SETZERO(ptr_array, array_length, array_type) (memset((array_type *)(ptr_array), 0, (array_length) * sizeof(array_type)))

#define MY_FREE(ptr)( my_memory_free( (void*)(ptr) ) )
#define MY_FREE_MATRIX(ptr_array, dim1)(my_memory_free_matrix((void**)(ptr_array),(dim1)))

void my_memory_free(void *ptr);

extern void* my_header_memory_free;
#define MY_FREE_MULTI(ptr1,...) (my_memory_free_varargs(my_header_memory_free, (void*) ptr1, __VA_ARGS__ , my_header_memory_free))

void my_memory_free_varargs(void *header, ...);
void my_memory_free_matrix(void** ptr_array, size_t dim1);

double **my_memory_newMatrixDouble(int64_t width, int64_t height,
		double init_val);
uint8_t **my_memory_bytes2bitMatrix(char *array1, int64_t width, int64_t height);

#endif
