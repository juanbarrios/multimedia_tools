/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_PIPSEL {
	uchar adentro;
	IplImage *imgZona, *imgAmpliada;
};

void tra_config_pipsel(const char *trCode, const char *trParameters, void **out_state) {
	struct Proceso_PIPSEL *es = MY_MALLOC(1, struct Proceso_PIPSEL);
	if (my_string_equals(trParameters, "IN"))
		es->adentro = 1;
	else if (my_string_equals(trParameters, "OUT"))
		es->adentro = 0;
	else
		my_log_error("error in %s\n", trParameters);
	*out_state = es;
}
IplImage *tra_transformar_pipsel(IplImage *imagen, int64_t numFrame, void* estado) {
	struct Proceso_PIPSEL *es = estado;
	if (es->imgZona == NULL) {
		//debe haber un cvSetImageROI
		CvRect rect = cvGetImageROI(imagen);
		if (rect.width == imagen->width && rect.height == imagen->height)
			return imagen;
		es->imgZona = cvCreateImage(cvSize(rect.width, rect.height),
				imagen->depth, imagen->nChannels);
		if (!es->adentro) {
			cvSetZero(es->imgZona);
		} else {
			double factor = 240.0 / rect.height;
			CvSize ampliado = cvSize(my_math_round_int(rect.width * factor),
					my_math_round_int(rect.height * factor));
			es->imgAmpliada = cvCreateImage(ampliado, imagen->depth,
					imagen->nChannels);
		}
	}
	if (es->adentro) {
		cvCopy(imagen, es->imgZona, NULL);
		cvResetImageROI(imagen);
		if (es->imgAmpliada != NULL) {
			cvResize(es->imgZona, es->imgAmpliada, CV_INTER_CUBIC);
			return es->imgAmpliada;
		} else
			return es->imgZona;
	} else {
		cvCopy(es->imgZona, imagen, NULL);
		cvResetImageROI(imagen);
		return imagen;
	}
}
void tra_release_pipsel(void *estado) {
	struct Proceso_PIPSEL *es = estado;
	my_image_release(es->imgZona);
	MY_FREE(es);
}
void tra_reg_pipsel() {
	Transform_Def *def = newTransformDef("PSEL", "(IN|OUT)");
	def->func_new = tra_config_pipsel;
	def->func_transform_frame = tra_transformar_pipsel;
	def->func_release = tra_release_pipsel;
}
#endif
