/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_SHOTS {
	IplImage *imgGraph, *imgFondo;
	double threshold, *valores, valMax;
	MknnDistance *distance;
	MknnDistanceEval *distanceEval;
	MknnDomain *domain;
	Extractor *ex;
	void *desPrev;
	int64_t samplingBase, val_pos, val_largo, val_cant;
};

static void tra_config_shots(const char *trCode, const char *trParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, ',');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct Proceso_SHOTS *es = MY_MALLOC(1, struct Proceso_SHOTS);
	es->samplingBase = my_tokenizer_nextInt(tk);
	es->ex = getExtractor(my_tokenizer_nextToken(tk));
	es->distance = mknn_distance_newPredefined(
			mknn_distanceParams_newParseString(my_tokenizer_nextToken(tk)),
			true);
	es->threshold = my_tokenizer_nextDouble(tk);
	my_tokenizer_releaseValidateEnd(tk);
	es->domain = getMknnDomainDescriptorArray(getDescriptorType(es->ex));
	es->distanceEval = mknn_distance_newDistanceEval(es->distance, es->domain,
			es->domain);
	*out_state = es;
}

static void dibujarGrafico(struct Proceso_SHOTS *es) {
	int64_t maxW = es->imgGraph->width;
	int64_t maxH = es->imgGraph->height;
	int64_t cont, height, width = maxW / es->val_cant;
	double heightVal = es->valMax == 0 ? 0 : maxH / es->valMax;
	cvCopy(es->imgFondo, es->imgGraph, NULL);
	CvPoint ab_izq = cvPoint(0, maxH - 1);
	CvPoint ar_der = cvPoint(width - 1, 0);
	CvScalar colorBajo = cvScalarAll(0);
	CvScalar colorAlto = cvScalarAll(160);
	int64_t i = (es->val_pos - es->val_largo + es->val_cant) % es->val_cant;
	for (cont = 0; cont < es->val_largo; cont++) {
		height = (int64_t) (es->valores[i] * heightVal);
		ar_der.y = ab_izq.y - height;
		cvRectangle(es->imgGraph, ab_izq, ar_der,
				((es->valores[i] < es->threshold) ? colorBajo : colorAlto),
				CV_FILLED, 8, 0);
		ab_izq.x += width;
		ar_der.x += width;
		i = (i + 1) % es->val_cant;
	}
	height = (int64_t) (es->threshold * heightVal);
	if (height >= 0 && height < maxH) {
		CvPoint d = cvPoint(0, maxH - 1 - height);
		CvPoint h = cvPoint(ar_der.x, d.y);
		cvLine(es->imgGraph, d, h, cvScalar(120, 120, 220, 0), 1, 8, 0);
	}
}
static IplImage *tra_transformar_shots(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_SHOTS *es = estado;
	if (es->imgGraph == NULL) {
		es->imgGraph = cvCreateImage(cvSize(imagen->width, imagen->height),
		IPL_DEPTH_8U, 3);
		es->valMax = es->val_largo = es->val_pos = 0;
		es->val_cant = imagen->width / 8;
		es->valores = MY_MALLOC_NOINIT(es->val_cant, double);
		es->imgFondo = cvCreateImage(cvGetSize(es->imgGraph), IPL_DEPTH_8U, 3);
		my_image_fill_white(es->imgFondo);
		cvCopy(es->imgFondo, es->imgGraph, NULL);
	}
	if (numFrame % es->samplingBase != 0)
		return es->imgGraph;
	void *desActual = extractVolatileDescriptor(es->ex, imagen);
	if (es->desPrev == NULL) {
		es->desPrev = cloneDescriptor(getDescriptorType(es->ex), desActual);
		return es->imgGraph;
	}
	double d = mknn_distanceEval_eval(es->distanceEval, es->desPrev, desActual);
	if (d > es->valMax)
		es->valMax = d;
	es->valores[es->val_pos] = d;
	es->val_pos = (es->val_pos + 1) % es->val_cant;
	if (es->val_largo < es->val_cant)
		es->val_largo++;
	dibujarGrafico(es);
	MY_FREE(es->desPrev);
	es->desPrev = cloneDescriptor(getDescriptorType(es->ex), desActual);
	return es->imgGraph;
}

static void tra_release_shots(void *estado) {
	struct Proceso_SHOTS *es = estado;
	mknn_distanceEval_release(es->distanceEval);
	mknn_distance_release(es->distance);
	mknn_domain_release(es->domain);
	my_image_release(es->imgGraph);
	my_image_release(es->imgFondo);
	MY_FREE(es);
}
void tra_reg_shots() {
	Transform_Def *def = newTransformDef("SHOTS",
			"numFramesCada,Descriptor,Distancia,threshold");
	def->func_new = tra_config_shots;
	def->func_transform_frame = tra_transformar_shots;
	def->func_release = tra_release_shots;
}
#endif
