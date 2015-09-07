/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_LAPLACE {
	int64_t aperture_size;
	IplImage *imgGris, *imgGris16bits;
};

static void tra_config_laplace(const char *trCode, const char *trParameters,
		void **out_state) {
	struct Proceso_LAPLACE *es = MY_MALLOC(1, struct Proceso_LAPLACE);
	es->aperture_size = my_parse_int(trParameters);
	*out_state = es;
}
static IplImage *tra_transformar_laplace(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_LAPLACE *es = estado;
	if (es->imgGris == NULL) {
		es->imgGris = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgGris16bits = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_16S, 1);
	}
	if (imagen->nChannels > 1) {
		cvCvtColor(imagen, es->imgGris, CV_BGR2GRAY);
	} else {
		cvCopy(imagen, es->imgGris, NULL);
	}
	cvLaplace(es->imgGris, es->imgGris16bits, es->aperture_size);
	cvConvertScale(es->imgGris16bits, es->imgGris, 1, 0);
	cvCvtColor(es->imgGris, imagen, CV_GRAY2BGR);
	return imagen;
}
static void tra_release_laplace(void *estado) {
	struct Proceso_LAPLACE *es = estado;
	my_image_release(es->imgGris);
	my_image_release(es->imgGris16bits);
	MY_FREE(es);
}
void tra_reg_laplace() {
	Transform_Def *def = newTransformDef("LAPLACE", "apertureSize");
	def->func_new = tra_config_laplace;
	def->func_transform_frame = tra_transformar_laplace;
	def->func_release = tra_release_laplace;
}
#endif
