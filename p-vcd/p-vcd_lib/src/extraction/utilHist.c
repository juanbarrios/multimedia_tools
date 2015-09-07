/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"
#include "exHist.h"

#ifndef NO_OPENCV
void hist_getBins3Channels(IplImage *image, struct Zone *z,
		void (*func_accumBins)(uchar pixelVal1, uchar pixelVal2,
				uchar pixelVal3, void *state), void *state) {
	int64_t x, y;
	for (y = z->pix_start_h; y < z->pix_end_h; ++y) {
		uchar *ptr1 = (uchar*) (image->imageData + image->widthStep * y);
		for (x = z->pix_start_w; x < z->pix_end_w; ++x) {
			uchar val1 = ptr1[3 * x];
			uchar val2 = ptr1[3 * x + 1];
			uchar val3 = ptr1[3 * x + 2];
			func_accumBins(val1, val2, val3, state);
		}
	}
}

void hist_getBins1Channels(IplImage *image, struct Zone *z,
		void (*func_accumBins)(uchar pixelVal, void *state), void *state) {
	int64_t x, y;
	for (y = z->pix_start_h; y < z->pix_end_h; ++y) {
		uchar *ptr1 = (uchar*) (image->imageData + image->widthStep * y);
		for (x = z->pix_start_w; x < z->pix_end_w; ++x) {
			uchar val = ptr1[x];
			func_accumBins(val, state);
		}
	}
}

int64_t *hist_val2bin(int64_t numBins, int64_t maxVal) {
	int64_t *limits = my_math_partitionIntUB(numBins, maxVal);
	int64_t *val2bin = MY_MALLOC(maxVal, int64_t);
	int64_t val;
	for (val = 0; val < maxVal; ++val) {
		int64_t i, numBin = -1;
		for (i = 0; i < numBins; ++i) {
			if (val < limits[i]) {
				numBin = i;
				break;
			}
		}
		my_assert_greaterEqual_int("numBin", numBin, 0);
		val2bin[val] = numBin;
	}
	MY_FREE(limits);
	return val2bin;
}
void hist_getValsWithBin(int64_t numBin, int64_t *val2bin, int64_t val2bin_length,
		int64_t *out_minVal, int64_t *out_maxVal, int64_t *out_avgVal) {
	int64_t val, minVal = INT64_MAX, maxVal = 0, sumVals = 0, contVals = 0;
	for (val = 0; val < val2bin_length; ++val) {
		if (val2bin[val] == numBin) {
			minVal = MIN(minVal, val);
			maxVal = MAX(maxVal, val);
			sumVals += val;
			contVals++;
		}
	}
	if (contVals == 0) {
		*out_minVal = 0;
		*out_maxVal = 0;
		*out_avgVal = 0;
	} else {
		*out_minVal = minVal;
		*out_maxVal = maxVal;
		*out_avgVal = my_math_round_uint8(sumVals / (double) contVals);
	}
}
void hist_setCols(uchar *vals, uchar val1, uchar val2, uchar val3) {
	vals[0] = val1;
	vals[1] = val2;
	vals[2] = val3;
}
#endif
