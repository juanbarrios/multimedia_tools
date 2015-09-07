/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_RANDOM_UTIL_H
#define MY_RANDOM_UTIL_H

#include "../myutils_c.h"

void my_random_setSeed(int64_t seed);

int64_t my_random_int(int64_t minValueIncluded, int64_t maxValueNotIncluded);

void my_random_intList(int64_t minValueIncluded, int64_t maxValueNotIncluded,
		int64_t *array, int64_t size);

double my_random_double(double minValueIncluded, double maxValueNotIncluded);

void my_random_doubleList(double minValueIncluded, double maxValueNotIncluded,
		double *array, int64_t size);

int64_t* my_random_newPermutation(int64_t minValueIncluded,
		int64_t maxValueNotIncluded);

void my_random_intList_noRepetitions(int64_t minValueIncluded,
		int64_t maxValueNotIncluded, int64_t *array, int64_t sample_size);

MyVectorInt *my_random_sampleNoRepetitionsSorted(int64_t minValueIncluded,
		int64_t maxValueNotIncluded, double sampleSizeOrFraction);

void my_random_testInt(int64_t minValueIncluded, int64_t maxValueNotIncluded,
		int64_t numSamples);

void my_random_testDouble(double minValueIncluded, double maxValueNotIncluded,
		int64_t numSamples);

#endif
