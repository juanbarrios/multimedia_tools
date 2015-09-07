/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_DOG {
	double variance1, variance2, threshold;
	IplImage *imgGray, *imgBuff1, *imgBuff2, *imgDiff;
};

void tra_config_dog(const char *trCode, const char *trParameters,
		void **out_state) {
	struct Proceso_DOG *es = MY_MALLOC(1, struct Proceso_DOG);
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	es->variance1 = my_tokenizer_nextDouble(tk);
	es->variance2 = my_tokenizer_nextDouble(tk);
	if (my_tokenizer_hasNext(tk))
		es->threshold = my_tokenizer_nextDouble(tk);
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}

IplImage *tra_transformar_dog(IplImage *imagen, int64_t numFrame, void* estado) {
	struct Proceso_DOG *es = estado;
	if (es->imgGray == NULL) {
		es->imgGray = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgBuff1 = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgBuff2 = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U, 1);
		es->imgDiff = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_16S, 1);
	}
	if (imagen->nChannels > 1) {
		cvCvtColor(imagen, es->imgGray, CV_BGR2GRAY);
	} else {
		cvCopy(imagen, es->imgGray, NULL);
	}
	cvSmooth(es->imgGray, es->imgBuff1, CV_GAUSSIAN, es->variance1,
			es->variance1, 0, 0);
	cvSmooth(es->imgGray, es->imgBuff2, CV_GAUSSIAN, es->variance2,
			es->variance2, 0, 0);
	cvSub(es->imgBuff1, es->imgBuff2, es->imgDiff, NULL);
	if (es->threshold > 0) {
		cvThreshold(es->imgDiff, es->imgGray, es->threshold, 255,
				CV_THRESH_BINARY);
	} else {
		//cvConvertScaleAbs(es->imgDiff, es->imgGray, 1, 0);
		double max_val = 0;
		cvMinMaxLoc(es->imgDiff, NULL, &max_val, NULL, NULL, NULL);
		cvConvertScaleAbs(es->imgDiff, es->imgGray,
				(max_val == 0) ? 0 : 255.0 / max_val, 0);
	}
	return es->imgGray;
}
void tra_release_dog(void *estado) {
	struct Proceso_DOG *es = estado;
	my_image_release(es->imgGray);
	my_image_release(es->imgBuff1);
	my_image_release(es->imgBuff2);
	my_image_release(es->imgDiff);
	MY_FREE(es);
}
void tra_reg_dog() {
	Transform_Def *def = newTransformDef("DOG", "var1_var2_threshold");
	def->func_new = tra_config_dog;
	def->func_transform_frame = tra_transformar_dog;
	def->func_release = tra_release_dog;
}
#endif
