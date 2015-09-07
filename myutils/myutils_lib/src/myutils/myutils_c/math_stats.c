/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "math_util.h"

struct DataStats {
	double min, max;
	double avg, sum_diffs;
	//double sum_diff3, sum_diff4;
	double last_diff;
	double *sum_codiff;
};
struct MyDataStatsCompute {
	int64_t num_dimensions;
	int64_t cont_samples;
	struct DataStats *stats_by_dimension;
};

struct MyDataStatsCompute *my_math_computeStats_new(int64_t num_dimensions) {
	struct MyDataStatsCompute *dsc = MY_MALLOC(1, struct MyDataStatsCompute);
	dsc->num_dimensions = num_dimensions;
	dsc->stats_by_dimension = MY_MALLOC(num_dimensions, struct DataStats);
	for (int64_t i = 0; i < dsc->num_dimensions; i++) {
		struct DataStats *dim_stats = dsc->stats_by_dimension + i;
		dim_stats->sum_codiff = MY_MALLOC(dsc->num_dimensions, double);
	}
	return dsc;
}
int64_t my_math_computeStats_getNumDimensions(struct MyDataStatsCompute *dsc) {
	return dsc->num_dimensions;
}
static void addValue(struct MyDataStatsCompute *dsc, int64_t i, double value) {
	struct DataStats *dim_stats = dsc->stats_by_dimension + i;
	if (dsc->cont_samples <= 1 || value > dim_stats->max)
		dim_stats->max = value;
	if (dsc->cont_samples <= 1 || value < dim_stats->min)
		dim_stats->min = value;
	//computing average and variance: Knuth, the ACP, vol.2, pag 232
	//http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
	//http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
	double diffpre = value - dim_stats->avg;
	dim_stats->avg += diffpre / dsc->cont_samples;
	double diffpos = value - dim_stats->avg;
	dim_stats->sum_diffs += diffpre * diffpos;
	//double diffpos2 = diffpos * diffpos;
	//dim_stats->sum_diff3 += diffpos2 * diffpos;
	//dim_stats->sum_diff4 += diffpos2 * diffpos2;
	dim_stats->last_diff = diffpos;
	for (int64_t j = 0; j < i; j++) {
		struct DataStats *co_dim_stats = dsc->stats_by_dimension + j;
		co_dim_stats->sum_codiff[i] += diffpre * co_dim_stats->last_diff;
	}
}

#define ADDSAMPLE_FUNCTIONS(typeVector) \
static void addSample_##typeVector(struct MyDataStatsCompute *dsc, void *vector) { \
	typeVector *array = vector; \
	dsc->cont_samples++; \
	for (int64_t i = 0; i < dsc->num_dimensions; i++) { \
		addValue(dsc, i, array[i]); \
	} \
}
ADDSAMPLE_FUNCTIONS(int8_t)
ADDSAMPLE_FUNCTIONS(int16_t)
ADDSAMPLE_FUNCTIONS(int32_t)
ADDSAMPLE_FUNCTIONS(int64_t)
ADDSAMPLE_FUNCTIONS(uint8_t)
ADDSAMPLE_FUNCTIONS(uint16_t)
ADDSAMPLE_FUNCTIONS(uint32_t)
ADDSAMPLE_FUNCTIONS(uint64_t)
ADDSAMPLE_FUNCTIONS(float)
ADDSAMPLE_FUNCTIONS(double)

#define INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, constantDatatype, nameDatatype) \
if (varDatatype.my_datatype_code == constantDatatype.my_datatype_code) { \
	return namePrefix##nameDatatype; \
}

#define RETURN_SINGLE_DATATYPE(varDatatype, namePrefix) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_INT8    , int8_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_INT16   , int16_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_INT32   , int32_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_INT64   , int64_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_UINT8   , uint8_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_UINT16  , uint16_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_UINT32  , uint32_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_UINT64  , uint64_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_FLOAT32 , float) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, MY_DATATYPE_FLOAT64 , double)

my_math_computeStats_addSample my_math_computeStats_getAddSampleFunction(
		MyDatatype dtype_vector) {
	RETURN_SINGLE_DATATYPE(dtype_vector, addSample_)
	my_log_error("unknown type %i\n", (int) dtype_vector.my_datatype_code);
	return NULL;
}

void my_math_computeStats_getStats(struct MyDataStatsCompute *dsc,
		int64_t id_dimension, struct MyDataStats *out_stats) {
	struct DataStats *dim_stats = dsc->stats_by_dimension + id_dimension;
	out_stats->num_samples = dsc->cont_samples;
	out_stats->minimum = dim_stats->min;
	out_stats->maximum = dim_stats->max;
	out_stats->average = dim_stats->avg;
	double variance =
			(dsc->cont_samples == 0) ?
					0 : dim_stats->sum_diffs / dsc->cont_samples;
	if (variance == 0) {
		out_stats->variance = out_stats->std_dev = out_stats->skewness =
				out_stats->kurtosis = out_stats->rho = 0;
	} else {
		out_stats->variance = variance;
		out_stats->std_dev = sqrt(variance);
		//double variance_2 = variance * variance;
		//double variance_3 = variance * variance_2;
		//out_stats->skewness = (dim_stats->sum_diff3 / dsc->cont_samples)/ sqrt(variance_3);
		//out_stats->kurtosis = (dim_stats->sum_diff4 / dsc->cont_samples)/ variance_2 - 3;
		out_stats->rho = dim_stats->avg * (dim_stats->avg / variance) / 2;
	}
}
double my_math_computeStats_getCovariance(struct MyDataStatsCompute *dsc,
		int64_t dim1, int64_t dim2) {
	if (dim1 == dim2)
		return dsc->stats_by_dimension[dim1].sum_diffs / dsc->cont_samples;
	else if (dim1 < dim2)
		return (dsc->stats_by_dimension + dim1)->sum_codiff[dim2]
				/ dsc->cont_samples;
	else
		return (dsc->stats_by_dimension + dim2)->sum_codiff[dim1]
				/ dsc->cont_samples;
}
void my_math_computeStats_fillCovarianceMatrix(struct MyDataStatsCompute *dsc,
		double **matrix) {
	for (int64_t i = 0; i < dsc->num_dimensions; i++) {
		for (int64_t j = 0; j <= i; j++) {
			matrix[i][j] = matrix[j][i] = my_math_computeStats_getCovariance(
					dsc, i, j);
		}
	}
}
void my_math_computeStats_release(struct MyDataStatsCompute *dsc) {
	if (dsc == NULL)
		return;
	for (int64_t i = 0; i < dsc->num_dimensions; i++) {
		struct DataStats *dim_stats = dsc->stats_by_dimension + i;
		free(dim_stats->sum_codiff);
	}
	free(dsc->stats_by_dimension);
	free(dsc);
}

struct MyDataStats my_math_computeStats(int64_t data_size, double *data_values) {
	struct MyDataStatsCompute *dsc = my_math_computeStats_new(1);
	for (int64_t i = 0; i < data_size; ++i) {
		dsc->cont_samples++;
		addValue(dsc, 0, data_values[i]);
	}
	struct MyDataStats stats = { 0 };
	my_math_computeStats_getStats(dsc, 0, &stats);
	my_math_computeStats_release(dsc);
	return stats;
}
char *my_math_statsSummary_newString(struct MyDataStats *stats) {
	char *st1 = my_newString_int(stats->num_samples);
	char *st2 = my_newString_doubleDec(stats->minimum, 1);
	char *st3 = my_newString_doubleDec(stats->maximum, 1);
	char *st4 = my_newString_doubleDec(stats->average, 1);
	char *st5 = my_newString_doubleDec(stats->variance, 1);
	char *txt = my_newString_format("samples=%s min=%s max=%s avg=%s var=%s",
			st1, st2, st3, st4, st5);
	MY_FREE_MULTI(st1, st2, st3, st4, st5);
	return txt;
}
static void append_DataStat(MyStringBuffer *sb, const char *field_name,
		double field_value, const char *field_prefix,
		const char *field_separator, const char *field_suffix) {
	my_stringbuf_appendString(sb, field_prefix);
	my_stringbuf_appendString(sb, field_name);
	my_stringbuf_appendString(sb, field_separator);
	my_stringbuf_appendDouble(sb, field_value);
	my_stringbuf_appendString(sb, field_suffix);
}
char *my_math_statsDetail_newString(struct MyDataStats *stats,
		const char *field_prefix, const char *field_separator,
		const char *field_suffix) {
	MyStringBuffer *sb = my_stringbuf_new();
	append_DataStat(sb, "samples", stats->num_samples, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "minimum", stats->minimum, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "maximum", stats->maximum, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "average", stats->average, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "variance", stats->variance, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "std_dev", stats->std_dev, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "skewness", stats->skewness, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "kurtosis", stats->kurtosis, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "rho", stats->rho, field_prefix, field_separator,
			field_suffix);
	return my_stringbuf_releaseReturnBuffer(sb);
}

/* *************** */

static double get_value_at(int64_t data_size, double *data_values,
		double fraction) {
	int64_t pos = my_math_round_int((data_size - 1) * fraction);
	my_assert_indexRangeInt("pos", pos, data_size);
	return data_values[pos];
}
struct MyQuantiles my_math_computeQuantiles(int64_t data_size,
		double *data_values) {
	struct MyQuantiles quant = { .a0 = 0, .a1 = 0 };
	if (data_size == 0)
		return quant;
//in order to compute quantiles, data must be sorted
	MyTimer *timer = my_timer_new();
	if (data_size > 1000000)
		my_log_info("sorting %"PRIi64" values (1 thread)...\n", data_size);
	my_qsort_double_array(data_values, data_size);
	double secs = my_timer_getSeconds(timer);
	if (secs > 5)
		my_log_info("%"PRIi64" values sorted in %1.1lf seconds\n", data_size,
				secs);
	my_timer_release(timer);
	quant.a1 = get_value_at(data_size, data_values, 1);
	quant.a0_9 = get_value_at(data_size, data_values, 0.9);
	quant.a0_8 = get_value_at(data_size, data_values, 0.8);
	quant.a0_7 = get_value_at(data_size, data_values, 0.7);
	quant.a0_6 = get_value_at(data_size, data_values, 0.6);
	quant.a0_5 = get_value_at(data_size, data_values, 0.5);
	quant.a0_4 = get_value_at(data_size, data_values, 0.4);
	quant.a0_3 = get_value_at(data_size, data_values, 0.3);
	quant.a0_2 = get_value_at(data_size, data_values, 0.2);
	quant.a0_1 = get_value_at(data_size, data_values, 0.1);
	quant.a0 = get_value_at(data_size, data_values, 0);
	quant.a0_01 = get_value_at(data_size, data_values, 0.01);
	quant.a0_001 = get_value_at(data_size, data_values, 0.001);
	quant.a0_0001 = get_value_at(data_size, data_values, 0.0001);
	quant.a0_00001 = get_value_at(data_size, data_values, 0.00001);
	quant.a0_99 = get_value_at(data_size, data_values, 0.99);
	quant.a0_999 = get_value_at(data_size, data_values, 0.999);
	quant.a0_9999 = get_value_at(data_size, data_values, 0.9999);
	quant.a0_99999 = get_value_at(data_size, data_values, 0.99999);
	return quant;
}

char *my_math_computeQuantiles_newString(struct MyQuantiles stats,
		const char *field_prefix, const char *field_separator,
		const char *field_suffix) {
	MyStringBuffer *sb = my_stringbuf_new();
	append_DataStat(sb, "alpha 0", stats.a0, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.1", stats.a0_1, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.2", stats.a0_2, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.3", stats.a0_3, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.4", stats.a0_4, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.5", stats.a0_5, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.6", stats.a0_6, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.7", stats.a0_7, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.8", stats.a0_8, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.9", stats.a0_9, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 1", stats.a1, field_prefix, field_separator,
			field_suffix);
	append_DataStat(sb, "alpha 0.01", stats.a0_01, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.001", stats.a0_001, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.0001", stats.a0_0001, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.00001", stats.a0_00001, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.99", stats.a0_99, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.999", stats.a0_999, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.9999", stats.a0_9999, field_prefix,
			field_separator, field_suffix);
	append_DataStat(sb, "alpha 0.99999", stats.a0_99999, field_prefix,
			field_separator, field_suffix);
	return my_stringbuf_releaseReturnBuffer(sb);
}
