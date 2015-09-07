/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "random_util.hpp"
#include "../myutils_c.h"

using namespace my;

void random::setSeed(long long seed) {
	return my_random_setSeed(seed);
}

long long random::intValue(long long minValueIncluded,
		long long maxValueNotIncluded) {
	return my_random_int(minValueIncluded, maxValueNotIncluded);
}

std::vector<long long> intList(long long minValueIncluded,
		long long maxValueNotIncluded, size_t size) {
	int64_t *array = MY_MALLOC(size, int64_t);
	my_random_intList(minValueIncluded, maxValueNotIncluded, array, size);
	std::vector<long long> val(size);
	for (size_t i = 0; i < size; ++i)
		val[i] = array[i];
	free(array);
	return val;
}

double random::doubleValue(double minValueIncluded,
		double maxValueNotIncluded) {
	return my_random_double(minValueIncluded, maxValueNotIncluded);
}

std::vector<double> random::doubleList(double minValueIncluded,
		double maxValueNotIncluded, size_t size) {
	double *array = MY_MALLOC(size, double);
	my_random_doubleList(minValueIncluded, maxValueNotIncluded, array, size);
	std::vector<double> val(size);
	for (size_t i = 0; i < size; ++i)
		val[i] = array[i];
	free(array);
	return val;
}

std::vector<long long> random::permutation(long long minValueIncluded,
		long long maxValueNotIncluded) {
	long long size = maxValueNotIncluded - minValueIncluded;
	int64_t *array = my_random_newPermutation(minValueIncluded,
			maxValueNotIncluded);
	std::vector<long long> val(size);
	for (long long i = 0; i < size; ++i)
		val[i] = array[i];
	free(array);
	return val;
}

std::vector<long long> random::intList_noRepetitions(long long minValueIncluded,
		long long maxValueNotIncluded, size_t size) {
	int64_t *array = MY_MALLOC(size, int64_t);
	my_random_intList_noRepetitions(minValueIncluded, maxValueNotIncluded,
			array, size);
	std::vector<long long> val(size);
	for (size_t i = 0; i < size; ++i)
		val[i] = array[i];
	free(array);
	return val;
}
