/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_SOBEL {
	int64_t xorder, yorder;
	uchar esAprox;
	double threshold;
	IplImage *imgGris, *imgSobelX, *imgSobelY;
};

static void tra_config_sobel(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct Proceso_SOBEL *es = MY_MALLOC(1, struct Proceso_SOBEL);
	es->xorder = my_tokenizer_nextInt(tk);
	es->yorder = my_tokenizer_nextInt(tk);
	if (my_tokenizer_hasNext(tk))
		es->threshold = my_tokenizer_nextFraction(tk);
	if (my_tokenizer_isNext(tk, "APROX"))
		es->esAprox = 1;
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}

static IplImage *tra_transformar_sobel(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_SOBEL *es = estado;
	if (es->imgGris == NULL) {
		es->imgGris = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgSobelX = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_16S, 1);
		es->imgSobelY = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_16S, 1);
	}
	if (imagen->nChannels > 1) {
		cvCvtColor(imagen, es->imgGris, CV_BGR2GRAY);
	} else {
		cvCopy(imagen, es->imgGris, NULL);
	}
	if (es->xorder > 0)
		cvSobel(es->imgGris, es->imgSobelX, es->xorder, 0, 3);
	if (es->yorder > 0)
		cvSobel(es->imgGris, es->imgSobelY, 0, es->yorder, 3);
#if 0
	double min = DBL_MAX, max = -DBL_MAX;
#endif
	int64_t x, y;
	for (y = 0; y < imagen->height; ++y) {
		short *ptrX = (short*) (es->imgSobelX->imageData
				+ es->imgSobelX->widthStep * y);
		short *ptrY = (short*) (es->imgSobelY->imageData
				+ es->imgSobelY->widthStep * y);
		uchar *ptr = (uchar*) (es->imgGris->imageData
				+ es->imgGris->widthStep * y);
		for (x = 0; x < imagen->width; ++x) {
			double val;
			if (es->xorder == 0) {
				val = MY_ABS_SHORT(ptrY[x]);
			} else if (es->yorder == 0) {
				val = MY_ABS_SHORT(ptrX[x]);
			} else if (es->esAprox) {
				val = MY_ABS_SHORT(ptrX[x]) + MY_ABS_SHORT(ptrY[x]);
			} else {
				val = sqrt(
						((double) ptrX[x]) * ptrX[x]
								+ ((double) ptrY[x]) * ptrY[x]);
			}
			if (es->threshold == 0)
				ptr[x] = my_math_round_uint8(val);
			else
				ptr[x] = (val >= es->threshold) ? 255 : 0;
#if 0
			//veo la direccion del gradiente
			if (val >= es->threshold) {
				val = ptrX[x] == 0 ? (ptrY[x] < 0 ? -M_PI_2 : M_PI_2) : atan(
						ptrY[x] / ptrX[x]);
				if (val < min)
				min = val;
				if (val > max)
				max = val;
			}
#endif
		}
	}
#if 0
	min += M_PI_2;
	max += M_PI_2;
	if (min < 0 || min > M_PI)
	log_info("min=%lf\n", min);
	if (max < 0 || max > M_PI)
	log_info("max=%lf\n", max);
#endif
	return es->imgGris;
}
static void tra_release_sobel(void *estado) {
	struct Proceso_SOBEL *es = estado;
	my_image_release(es->imgGris);
	my_image_release(es->imgSobelX);
	my_image_release(es->imgSobelY);
	MY_FREE(es);
}
void tra_reg_sobel() {
	Transform_Def *def = newTransformDef("SOBEL",
			"xorder_yorder_threshold[_APROX]");
	def->func_new = tra_config_sobel;
	def->func_transform_frame = tra_transformar_sobel;
	def->func_release = tra_release_sobel;
}
#endif
