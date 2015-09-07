/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_MATH_UTIL_H
#define MY_MATH_UTIL_H

#include "../myutils_c.h"

#define MY_BETWEEN(value, desde, hasta) ((value) >= (desde) && (value) <= (hasta))
#define MY_INTERSECT(desde1, hasta1, desde2, hasta2) (MY_BETWEEN((desde1),(desde2),(hasta2))||MY_BETWEEN((hasta1),(desde2),(hasta2))||MY_BETWEEN((desde2),(desde1),(hasta1)))
#define MY_IS_REAL(value) (!isnan(value)&&!isinf(value))
#define MY_RAD2GRAD(radianes) (180.0*radianes/M_PI)

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define MY_ABS_SHORT abs
#define MY_ABS_INT   llabs
#define MY_ABS_DOUBLE fabs

/**
 * intersection/MIN(length1,length2)
 * @param start1
 * @param end1
 * @param start2
 * @param end2
 * @return
 */
double my_math_pctIntersection(double start1, double end1, double start2,
		double end2);

double my_math_averageDoubleArray(int64_t num_values, double *values);
double my_math_averageIntArray(int64_t num_values, int64_t *values);

int64_t my_math_round_int(double d);
uint8_t my_math_round_uint8(double d);
int64_t my_math_floor_int(double d);
int64_t my_math_ceil_int(double d);
double my_math_fractionalPart(double d);

int64_t my_math_getFractionSize(double fraction, int64_t max_number);

int64_t* my_math_partitionIntUB(int64_t numSegments,
		int64_t maxValueNotIncluded);
double* my_math_partitionDoubleUB(int64_t numBins, double maxValue);
MyVectorInt *my_math_computeBinSizes(double binsize, int64_t maxValue);

float my_math_getMaxFloatArray(float *array, int64_t length);
double my_math_getMaxDoubleArray(double *array, int64_t length);

float my_math_getSumFloatArray(float *array, int64_t length);
double my_math_getSumDoubleArray(double *array, int64_t length);

void my_math_scaleFloatArray(float *array, int64_t length, float factor);
void my_math_scaleDoubleArray(double *array, int64_t length, double factor);

void my_math_normalizeSum1_float(float *array, int64_t length);
void my_math_normalizeSum1_double(double *array, int64_t length);
void my_math_normalizeNorm1_double(double *array, int64_t length);
void my_math_normalizeMax1_float(float *array, int64_t length);
void my_math_normalizeMax1_double(double *array, int64_t length);

struct MyLinearMatrix {
	int64_t num_dimensions;
	int64_t length_d1, length_d2, length_d3;
	int64_t total_size, length_sum_d2d3;
};
struct MyLinearMatrix my_linear2d_new(int64_t length_d1, int64_t length_d2);
int64_t my_linear2d_get1d(int64_t pos_d1, int64_t pos_d2,
		struct MyLinearMatrix *lm);
void my_linear2d_getCoords(int64_t position, int64_t *out_pos_d1,
		int64_t *out_pos_d2, struct MyLinearMatrix *lm);

struct MyLinearMatrix my_linear3d_new(int64_t length_d1, int64_t length_d2,
		int64_t length_d3);
int64_t my_linear3d_get1d(int64_t pos_d1, int64_t pos_d2, int64_t pos_d3,
		struct MyLinearMatrix *lm);
void my_linear3d_getCoords(int64_t position, int64_t *out_pos_d1,
		int64_t *out_pos_d2, int64_t *out_pos_d3, struct MyLinearMatrix *lm);

#endif
