/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_CROP {
	uchar autoCrop, cropStatic, autoCenter;
	IplImage *imgZona;
	CvRect zone;
};

static void tra_config_crop(const char *trCode, const char *trParameters, void **out_state) {
	struct Proceso_CROP *es = MY_MALLOC(1, struct Proceso_CROP);
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	if (my_tokenizer_isNext(tk, "AUTO")) {
		es->autoCrop = 1;
		if (my_tokenizer_isNext(tk, "BLACK"))
			es->cropStatic = 0;
		else if (my_tokenizer_isNext(tk, "STATIC"))
			es->cropStatic = 1;
		else
			my_log_error("unknown %s\n", my_tokenizer_nextToken(tk));
	} else {
		MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
		es->zone.width = my_tokenizer_nextInt(tk2);
		es->zone.height = my_tokenizer_nextInt(tk2);
		my_tokenizer_release(tk2);
		if (my_tokenizer_isNext(tk, "CENTER")) {
			es->autoCenter = 1;
		} else {
			es->zone.x = my_tokenizer_nextInt(tk);
			es->zone.y = my_tokenizer_nextInt(tk);
		}
	}
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}
struct Crop_Rectangulo {
	int64_t asignado, x, y, w, h;
};

#define CROP_NO(x,y) (sigmas2[(x)][(y)] > 100 || (!cropStatic && mus[(x)][(y)] > 20))
static void tra_zonaConMovimiento(double **mus, double **sigmas2, int64_t width,
		int64_t height, uchar cropStatic, struct Crop_Rectangulo *out_rectangulo) {
	int64_t MAXS_NO_X = width / 8, MAXS_NO_Y = height / 8;
	int64_t x, y, contNo = 0, continuar = 1;
	int64_t xTL = 0, yTL = 0, xBR = width - 1, yBR = height - 1;
	for (y = 0; y < height && continuar; ++y) {
		contNo = 0;
		for (x = 0; x < width && continuar; ++x)
			if (CROP_NO(x, y)) {
				contNo++;
				if (contNo > MAXS_NO_X) {
					yTL = y;
					continuar = 0;
				}
			}
	}
	continuar = 1;
	for (x = 0; x < width && continuar; ++x) {
		contNo = 0;
		for (y = 0; y < height && continuar; ++y)
			if (CROP_NO(x, y)) {
				contNo++;
				if (contNo > MAXS_NO_Y) {
					xTL = x;
					continuar = 0;
				}
			}
	}
	continuar = 1;
	for (y = height - 1; y >= 0 && continuar; --y) {
		contNo = 0;
		for (x = 0; x < width && continuar; ++x)
			if (CROP_NO(x, y)) {
				contNo++;
				if (contNo > MAXS_NO_X) {
					yBR = y;
					continuar = 0;
				}
			}
	}
	continuar = 1;
	for (x = width - 1; x >= 0 && continuar; --x) {
		contNo = 0;
		for (y = 0; y < height && continuar; ++y)
			if (CROP_NO(x, y)) {
				contNo++;
				if (contNo > MAXS_NO_Y) {
					xBR = x;
					continuar = 0;
				}
			}
	}
	if (xTL == 0 && yTL == 0 && xBR == width - 1 && yBR == height - 1) {
		out_rectangulo->asignado = 0;
	} else {
		out_rectangulo->asignado = 1;
		out_rectangulo->x = xTL;
		out_rectangulo->y = yTL;
		out_rectangulo->w = xBR - xTL + 1;
		out_rectangulo->h = yBR - yTL + 1;
	}
}
#define SHOW_DEBUG 0
static uchar tra_preprocesar_compute_crop(VideoFrame *video_frame,
		FileDB *fileDB, void* estado, char **out_stateToSave) {
	struct Proceso_CROP *es = estado;
	if (!es->autoCrop)
		return 1;
	IplImage *frame = NULL;
	int64_t x, y, cont = 0;
	uchar *ptr1;
	double delta, **mus = NULL, **sigmas2 = NULL;
	while (loadNextFrame(video_frame)) {
		frame = getCurrentFrameGray(video_frame);
		if (sigmas2 == NULL) {
			mus = MY_MALLOC_MATRIX(frame->width, frame->height, double);
			sigmas2 = MY_MALLOC_MATRIX(frame->width, frame->height, double);
		}
		cont++;
		for (y = 0; y < frame->height; ++y) {
			ptr1 = (uchar*) (frame->imageData + frame->widthStep * y);
			for (x = 0; x < frame->width; ++x) {
				delta = ptr1[x] - mus[x][y];
				mus[x][y] += delta / cont;
				sigmas2[x][y] += delta * (ptr1[x] - mus[x][y]);
			}
		}
	}
	for (y = 0; y < frame->height; ++y)
		for (x = 0; x < frame->width; ++x)
			sigmas2[x][y] = sigmas2[x][y] / cont;
	struct Crop_Rectangulo rect = { 0 };
	tra_zonaConMovimiento(mus, sigmas2, frame->width, frame->height,
			es->cropStatic, &rect);
	if (SHOW_DEBUG) {
		if (rect.asignado) {
			IplImage *crop = cvCreateImage(cvGetSize(frame), 8, 3);
			cvSetZero(crop);
			cvRectangle(crop, cvPoint(rect.x, rect.y),
					cvPoint(rect.x + rect.w - 1, rect.y + rect.h - 1),
					cvScalarAll(255), 1, 8, 0);
			cvNamedWindow("crop", CV_WINDOW_AUTOSIZE);
			cvShowImage("crop", crop);
		}
		cvNamedWindow("mus", CV_WINDOW_AUTOSIZE);
		cvShowImage("mus",
				my_image_newFromArray(mus, frame->width, frame->height, 255));
		cvNamedWindow("sigmas", CV_WINDOW_AUTOSIZE);
		cvShowImage("sigmas",
				my_image_newFromArray(sigmas2, frame->width, frame->height, 0));
		//cvSaveImage("medianas.png", imgMeds, NULL);
		cvWaitKey(0);
		//cvDestroyWindow("MAXIMOS");
	}
	MY_FREE_MATRIX(mus, frame->width);
	MY_FREE_MATRIX(sigmas2, frame->width);
	if (rect.asignado) {
		my_log_info("crop %s: (%"PRIi64",%"PRIi64") w=%"PRIi64" h=%"PRIi64"\n",
				getVideoName(video_frame), rect.x, rect.y, rect.w, rect.h);
		if (rect.w < 64 || rect.h < 64)
			my_log_info(
					"crop %s: la zona recortada es muy chica %"PRIi64"x%"PRIi64" (video negro?)\n",
					getVideoName(video_frame), rect.w, rect.h);
		*out_stateToSave = my_newString_format("%"PRIi64"-%"PRIi64"-%"PRIi64"-%"PRIi64"",
				rect.x, rect.y, rect.w, rect.h);
		return 1;
	} else {
		my_log_info("crop %s: sin zona\n", getVideoName(video_frame));
		return 0;
	}
}
static void tra_preprocesar_load_crop(void* estado, const char *savedState) {
	struct Proceso_CROP *es = estado;
	if (!es->autoCrop)
		return;
	MyTokenizer *tk = my_tokenizer_new(savedState, '-');
	es->zone.x = my_tokenizer_nextInt(tk);
	es->zone.y = my_tokenizer_nextInt(tk);
	es->zone.width = my_tokenizer_nextInt(tk);
	es->zone.height = my_tokenizer_nextInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
}
static IplImage *tra_transformar_crop(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_CROP *es = estado;
	if (es->imgZona == NULL) {
		if (es->autoCenter) {
			if (imagen->width > es->zone.width)
				es->zone.x = (imagen->width - es->zone.width) / 2;
			if (imagen->height > es->zone.height)
				es->zone.y = (imagen->height - es->zone.height) / 2;
		}
		es->imgZona = cvCreateImage(cvSize(es->zone.width, es->zone.height),
				imagen->depth, imagen->nChannels);
	}
	cvSetImageROI(imagen, es->zone);
	cvCopy(imagen, es->imgZona, NULL);
	cvResetImageROI(imagen);
	return es->imgZona;
}
static void tra_release_crop(void *estado) {
	struct Proceso_CROP *es = estado;
	my_image_release(es->imgZona);
	MY_FREE(es);
}
void tra_reg_crop() {
	Transform_Def *def = newTransformDef("CROP",
			"[WidthxHeight_[CENTER|x_y]|AUTO_[BLACK|STATIC]]");
	def->func_new = tra_config_crop;
	def->func_preprocess_compute = tra_preprocesar_compute_crop;
	def->func_preprocess_load = tra_preprocesar_load_crop;
	def->func_transform_frame = tra_transformar_crop;
	def->func_release = tra_release_crop;
}
#endif
