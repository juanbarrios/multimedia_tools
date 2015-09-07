/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef EXHIST_H
#define EXHIST_H

#include "ex.h"

#ifndef NO_OPENCV
struct ColorBin {
	uchar col_low[3];
	uchar col_avg[3];
	uchar col_high[3];
};

struct ColorBin *getColorBins_gray(void *state);
struct ColorBin *getColorBins_color(void *state);
struct ColorBin *getColorBins_byChannel(void *state);
struct ColorBin *getColorBins_gradient(void *state);

char *getImageZoningText_gray(void *state);
char *getImageZoningText_color(void *state);
char *getImageZoningText_byChannel(void *state);
char *getImageZoningText_gradient(void *state);

struct MyColorSpace getColorSpace_color(void *state);
struct MyColorSpace getColorSpace_byChannel(void *state);

void hist_getBins3Channels(IplImage *image, struct Zone *z,
		void (*func_accumBins)(uchar pixelVal1, uchar pixelVal2,
				uchar pixelVal3, void *state), void *state);
void hist_getBins1Channels(IplImage *image, struct Zone *z,
		void (*func_accumBins)(uchar pixelVal, void *state), void *state);
int64_t *hist_val2bin(int64_t numBins, int64_t maxVal);
void hist_getValsWithBin(int64_t numBin, int64_t *val2bin, int64_t val2bin_length,
		int64_t *out_minVal, int64_t *out_maxVal, int64_t *out_avgVal);
void hist_setCols(uchar *vals, uchar val1, uchar val2, uchar val3);

#endif
#endif
