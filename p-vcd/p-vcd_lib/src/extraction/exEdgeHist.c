/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
//MANJUNATH 2001 MPEG7
struct State_EHD {
	int64_t numDivisionsW, numDivisionsH, numSubDivisionsW, numSubDivisionsH;
	int64_t lastW, lastH, *limitsW, *limitsH;
	int64_t numKernels;
	double threshold, **kernels, *double_array;
	int64_t length;
	struct Quantization quant;
	uchar withAbs;
	void *descriptor;
};

static double **ehd_getKernels(int64_t numKernels) {
	if (numKernels == 5) {
		//Manjunath et al. 2001
		double **k_manjunath = MY_MALLOC_NOINIT(5, double*);
		k_manjunath[0] = newKernel(1, -1, 1, -1);
		k_manjunath[1] = newKernel(1, 1, -1, -1);
		k_manjunath[2] = newKernel(sqrt(2), 0, 0, -sqrt(2));
		k_manjunath[3] = newKernel(0, sqrt(2), -sqrt(2), 0);
		k_manjunath[4] = newKernel(2, -2, -2, 2);
		return k_manjunath;
	} else if (numKernels == 10) {
		//Iwamoto et al.
		double **k_iwamoto = MY_MALLOC_NOINIT(10, double*);
		k_iwamoto[0] = newKernel(-1, 1, -1, 1); //0 -> 4
		k_iwamoto[1] = newKernel(0, sqrt(2), -sqrt(2), 0); //1 -> 3
		k_iwamoto[2] = newKernel(1, 1, -1, -1); //2 -> 2
		k_iwamoto[3] = newKernel(sqrt(2), 0, 0, -sqrt(2)); //3 -> 1
		k_iwamoto[4] = newKernel(1, -1, 1, -1); //4 -> 0
		k_iwamoto[5] = newKernel(0, -sqrt(2), sqrt(2), 0); //5 -> 7
		k_iwamoto[6] = newKernel(-1, -1, 1, 1); //6 -> 6
		k_iwamoto[7] = newKernel(-sqrt(2), 0, 0, sqrt(2)); // 7 -> 5
		k_iwamoto[8] = newKernel(2, -2, -2, 2); // 8-> 9
		k_iwamoto[9] = newKernel(-2, 2, 2, -2); // 9-> 8
		return k_iwamoto;
	}
	my_log_error("error kernels ehd\n");
	return NULL;
}

static void ext_config_ehd(const char *code, const char *parameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct State_EHD *es = MY_MALLOC(1, struct State_EHD);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	es->numDivisionsW = my_tokenizer_nextInt(tk2);
	es->numDivisionsH = my_tokenizer_nextInt(tk2);
	my_tokenizer_release(tk2);
	tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	es->numSubDivisionsW = my_tokenizer_nextInt(tk2);
	es->numSubDivisionsH = my_tokenizer_nextInt(tk2);
	my_tokenizer_release(tk2);
	es->threshold = my_tokenizer_nextDouble(tk);
	const char *token = my_tokenizer_nextToken(tk);
	if (token != NULL && token[0] == 'K')
		es->numKernels = my_parse_int_isubFromEnd(token, 1);
	else
		my_log_error("unknown kernels '%s'\n", token);
	es->kernels = ehd_getKernels(es->numKernels);
	es->withAbs = my_tokenizer_isNext(tk, "ABS") ? 1 : 0;
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	my_tokenizer_releaseValidateEnd(tk);
	es->length = es->numDivisionsW * es->numDivisionsH * es->numKernels;
	es->double_array = MY_MALLOC_NOINIT(es->length, double);
	es->descriptor = newDescriptorQuantize(es->quant, es->length);
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->length, es->numDivisionsW,
			es->numDivisionsH);
	*out_useImgGray = true;
}

static int64_t ehd_getIdMaxKernel(IplImage *imgGray, int64_t posW, int64_t posH,
		struct State_EHD *es) {
	int64_t startW = posW > 0 ? es->limitsW[posW - 1] : 0;
	int64_t startH = posH > 0 ? es->limitsH[posH - 1] : 0;
	int64_t medW = es->limitsW[posW];
	int64_t medH = es->limitsH[posH];
	int64_t endW = es->limitsW[posW + 1];
	int64_t endH = es->limitsH[posH + 1];
	double bloq_00 = averageIntensity(imgGray, startW, startH, medW, medH);
	double bloq_01 = averageIntensity(imgGray, medW, startH, endW, medH);
	double bloq_10 = averageIntensity(imgGray, startW, medH, medW, endH);
	double bloq_11 = averageIntensity(imgGray, medW, medH, endW, endH);
	int64_t k, idMaxKernel = -1; //default maximum
	double maxVal = es->threshold; //minimum threshold
	for (k = 0; k < es->numKernels; ++k) {
		double* kernel = es->kernels[k];
		double val = kernel[0] * bloq_00 + kernel[1] * bloq_01
				+ kernel[2] * bloq_10 + kernel[3] * bloq_11;
		if (es->withAbs && val < 0)
			val = -val;
		if (val >= maxVal) {
			maxVal = val;
			idMaxKernel = k;
		}
	}
	return idMaxKernel;
}
static void ehd_compute_histogram(IplImage *imgGray, int64_t numBloqW,
		int64_t numBloqH, double *values, struct State_EHD *es) {
	int64_t i, j;
	int64_t posH = 2 * es->numSubDivisionsH * numBloqH;
	for (j = 0; j < es->numSubDivisionsH; ++j) {
		int64_t posW = 2 * es->numSubDivisionsW * numBloqW;
		for (i = 0; i < es->numSubDivisionsW; ++i) {
			int64_t idKernel = ehd_getIdMaxKernel(imgGray, posW, posH, es);
			if (idKernel >= 0) {
				values[idKernel]++;
			}
			posW += 2;
		}
		posH += 2;
	}
	my_math_normalizeSum1_double(values, es->numKernels);
}
static void *ext_extract_ehd(IplImage *image, void *state) {
	struct State_EHD *es = state;
	if (es->lastW != image->width || es->lastH != image->height) {
		MY_FREE_MULTI(es->limitsW, es->limitsH);
		es->limitsW = my_math_partitionIntUB(
				2 * es->numDivisionsW * es->numSubDivisionsW, image->width);
		es->limitsH = my_math_partitionIntUB(
				2 * es->numDivisionsH * es->numSubDivisionsH, image->height);
		es->lastW = image->width;
		es->lastH = image->height;
	}
	MY_SETZERO(es->double_array, es->length, double);
	double *double_array = es->double_array;
	for (int64_t j = 0; j < es->numDivisionsH; ++j) {
		for (int64_t i = 0; i < es->numDivisionsW; ++i) {
			ehd_compute_histogram(image, i, j, double_array, es);
			double_array += es->numKernels;
		}
	}
	quantize(es->quant, es->double_array, es->descriptor, es->length);
	return es->descriptor;
}
static void ext_release_ehd(void *state) {
	struct State_EHD *es = state;
	MY_FREE_MULTI(es->descriptor, es->double_array, es->kernels, es->limitsW,
			es->limitsH, es);
}
void ext_reg_ehd() {
	addExtractorGlobalDef(false, "EHD",
			"(zonesW)x(zonesH)_(numSubDivisionsW)x(numSubDivisionsH)_[threshold]_K[numKernels=(5|10)][_opt:ABS]_(quant:1U|4F...)",
			ext_config_ehd, ext_extract_ehd, ext_release_ehd);
}
#endif
