/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Template {
	int64_t sizeLeft, sizeUp, sizeRight, sizeDown, numPixels;
	uchar **matrix1, **matrix2, **matrix3;
	double **weigth;
};
struct PR_Templ {
	double threshold;
	char *nameTemplate;
	IplImage *imgGris, *imgBin;
	MyVectorObj *templates;
};

#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static struct Template *loadTemplate(IplImage *img, IplImage *mask) {
	struct Template *templ = MY_MALLOC(1, struct Template);
	templ->sizeLeft = img->width / 2;
	templ->sizeRight = img->width - templ->sizeLeft - 1;
	templ->sizeUp = img->height / 2;
	templ->sizeDown = img->height - templ->sizeUp - 1;
	templ->numPixels = img->width * img->height;
	templ->matrix1 = MY_MALLOC_MATRIX(img->width, img->height, uchar);
	templ->matrix2 = MY_MALLOC_MATRIX(img->width, img->height, uchar);
	templ->matrix3 = MY_MALLOC_MATRIX(img->width, img->height, uchar);
	templ->weigth = MY_MALLOC_MATRIX(img->width, img->height, double);
	int64_t i, j;
	for (j = 0; j < img->height; ++j) {
		for (i = 0; i < img->width; ++i) {
			templ->matrix1[i][j] = PIXC_8U(img, i, j, 0);
			templ->matrix2[i][j] = PIXC_8U(img, i, j, 1);
			templ->matrix3[i][j] = PIXC_8U(img, i, j, 2);
			templ->weigth[i][j] = PIXC_8U(mask, i, j, 0) > 120 ? 1 : 0;
		}
	}
	return templ;
}
static void loadTemplates(const char *filename, struct PR_Templ *es) {
	IplImage *imgTempl = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	my_assert_notNull("img", imgTempl);
	char *fm = my_newString_concat(filename, ".mask.png");
	IplImage *imgMask = cvLoadImage(fm, CV_LOAD_IMAGE_GRAYSCALE);
	my_assert_notNull("mask", imgMask);
	MY_FREE(fm);
	my_assert_equalInt("width", imgTempl->width, imgMask->width);
	my_assert_equalInt("height", imgTempl->height, imgMask->height);
	es->templates = my_vectorObj_new();
	int64_t maxW = 100, maxH = 50, minW = 10, minH = 10;
	double fctIni = MIN(maxW / (double )imgTempl->width,
			maxH / (double )imgTempl->height);
	double fctFin = MAX(minW / (double )imgTempl->width,
			minH / (double )imgTempl->height);
	double fct = fctIni;
	while (fct >= fctFin) {
		CvSize newSize = cvSize(my_math_round_int(imgTempl->width * fct),
				my_math_round_int(imgTempl->height * fct));
		IplImage *img = cvCreateImage(newSize, imgTempl->depth,
				imgTempl->nChannels);
		IplImage *mask = cvCreateImage(newSize, imgMask->depth,
				imgMask->nChannels);
		cvResize(imgTempl, img, CV_INTER_AREA);
		cvResize(imgMask, mask, CV_INTER_AREA);
		struct Template *templ = loadTemplate(img, mask);
		my_vectorObj_add(es->templates, templ);
		my_image_release(img);
		my_image_release(mask);
		fct *= 0.5;
	}
	my_image_release(imgTempl);
	my_image_release(imgMask);
}
static void tra_config_templ(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct PR_Templ *es = MY_MALLOC(1, struct PR_Templ);
	es->threshold = my_tokenizer_nextFraction(tk);
	loadTemplates(my_tokenizer_getCurrentTail(tk), es);
	my_tokenizer_release(tk);
	*out_state = es;
}
static double testTemplate(IplImage *img, CvPoint center, struct Template *tpl) {
	CvPoint from = cvPoint(center.x - tpl->sizeLeft, center.y - tpl->sizeUp);
	CvPoint to = cvPoint(center.x + tpl->sizeRight, center.y + tpl->sizeDown);
	if (from.x < 0 || from.x >= img->width || from.y < 0
			|| from.y >= img->height)
		return DBL_MAX;
	double dist = 0;
	int64_t x, y, tplx, tply, cont = 0;
	for (y = from.y, tply = 0; y < to.y; ++y, ++tply) {
		if (y < 0 || y >= img->height)
			continue;
		for (x = from.x, tplx = 0; x < to.x; ++x, ++tplx) {
			if (x < 0 || x >= img->width)
				continue;
			int64_t d1 = (int64_t) PIXC_8U(img, x, y, 0)
					- (int64_t) tpl->matrix1[tplx][tply];
			int64_t d2 = (int64_t) PIXC_8U(img, x, y, 1)
					- (int64_t) tpl->matrix2[tplx][tply];
			int64_t d3 = (int64_t) PIXC_8U(img, x, y, 2)
					- (int64_t) tpl->matrix3[tplx][tply];
			double diff = sqrt(d1 * d1 + d2 * d2 + d3 * d3);
			dist += diff * tpl->weigth[tplx][tply];
			cont++;
		}
	}
	dist /= cont;
	return dist;
}
IplImage *tra_transformar_templ(IplImage *img, int64_t numFrame, void* estado) {
	struct PR_Templ *es = estado;
	if (es->imgGris == NULL) {
		es->imgGris = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
		es->imgBin = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	}
	if (img->nChannels > 1) {
		cvCvtColor(img, es->imgGris, CV_BGR2GRAY);
	} else {
		cvCopy(img, es->imgGris, NULL);
	}
	cvSetZero(es->imgBin);
	my_log_info("%ix%i\n", es->imgGris->height, es->imgGris->width);
	int64_t x, y;
	for (y = 0; y < es->imgGris->height; y += 3) {
		for (x = 0; x < es->imgGris->width; x += 3) {
			CvPoint center = cvPoint(x, y);
			double minDist = DBL_MAX;
			int64_t i;
			struct Template *minTempl = NULL;
			for (i = 0; i < my_vectorObj_size(es->templates); ++i) {
				struct Template *templ = my_vectorObj_get(es->templates, i);
				double dist = testTemplate(es->imgGris, center, templ);
				if (dist < minDist) {
					minDist = dist;
					minTempl = templ;
				}
			}
			if (minTempl != NULL) {
				my_log_info("%"PRIi64",%"PRIi64" %lf\n", x, y, minDist);
				if (minDist < es->threshold) {
					cvCircle(es->imgBin, center, 2, cvScalarAll(255), 2,
					CV_FILLED, 0);
				}
			}
		}
	}
	return es->imgBin;
}
void tra_release_templ(void *estado) {
	struct PR_Templ *es = estado;
	my_image_release(es->imgGris);
	my_image_release(es->imgBin);
	MY_FREE(es);
}
void tra_reg_templ() {
	Transform_Def *def = newTransformDef("TEMPLATE", "threshold_templateFile");
	def->func_new = tra_config_templ;
	def->func_transform_frame = tra_transformar_templ;
	def->func_release = tra_release_templ;
}
#endif
