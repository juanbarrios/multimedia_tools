/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
//IWAMOTO, KASUTANI, YAMADA 2006
struct Estado_EDGES {
	int64_t numDivisionsW, numDivisionsH;
	int64_t lastW, lastH, *limitsW, *limitsH;
	int64_t numKernels;
	double threshold, **kernels;
	uchar *idsKernels;
};

static void ext_config_edg(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct Estado_EDGES *es = MY_MALLOC(1, struct Estado_EDGES);
	es->numDivisionsW = my_tokenizer_nextInt(tk);
	es->numDivisionsH = my_tokenizer_nextInt(tk);
	es->threshold = my_tokenizer_nextDouble(tk);
	es->numKernels = 10;
	es->kernels = MY_MALLOC_NOINIT(es->numKernels, double*);
	int64_t n = 0;
	es->kernels[n++] = newKernel(-1, 1, -1, 1);
	es->kernels[n++] = newKernel(0, sqrt(2), -sqrt(2), 0);
	es->kernels[n++] = newKernel(1, 1, -1, -1);
	es->kernels[n++] = newKernel(sqrt(2), 0, 0, -sqrt(2));
	es->kernels[n++] = newKernel(1, -1, 1, -1);
	es->kernels[n++] = newKernel(0, -sqrt(2), sqrt(2), 0);
	es->kernels[n++] = newKernel(-1, -1, 1, 1);
	es->kernels[n++] = newKernel(-sqrt(2), 0, 0, sqrt(2));
	es->kernels[n++] = newKernel(2, -2, -2, 2);
	es->kernels[n++] = newKernel(-2, 2, 2, -2);
	if (n != es->numKernels)
		my_log_error("error edges->numKernels en edges\n");
	my_tokenizer_releaseValidateEnd(tk);
	int64_t largo = es->numDivisionsW * es->numDivisionsH;
	es->idsKernels = MY_MALLOC_NOINIT(largo, uchar);
	*out_state = es;
	*out_td = descriptorType(DTYPE_ARRAY_UCHAR, largo, es->numDivisionsW,
			es->numDivisionsH);
	*out_useImgGray = true;
}

static int64_t edg_buscarMaxKernel(IplImage *imgGris, int64_t numBloqW,
		int64_t numBloqH, struct Estado_EDGES *es) {
	int64_t iniW = numBloqW > 0 ? es->limitsW[2 * numBloqW - 1] : 0;
	int64_t iniH = numBloqH > 0 ? es->limitsH[2 * numBloqH - 1] : 0;
	int64_t medW = es->limitsW[2 * numBloqW];
	int64_t medH = es->limitsH[2 * numBloqH];
	int64_t finW = es->limitsW[2 * numBloqW + 1];
	int64_t finH = es->limitsH[2 * numBloqH + 1];
	double bloq_00 = averageIntensity(imgGris, iniW, iniH, medW, medH);
	double bloq_01 = averageIntensity(imgGris, medW, iniH, finW, medH);
	double bloq_10 = averageIntensity(imgGris, iniW, medH, medW, finH);
	double bloq_11 = averageIntensity(imgGris, medW, medH, finW, finH);
	int64_t k, kernelMax = es->numKernels; //maximo por defecto
	double val, valMax = es->threshold; //threshold minimo a superar
	for (k = 0; k < es->numKernels; ++k) {
		double* kernel = es->kernels[k];
		val = kernel[0] * bloq_00 + kernel[1] * bloq_01 + kernel[2] * bloq_10
				+ kernel[3] * bloq_11;
		if (val >= valMax) {
			valMax = val;
			kernelMax = k;
		}
	}
	return kernelMax;
}
static void *ext_extraer_edg(IplImage *image, void *state) {
	struct Estado_EDGES *es = state;
	if (es->lastW != image->width || es->lastH != image->height) {
		MY_FREE_MULTI(es->limitsW, es->limitsH);
		es->limitsW = my_math_partitionIntUB(2 * es->numDivisionsW,
				image->width);
		es->limitsH = my_math_partitionIntUB(2 * es->numDivisionsH,
				image->height);
		es->lastW = image->width;
		es->lastH = image->height;
	}
	int64_t i, j, cont = 0;
	for (j = 0; j < es->numDivisionsH; ++j)
		for (i = 0; i < es->numDivisionsW; ++i)
			es->idsKernels[cont++] = edg_buscarMaxKernel(image, i, j, es);
	return es->idsKernels;
}

static void ext_release_edg(void *estado) {
	struct Estado_EDGES *es = estado;
	MY_FREE_MULTI(es->kernels, es->idsKernels, es->limitsW, es->limitsH, es);
}
void ext_reg_edg() {
	addExtractorGlobalDef(false, "EDGES",
			"[numDivisionsW]X[numDivisionsH]_[threshold]", ext_config_edg,
			ext_extraer_edg, ext_release_edg);
}
#endif
