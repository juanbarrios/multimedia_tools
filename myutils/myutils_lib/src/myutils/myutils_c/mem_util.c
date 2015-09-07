/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "mem_util.h"

void* my_memory_alloc(int64_t num_objs, size_t size_objs, bool initWithZeros) {
	int64_t sizes = (int64_t) size_objs;
	int64_t mytot = num_objs * sizes;
	if (mytot < 0) {
		double mb = mytot / (1024.0 * 1024.0);
		my_log_error(
				"internal error. %"PRIi64" * %"PRIi64"= %1.1lf MB requested\n",
				num_objs, sizes, mb);
	} else if (mytot == 0) {
		return NULL;
	}
	void *ptr = initWithZeros ? calloc(num_objs, sizes) : malloc(mytot);
	//memset(ptr,0,mytot);
	if (ptr == NULL) {
		double mb = mytot / (1024.0 * 1024.0);
		my_log_error(
				"\n*** out of memory ***. %"PRIi64" * %"PRIi64"= %1.1lf MB requested\n",
				num_objs, sizes, mb);
	}
	return ptr;
}
void* my_memory_realloc(void *ptr, int64_t num_objs, size_t size_objs) {
	int64_t sizes = (int64_t) size_objs;
	int64_t mytot = num_objs * sizes;
	if (mytot <= 0) {
		double mb = mytot / (1024.0 * 1024.0);
		my_log_error(
				"internal error. %"PRIi64" * %"PRIi64"= %1.1lf MB requested\n",
				num_objs, sizes, mb);
	}
	void* ptr2 = realloc(ptr, mytot);
	if (ptr2 == NULL) {
		double mb = mytot / (1024.0 * 1024.0);
		my_log_error(
				"\n*** out of memory ***. %"PRIi64" * %"PRIi64"= %1.1lf MB requested\n",
				num_objs, sizes, mb);
	}
	return ptr2;
}

void** my_memory_alloc_matrix(int64_t dim1, int64_t dim2, size_t size_objs) {
	int64_t sizes = (int64_t) size_objs;
	int64_t total = dim1 * dim2 * sizes;
	if (total <= 0) {
		double mb = total / (1024.0 * 1024.0);
		my_log_error(
				"internal error. %"PRIi64" x %"PRIi64" * %"PRIi64"=%1.1lf MB requested\n",
				dim1, dim2, sizes, mb);
	}
	int64_t total1 = dim1 * sizeof(void*);
	void** ptr = malloc(total1);
	if (ptr == NULL) {
		double mb = total / (1024.0 * 1024.0);
		my_log_error(
				"\n*** out of memory ***. %"PRIi64" x %"PRIi64" * %"PRIi64"=%1.1lf MB requested\n",
				dim1, dim2, sizes, mb);
	}
	for (int64_t i = 0; i < dim1; ++i) {
		ptr[i] = calloc(dim2, sizes);
		if (ptr[i] == NULL) {
			double mb = total / (1024.0 * 1024.0);
			my_log_error(
					"\n*** out of memory ***. %"PRIi64" x %"PRIi64" * %"PRIi64"=%1.1lf MB requested\n",
					dim1, dim2, sizes, mb);
		}
	}
	return ptr;
}

void my_memory_free(void *ptr) {
	if (ptr != NULL)
		free(ptr);
}

void* my_header_memory_free = "my_header_memory_free_varargs";

void my_memory_free_varargs(void *header, ...) {
	my_assert_equalPtr("my_header_memory_free_varargs", header,
			my_header_memory_free);
	va_list arguments;
	va_start(arguments, header);
	for (size_t cont = 0;; cont++) {
		void *ptr = va_arg(arguments, void*);
		if (ptr == my_header_memory_free)
			break;
		if (ptr != NULL)
			free(ptr);
	}
	va_end(arguments);
}
void my_memory_free_matrix(void** ptr_array, size_t dim1) {
	if (ptr_array == NULL)
		return;
	for (size_t i = 0; i < dim1; ++i) {
		if (ptr_array[i] != NULL)
			free(ptr_array[i]);
	}
	free(ptr_array);
}

double **my_memory_newMatrixDouble(int64_t width, int64_t height,
		double init_val) {
	double **matriz = MY_MALLOC_NOINIT(width, double*);
	for (int64_t x = 0; x < width; ++x) {
		if (init_val == 0)
			matriz[x] = MY_MALLOC(height, double);
		else {
			matriz[x] = MY_MALLOC_NOINIT(height, double);
			for (int64_t y = 0; y < height; ++y)
				matriz[x][y] = init_val;
		}
	}
	return matriz;
}

uint8_t **my_memory_bytes2bitMatrix(char *array1, int64_t width, int64_t height) {
	uint8_t **img1 = MY_MALLOC_NOINIT(width, uint8_t*);
	int64_t contByte = 0, contBit = 0, i = 0, j = 0;
	while (j < height) {
		if (j == 0)
			img1[i] = MY_MALLOC_NOINIT(height, uint8_t);
		img1[i][j] = (uint8_t) ((array1[(contByte)] >> (contBit)) & 1);
		contBit++;
		if (contBit == 8) {
			contBit = 0;
			contByte++;
		}
		i++;
		if (i >= width) {
			i = 0;
			j++;
		}
	}
	return img1;
}

