/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "math_util.h"

double my_math_pctIntersection(double start1, double end1, double start2,
		double end2) {
	if (end1 <= start2 || end2 <= start1) {
		//no intersection
		return 0;
	} else if ((start2 <= start1 && end1 <= end2)
			|| (start1 <= start2 && end2 <= end1)) {
		//full intersection
		return 1;
	}
//the proportion between intersection and length
	double l1 = end1 - start1;
	double l2 = end2 - start2;
	if (l1 <= 0 || l2 <= 0)
		return 1;
	double l12 = end2 - start1;
	double l21 = end1 - start2;
	my_assert_greaterDouble("l12", l12, 0);
	my_assert_greaterDouble("l21", l21, 0);
	return MIN(l12, l21) / MIN(l1, l2);
}
double my_math_averageDoubleArray(int64_t num_values, double *values) {
	double avg = 0;
	for (int64_t i = 0; i < num_values; i++) {
		avg += (values[i] - avg) / (i + 1);
	}
	return avg;
}
double my_math_averageIntArray(int64_t num_values, int64_t *values) {
	double avg = 0;
	for (int64_t i = 0; i < num_values; i++) {
		avg += (values[i] - avg) / (i + 1);
	}
	return avg;
}

double round(double x); //math.h

int64_t my_math_round_int(double d) {
	return (int64_t) round(d);
}
uint8_t my_math_round_uint8(double d) {
	int64_t n = my_math_round_int(d);
	if (n <= 0)
		return 0;
	else if (n >= 255)
		return 255;
	return n;
}
int64_t my_math_floor_int(double d) {
	return (int64_t) d;
}
int64_t my_math_ceil_int(double d) {
	int64_t n = (int64_t) round(d);
	return n == d ? n : n + 1;
}
double my_math_fractionalPart(double d) {
	d = fabs(d);
	return d - ((int64_t) d);
}
int64_t my_math_getFractionSize(double fraction, int64_t max_number) {
	if (fraction <= 0) {
		return 0;
	} else if (fraction > 1) {
		return my_math_round_int(fraction);
	} else {
		int64_t n = my_math_round_int(fraction * max_number);
		return MIN(MAX(1, n), max_number);
	}
}

//returns an array with the upper-bounds for each segment (u-b not included in the segment)
int64_t* my_math_partitionIntUB(int64_t numSegments,
		int64_t maxValueNotIncluded) {
	int64_t* limits = MY_MALLOC_NOINIT(numSegments, int64_t);
	int64_t width_floor = maxValueNotIncluded / numSegments;
	double width_exact = maxValueNotIncluded / ((double) numSegments);
	if (width_floor == width_exact) {
		for (int64_t i = 0; i < numSegments; i++)
			limits[i] = (i + 1) * width_floor;
	} else {
		double errorRoundDown = width_exact - width_floor;
		double errorRoundUp = width_floor + 1 - width_exact;
		double accumError = 0;
		for (int64_t i = 0; i < numSegments; i++) {
			limits[i] = (i == 0) ? 0 : limits[i - 1];
			limits[i] += width_floor;
			if (MY_ABS_DOUBLE(accumError + errorRoundDown)
					<= MY_ABS_DOUBLE(accumError - errorRoundUp)) {
				accumError += errorRoundDown;
			} else {
				limits[i]++;
				accumError -= errorRoundUp;
			}
		}
	}
	my_assert_equalInt("maxValue", limits[numSegments - 1],
			maxValueNotIncluded);
	return limits;
}
double* my_math_partitionDoubleUB(int64_t numBins, double maxValue) {
	double* limites = MY_MALLOC_NOINIT(numBins, double);
	double w = maxValue / numBins;
	for (int64_t i = 0; i < numBins; i++)
		limites[i] = ((i + 1) * w);
	limites[numBins - 1] = maxValue + 1; //xsiaca
	return limites;
}
//calcula intervalos discretos de tamano binsize
//retorna una lista con los inicios de cada intervalo
//el fin de cada intervalo es el numero sgte menos 1
//el primer value es 0 y el ultimo es >= maxValue
MyVectorInt *my_math_computeBinSizes(double binsize, int64_t maxValue) {
	MyVectorInt *limites = my_vectorInt_new_sorted(1, 1);
	int64_t size1 = MAX(1, (int64_t ) binsize);
	if (size1 == binsize) {
		int64_t current = 0;
		while (current < maxValue) {
			my_vectorInt_add(limites, current);
			current += size1;
		}
		my_vectorInt_add(limites, current);
	} else {
		int64_t current = 0;
		int64_t size2 = size1 + 1;
		double errorAcum = 0, esperado = 0;
		while (current < maxValue) {
			my_vectorInt_add(limites, current);
			esperado += binsize;
			double err1 = current + size1 - esperado;
			double err2 = current + size2 - esperado;
			if (MY_ABS_DOUBLE(errorAcum + err1)
					< MY_ABS_DOUBLE(errorAcum + err2)) {
				errorAcum += err1;
				current += size1;
			} else {
				errorAcum += err2;
				current += size2;
			}
		}
		my_vectorInt_add(limites, current);
	}
	return limites;
}

/*****************/
double my_math_getMaxDoubleArray(double *array, int64_t length) {
	my_assert_greaterDouble("array length", length, 0);
	double max = array[0];
	for (int64_t i = 1; i < length; ++i) {
		if (array[i] > max)
			max = array[i];
	}
	return max;
}
/*****************/
double my_math_getSumDoubleArray(double *array, int64_t length) {
	double sum = 0;
	for (int64_t i = 0; i < length; ++i)
		sum += array[i];
	return sum;
}
/*****************/
void my_math_scaleDoubleArray(double *array, int64_t length, double factor) {
	for (int64_t i = 0; i < length; ++i)
		array[i] *= factor;
}
/*****************/
void my_math_normalizeSum1_double(double *array, int64_t length) {
	double sum = my_math_getSumDoubleArray(array, length);
	if (sum > 0)
		my_math_scaleDoubleArray(array, length, 1 / sum);
}
/*****************/
void my_math_normalizeNorm1_double(double *array, int64_t length) {
	double norm = 0;
	for (int64_t i = 0; i < length; ++i)
		norm += array[i] * array[i];
	if (norm > 0) {
		my_math_scaleDoubleArray(array, length, 1 / sqrt(norm));
	}
}
/*****************/
void my_math_normalizeMax1_double(double *array, int64_t length) {
	double max = my_math_getMaxDoubleArray(array, length);
	if (max > 0)
		my_math_scaleDoubleArray(array, length, 1 / max);
}

/*******************/
struct MyLinearMatrix my_linear2d_new(int64_t length_d1, int64_t length_d2) {
	struct MyLinearMatrix lm = { 0 };
	lm.num_dimensions = 2;
	lm.length_d1 = length_d1;
	lm.length_d2 = length_d2;
	lm.total_size = length_d1 * length_d2;
	return lm;
}
int64_t my_linear2d_get1d(int64_t pos_d1, int64_t pos_d2,
		struct MyLinearMatrix *lm) {
	my_assert_equalInt("num_dimensions", lm->num_dimensions, 2);
	my_assert_indexRangeInt("pos_d1", pos_d1, lm->length_d1);
	my_assert_indexRangeInt("pos_d2", pos_d2, lm->length_d2);
	return pos_d1 * lm->length_d2 + pos_d2;
}
void my_linear2d_getCoords(int64_t position, int64_t *out_pos_d1,
		int64_t *out_pos_d2, struct MyLinearMatrix *lm) {
	my_assert_equalInt("num_dimensions", lm->num_dimensions, 2);
	my_assert_indexRangeInt("position", position, lm->total_size);
	*out_pos_d1 = position / lm->length_d2;
	*out_pos_d2 = position % lm->length_d2;
}
struct MyLinearMatrix my_linear3d_new(int64_t length_d1, int64_t length_d2,
		int64_t length_d3) {
	struct MyLinearMatrix lm = { 0 };
	lm.num_dimensions = 3;
	lm.length_d1 = length_d1;
	lm.length_d2 = length_d2;
	lm.length_d3 = length_d3;
	lm.length_sum_d2d3 = length_d2 + length_d3;
	lm.total_size = length_d1 * length_d2 * length_d3;
	return lm;
}
int64_t my_linear3d_get1d(int64_t pos_d1, int64_t pos_d2, int64_t pos_d3,
		struct MyLinearMatrix *lm) {
	my_assert_equalInt("num_dimensions", lm->num_dimensions, 3);
	my_assert_indexRangeInt("pos_d1", pos_d1, lm->length_d1);
	my_assert_indexRangeInt("pos_d2", pos_d2, lm->length_d2);
	my_assert_indexRangeInt("pos_d3", pos_d3, lm->length_d3);
	return pos_d1 * lm->length_sum_d2d3 + pos_d2 * lm->length_d3 + pos_d3;
}
void my_linear3d_getCoords(int64_t position, int64_t *out_pos_d1,
		int64_t *out_pos_d2, int64_t *out_pos_d3, struct MyLinearMatrix *lm) {
	my_assert_equalInt("num_dimensions", lm->num_dimensions, 3);
	my_assert_indexRangeInt("position", position, lm->total_size);
	*out_pos_d1 = position / lm->length_sum_d2d3;
	int64_t rest = position % lm->length_sum_d2d3;
	*out_pos_d2 = rest / lm->length_d3;
	*out_pos_d3 = rest % lm->length_d3;
}
