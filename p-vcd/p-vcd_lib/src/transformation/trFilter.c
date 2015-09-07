/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
#define F_TYPE_MIN 2
#define F_TYPE_MAX 3
#define F_TYPE_MEDIAN 4
#define F_TYPE_AVERAGE 10
#define F_TYPE_CONVOLUTION 11

struct Proceso_FILTRO {
	int64_t filtro_width, filtro_height, filtro_tipo, filtro_size;
	double *valores_kernel;
	int64_t array_dim1, array_dim2, array_dim3;
	double ***my_convolution_perform;
	uchar convol_abs;
	double convol_max;
	IplImage *imgOut;
	uchar *ventana;
	uchar useOpenCV, addToImg;
	struct MyConvolutionKernel *kc;
};

static void tra_new_filtro(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct Proceso_FILTRO *es = MY_MALLOC(1, struct Proceso_FILTRO);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	es->filtro_width = my_tokenizer_nextInt(tk2);
	es->filtro_height = my_tokenizer_nextInt(tk2);
	my_tokenizer_release(tk2);
	es->filtro_size = es->filtro_width * es->filtro_height;
	if (my_tokenizer_isNext(tk, "MEDIAN"))
		es->filtro_tipo = F_TYPE_MEDIAN;
	else if (my_tokenizer_isNext(tk, "MAX"))
		es->filtro_tipo = F_TYPE_MAX;
	else if (my_tokenizer_isNext(tk, "MIN"))
		es->filtro_tipo = F_TYPE_MIN;
	else if (my_tokenizer_isNext(tk, "AVG"))
		es->filtro_tipo = F_TYPE_AVERAGE;
	else {
		es->filtro_tipo = F_TYPE_CONVOLUTION;
		MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), ',');
		int64_t i;
		es->valores_kernel = MY_MALLOC_NOINIT(es->filtro_size, double);
		for (i = 0; i < es->filtro_size; ++i)
			es->valores_kernel[i] = my_tokenizer_nextFraction(tk2);
		if (my_tokenizer_isNext(tk2, "ABS"))
			es->convol_abs = 1;
		es->convol_max = 256;
		if (my_tokenizer_isNext(tk2, "MAX"))
			es->convol_max = my_tokenizer_nextFraction(tk2);
		my_tokenizer_releaseValidateEnd(tk2);
	}
	if (my_tokenizer_isNext(tk, "CV"))
		es->useOpenCV = 1;
	if (my_tokenizer_isNext(tk, "ADD"))
		es->addToImg = 1;
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}
#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static uchar tra_calcular_filtro(IplImage *imagen, int64_t x, int64_t y, int64_t nc,
		struct Proceso_FILTRO *es) {
	int64_t tipo = es->filtro_tipo, size = es->filtro_size;
	if (tipo == F_TYPE_MEDIAN && es->ventana == NULL)
		es->ventana = MY_MALLOC_NOINIT(size, uchar);
	int64_t i, j, pos = 0;
	uchar min = 255, max = 0;
	for (i = 0; i < es->filtro_width; ++i) {
		int64_t posx = x - es->filtro_width / 2 + i;
		if (posx < 0)
			posx = 0;
		if (posx >= imagen->width)
			posx = imagen->width - 1;
		for (j = 0; j < es->filtro_height; ++j) {
			int64_t posy = y - es->filtro_height / 2 + j;
			if (posy < 0)
				posy = 0;
			if (posy >= imagen->height)
				posy = imagen->height - 1;
			uchar pix = PIXC_8U(imagen, posx, posy, nc);
			switch (tipo) {
			case F_TYPE_MAX:
				if (pix > max)
					max = pix;
				break;
			case F_TYPE_MIN:
				if (pix < min)
					min = pix;
				break;
			case F_TYPE_MEDIAN:
				es->ventana[pos++] = pix;
				break;
			default:
				my_log_error("unsupported\n");
				break;
			}
		}
	}
	switch (tipo) {
	case F_TYPE_MAX:
		return max;
	case F_TYPE_MIN:
		return min;
	case F_TYPE_MEDIAN:
		my_qsort_uint8_array(es->ventana, size);
		return es->ventana[size / 2];
	default:
		return 0;
	}
}
static IplImage *tra_transformar_filtro(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_FILTRO *es = estado;
	if (es->my_convolution_perform == NULL) {
		es->array_dim1 = imagen->nChannels;
		es->array_dim2 = imagen->width;
		es->array_dim3 = imagen->height;
		es->my_convolution_perform = MY_MALLOC_NOINIT(es->array_dim1, double**);
		int64_t i, j;
		for (i = 0; i < es->array_dim1; ++i) {
			es->my_convolution_perform[i] = MY_MALLOC_NOINIT(es->array_dim2, double*);
			for (j = 0; j < es->array_dim2; ++j)
				es->my_convolution_perform[i][j] = MY_MALLOC_NOINIT(es->array_dim3, double);
		}
		es->imgOut = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U,
				imagen->nChannels);
	}
	if (es->useOpenCV) {
		switch (es->filtro_tipo) {
		case F_TYPE_AVERAGE:
			cvSmooth(imagen, es->imgOut, CV_BLUR, es->filtro_width,
					es->filtro_height, 0, 0);
			break;
		case F_TYPE_MEDIAN:
			cvSmooth(imagen, es->imgOut, CV_MEDIAN, es->filtro_width,
					es->filtro_height, 0, 0);
			break;
		default:
			my_log_error("unsupported by opencv\n");
			break;
		}
	} else if (es->filtro_tipo == F_TYPE_AVERAGE
			|| es->filtro_tipo == F_TYPE_CONVOLUTION) {
		if (es->kc == NULL) {
			es->kc = my_convolution_newKernel(es->filtro_width, es->filtro_height);
			if (es->filtro_tipo == F_TYPE_AVERAGE)
				my_convolution_initAverageKernel(es->kc);
			else if (es->filtro_tipo == F_TYPE_CONVOLUTION)
				my_convolution_initKernel(es->kc, es->valores_kernel);
		}
		my_convolution_perform(imagen, es->kc, es->my_convolution_perform);
		my_image_copyArrayToPixels(es->my_convolution_perform, es->convol_max, es->convol_abs,
				es->imgOut);
	} else {
		int64_t x, y, nc;
		for (y = 0; y < imagen->height; ++y) {
			for (x = 0; x < imagen->width; ++x) {
				for (nc = 0; nc < imagen->nChannels; ++nc) {
					PIXC_8U(es->imgOut,x,y,nc) = tra_calcular_filtro(imagen, x,
							y, nc, es);
				}
			}
		}
	}
	if (es->addToImg) {
		int64_t x, y;
		for (y = 0; y < imagen->height; ++y) {
			uchar *ptrIn = (uchar*) (imagen->imageData + imagen->widthStep * y);
			uchar *ptrOut = (uchar*) (es->imgOut->imageData
					+ es->imgOut->widthStep * y);
			for (x = 0; x < imagen->width; ++x) {
				int64_t val = ptrIn[x] + ptrOut[x];
				if (val < 0)
					val = 0;
				if (val > 255)
					val = 255;
				ptrOut[x] = val;
			}
		}
	}
	return es->imgOut;
}
static void tra_release_filtro(void *estado) {
	struct Proceso_FILTRO *es = estado;
	my_image_release(es->imgOut);
	my_convolution_releaseKernel(es->kc);
	MY_FREE(es->ventana);
	int64_t i, j;
	for (i = 0; i < es->array_dim1; ++i) {
		for (j = 0; j < es->array_dim2; ++j)
			MY_FREE(es->my_convolution_perform[i][j]);
		MY_FREE(es->my_convolution_perform[i]);
	}
	MY_FREE(es->my_convolution_perform);
	MY_FREE(es);
}

void tra_reg_filtro() {
	Transform_Def *def =
			newTransformDef("FILTER",
					"WidthxHeight_(MEDIAN|MAX|MIN|AVG|val1,val2,...[,ABS][,MAX,num]])[_CV][_ADD]");
	def->func_new = tra_new_filtro;
	def->func_transform_frame = tra_transformar_filtro;
	def->func_release = tra_release_filtro;
}
#endif
