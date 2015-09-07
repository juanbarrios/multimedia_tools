/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_KF {
	Extractor *ex;
	IplImage *img_gray, *img_scaled;
	double *descriptor;
	double factor;
	int64_t img_width, img_height;
};

static void tra_config_kf(const char *trCode, const char *trParameters,
		void **out_state) {
	struct Proceso_KF *es = MY_MALLOC(1, struct Proceso_KF);
	es->ex = getExtractor2(trCode, trParameters);
	DescriptorType td = getDescriptorType(es->ex);
	if (my_string_equals("KF", trCode)) {
		es->factor = 1;
	} else if (my_string_equals("OMD", trCode)) {
		es->factor = 255.0 / (td.array_length - 1);
	}
	my_assert_equalInt("length", es->img_width * es->img_height,
			td.array_length);
	es->descriptor = MY_MALLOC(td.array_length, double);
	es->img_gray = cvCreateImage(cvSize(es->img_width, es->img_height),
	IPL_DEPTH_8U, 1);
	*out_state = es;
}

#define SCALE_KF 1

static IplImage *tra_transformar_kft(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_KF *es = estado;
	void *desc = extractVolatileDescriptor(es->ex, imagen);
	DescriptorType td = getDescriptorType(es->ex);
	descriptor2doubleArray(td, desc, es->descriptor);
	int64_t pos = 0;
	for (int64_t y = 0; y < es->img_height; ++y) {
		uchar *ptr1 = (uchar*) (es->img_gray->imageData
				+ es->img_gray->widthStep * y);
		for (int64_t x = 0; x < es->img_width; ++x) {
			ptr1[x] = my_math_round_int(es->factor * es->descriptor[pos]);
			pos++;
		}
	}
	if (SCALE_KF) {
		if (es->img_scaled == NULL)
			es->img_scaled = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		cvResize(es->img_gray, es->img_scaled, CV_INTER_NN);
		return es->img_scaled;
	}
	return es->img_gray;
}
static void tra_release_kf(void *estado) {
	struct Proceso_KF *es = estado;
	my_image_release(es->img_gray);
	my_image_release(es->img_scaled);
	releaseExtractor(es->ex);
	MY_FREE(es->descriptor);
	MY_FREE(es);
}
static void reg(char *code) {
	const char *help = getExtractorHelp(code);
	Transform_Def *def = newTransformDef(code, help);
	def->func_new = tra_config_kf;
	def->func_transform_frame = tra_transformar_kft;
	def->func_release = tra_release_kf;
}
void tra_reg_kf() {
	reg("KF");
	reg("OMD");
}
#endif
