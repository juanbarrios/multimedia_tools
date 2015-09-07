/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct ImgsSegmento {
	IplImage *frameSel, *frameDesde, *frameHasta;
};

struct Proceso_VerFs {
	const struct Segmentation *seg;
	char *segmentacion_dir;
	LoadSegmentation *loader;
	struct ImgsSegmento **imgsSegmento;
	int64_t posImg;
	int64_t posFrame;
	IplImage *imagenSegmentos, *imagenFinal;
	CvScalar colorFondo;
	uchar hasInit;
};

static void tra_config_vseg(const char *trCode, const char *trParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, ',');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct Proceso_VerFs *es = MY_MALLOC(1, struct Proceso_VerFs);
	es->segmentacion_dir = my_newString_string(my_tokenizer_getCurrentTail(tk));
	es->colorFondo = cvScalar(200, 200, 200, 0);
	my_tokenizer_release(tk);
	*out_state = es;
}

static uchar tra_preprocesar_compute_vseg(VideoFrame *video_frame,
		FileDB *fileDB, void* estado, char **out_stateToSave) {
	struct Proceso_VerFs *es = estado;
	if (es->loader == NULL) {
		es->loader = newLoadSegmentation(fileDB->db, es->segmentacion_dir);
	}
	es->seg = loadSegmentationFileDB(es->loader, fileDB);
	for (int64_t i = 0; i < es->seg->num_segments; ++i) {
		my_log_info(
				"Seg %3"PRIi64") %4"PRIi64" - %4"PRIi64" - %4"PRIi64"  (%3"PRIi64" frames)  %4.1lf - %4.1lf - %4.1lf (%4.1lf segs)\n",
				es->seg->segments[i].num_segment,
				es->seg->segments[i].start_frame,
				es->seg->segments[i].selected_frame,
				es->seg->segments[i].end_frame,
				es->seg->segments[i].end_frame
						- es->seg->segments[i].start_frame + 1,
				es->seg->segments[i].start_second,
				es->seg->segments[i].selected_second,
				es->seg->segments[i].end_second,
				es->seg->segments[i].end_second
						- es->seg->segments[i].start_second);
	}
	return 1;
}

#define NUM_PREVIOS 9

static void tra_init_vseg(IplImage *imagen, struct Proceso_VerFs *es) {
	es->imgsSegmento = MY_MALLOC_NOINIT(NUM_PREVIOS, struct ImgsSegmento*);
	int64_t i;
	CvSize size = cvGetSize(imagen);
	for (i = 0; i < NUM_PREVIOS; ++i) {
		es->imgsSegmento[i] = MY_MALLOC(1, struct ImgsSegmento);
		es->imgsSegmento[i]->frameDesde = cvCreateImage(size, IPL_DEPTH_8U, 3);
		es->imgsSegmento[i]->frameSel = cvCreateImage(size, IPL_DEPTH_8U, 3);
		es->imgsSegmento[i]->frameHasta = cvCreateImage(size, IPL_DEPTH_8U, 3);
		my_image_fill_color(es->imgsSegmento[i]->frameDesde, es->colorFondo);
		my_image_fill_color(es->imgsSegmento[i]->frameSel, es->colorFondo);
		my_image_fill_color(es->imgsSegmento[i]->frameHasta, es->colorFondo);
	}
	es->posImg = 0;
	int64_t w = 3 * imagen->width;
	int64_t h = NUM_PREVIOS * imagen->height;
	es->imagenSegmentos = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	es->imagenFinal = cvCreateImage(cvSize(w / 3, h / 3), IPL_DEPTH_8U, 3);
	my_image_fill_white(es->imagenSegmentos);
	my_image_fill_white(es->imagenFinal);
}

static IplImage *tra_transformar_vseg(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_VerFs *es = estado;
	if (!es->hasInit) {
		tra_init_vseg(imagen, es);
		es->hasInit = 1;
	}
	struct VideoSegment *f = NULL;
	for (;;) {
		if (es->posFrame >= es->seg->num_segments)
			return es->imagenFinal;
		f = es->seg->segments + es->posFrame;
		if (numFrame < f->start_frame)
			return es->imagenFinal;
		if (f->start_frame <= numFrame && numFrame <= f->end_frame)
			break;
		es->posFrame++;
	}
	if (numFrame != f->start_frame && numFrame != f->selected_frame
			&& numFrame != f->end_frame) {
		return es->imagenFinal;
	}
	if (numFrame == f->start_frame) {
		es->posImg = (es->posImg + 1) % NUM_PREVIOS;
		cvCopy(imagen, es->imgsSegmento[es->posImg]->frameDesde, NULL);
		my_image_fill_color(es->imgsSegmento[es->posImg]->frameSel,
				es->colorFondo);
		my_image_fill_color(es->imgsSegmento[es->posImg]->frameHasta,
				es->colorFondo);
		my_log_info("Seg %"PRIi64") desde=%4"PRIi64"", f->num_segment,
				numFrame);
	}
	if (numFrame == f->selected_frame) {
		cvCopy(imagen, es->imgsSegmento[es->posImg]->frameSel, NULL);
		my_log_info(" selec=%4"PRIi64"", numFrame);
	}
	if (numFrame == f->end_frame) {
		cvCopy(imagen, es->imgsSegmento[es->posImg]->frameHasta, NULL);
		my_log_info(" hasta=%4"PRIi64"  (%3"PRIi64" frames)\n", numFrame,
				f->end_frame - f->start_frame + 1);
	}
	int64_t i, pos = es->posImg;
	int64_t w = imagen->width, h = imagen->height;
	for (i = 0; i < NUM_PREVIOS; ++i) {
		struct ImgsSegmento *is = es->imgsSegmento[pos];
		my_image_copyPixels(is->frameDesde, es->imagenSegmentos, 0, i * h);
		my_image_copyPixels(is->frameSel, es->imagenSegmentos, w, i * h);
		my_image_copyPixels(is->frameHasta, es->imagenSegmentos, 2 * w, i * h);
		pos = (pos + NUM_PREVIOS - 1) % NUM_PREVIOS;
	}
	cvResize(es->imagenSegmentos, es->imagenFinal, CV_INTER_CUBIC);
	return es->imagenFinal;
}
static void tra_release_vseg(void *estado) {
	struct Proceso_VerFs *es = estado;
	my_image_release(es->imagenSegmentos);
	releaseLoadSegmentation(es->loader);
	MY_FREE(es->segmentacion_dir);
	MY_FREE(es);
}
void tra_reg_vseg() {
	Transform_Def *def = newTransformDef("VERSEGM", "samplingDir");
	def->func_new = tra_config_vseg;
	def->func_preprocess_compute = tra_preprocesar_compute_vseg;
	def->func_transform_frame = tra_transformar_vseg;
	def->func_release = tra_release_vseg;
}
#endif
