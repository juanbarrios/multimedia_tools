/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_CANNY {
	double threshold1, threshold2;
	int64_t aperture_size;
	IplImage *imgGris, *imgCanny;
};

static void tra_config_canny(const char *trCode, const char *trParameters, void **out_state) {
	struct Proceso_CANNY *es = MY_MALLOC(1, struct Proceso_CANNY);
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	es->threshold1 = my_tokenizer_nextDouble(tk);
	es->threshold2 = my_tokenizer_nextDouble(tk);
	es->aperture_size = my_tokenizer_nextInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}

static IplImage *tra_transformar_canny(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_CANNY *es = estado;
	if (es->imgGris == NULL) {
		es->imgGris = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgCanny = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
	}
	IplImage *entrada = imagen;
	if (imagen->nChannels > 1) {
		cvCvtColor(imagen, es->imgGris, CV_BGR2GRAY);
		entrada = es->imgGris;
	}
	cvCanny(entrada, es->imgCanny, es->threshold1, es->threshold2,
			es->aperture_size);
	return es->imgCanny;
}
static void tra_release_canny(void *estado) {
	struct Proceso_CANNY *es = estado;
	my_image_release(es->imgGris);
	my_image_release(es->imgCanny);
	MY_FREE(es);
}

void tra_reg_canny() {
	Transform_Def *def = newTransformDef("CANNY",
			"threshold1_threshold2_apertureSize");
	def->func_new = tra_config_canny;
	def->func_transform_frame = tra_transformar_canny;
	def->func_release = tra_release_canny;
}
#endif
