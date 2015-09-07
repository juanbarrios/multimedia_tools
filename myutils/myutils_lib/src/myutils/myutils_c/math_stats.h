/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_MATH_STATS_H
#define MY_MATH_STATS_H

#include "../myutils_c.h"

struct MyDataStats {
	int64_t num_samples;
	double minimum, maximum;
	double average, variance, std_dev, skewness, kurtosis, rho;
};

struct MyDataStatsCompute;

typedef void (*my_math_computeStats_addSample)(struct MyDataStatsCompute *dsc,
		void *vector);

struct MyDataStatsCompute *my_math_computeStats_new(int64_t num_dimensions);

int64_t my_math_computeStats_getNumDimensions(struct MyDataStatsCompute *dsc);

my_math_computeStats_addSample my_math_computeStats_getAddSampleFunction(
		MyDatatype dtype_vector);

void my_math_computeStats_getStats(struct MyDataStatsCompute *dsc,
		int64_t id_dimension, struct MyDataStats *out_stats);

double my_math_computeStats_getCovariance(struct MyDataStatsCompute *dsc,
		int64_t dim1, int64_t dim2);

void my_math_computeStats_fillCovarianceMatrix(struct MyDataStatsCompute *dsc,
		double **matrix);

void my_math_computeStats_release(struct MyDataStatsCompute *dsc);

struct MyDataStats my_math_computeStats(int64_t data_size, double *data_values);

char *my_math_statsSummary_newString(struct MyDataStats *stats);

char *my_math_statsDetail_newString(struct MyDataStats *stats,
		const char *field_prefix, const char *field_separator,
		const char *field_suffix);
/* *************** */

struct MyQuantiles {
	//deciles: a0=min, a0_5=median, a1=max
	double a0, a0_1, a0_2, a0_3, a0_4, a0_5, a0_6, a0_7, a0_8, a0_9, a1;
	//other quantiles
	double a0_01, a0_001, a0_0001, a0_00001;
	//other quantiles
	double a0_99, a0_999, a0_9999, a0_99999;
};

struct MyQuantiles my_math_computeQuantiles(int64_t data_size,
		double *data_values);
char *my_math_computeQuantiles_newString(struct MyQuantiles stats,
		const char *field_prefix, const char *field_separator,
		const char *field_suffix);

/* *************** */

struct MyVectorStats {
	int64_t num_dimensions;
	struct MyDataStats *dimension_stats;
	double **covariance;
};
struct MyVectorStatsCompute;
struct MyVectorStatsCompute *my_math_computeVectorStats_new(
		int64_t num_dimensions);
void my_math_computeVectorStats_array(struct MyDataStatsCompute *dsc,
		int64_t num_vectors, double **vectors);
struct MyVectorStats *my_math_computeVectorStats_getStats(
		struct MyVectorStatsCompute *dsc);
void my_math_computeVectorStats_release(struct MyVectorStatsCompute *dsc);

#endif
