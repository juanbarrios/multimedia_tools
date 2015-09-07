/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_HISTOGRAM_H
#define MKNN_HISTOGRAM_H

#include "../metricknn_impl.h"

MknnHistogram *mknn_histogram_new(int64_t num_values, double *values,
		int64_t max_bins);
MknnHistogram *mknn_histogram_restore(const char *filenameHist, bool fail);

void mknn_histogram_addValue(MknnHistogram *ch, double value);
double mknn_histogram_getQuantile(MknnHistogram *ch, double value);
double mknn_histogram_getValueQuantile(MknnHistogram *ch, double quantile);

double* mknn_histogram_getBins(MknnHistogram *ch);
int64_t mknn_histogram_getNumBins(MknnHistogram *ch);

void mknn_histogram_save(MknnHistogram *ch, const char *filename);

void mknn_histogram_release(MknnHistogram *ch);

#endif
