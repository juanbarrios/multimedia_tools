/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct State_GRAY {
	IplImage *imgGray;
	MyVectorObj *operations;
};

#define OP_THRESHOLD 1
#define OP_EQUALIZE 2
#define OP_ERODE 3
#define OP_DILATE 4
#define OP_PIXEL 5

struct Operation {
	int64_t operation;
	double threshold;
	IplConvKernel* kernel;
	double contrast, brightness;
	uchar isAbs;
};

static void tra_new_gray(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct State_GRAY *es = MY_MALLOC(1, struct State_GRAY);
	es->operations = my_vectorObj_new();
	struct Operation *opPixel = NULL;
	while (my_tokenizer_hasNext(tk)) {
		const char *t = my_tokenizer_nextToken(tk);
		if (t[0] == 'c' || t[0] == 'C') {
			if (opPixel == NULL) {
				opPixel = MY_MALLOC(1, struct Operation);
				opPixel->operation = OP_PIXEL;
				my_vectorObj_add(es->operations, opPixel);
			}
			opPixel->contrast = my_parse_fraction(t + 1);
		} else if (t[0] == 'b' || t[0] == 'B') {
			if (opPixel == NULL) {
				opPixel = MY_MALLOC(1, struct Operation);
				opPixel->operation = OP_PIXEL;
				my_vectorObj_add(es->operations, opPixel);
			}
			opPixel->brightness = my_parse_fraction(t + 1);
		} else if (my_string_equals_ignorecase(t, "ABS")) {
			if (opPixel == NULL) {
				opPixel = MY_MALLOC(1, struct Operation);
				opPixel->operation = OP_PIXEL;
				my_vectorObj_add(es->operations, opPixel);
			}
			opPixel->isAbs = my_parse_fraction(t + 1);
		} else if (t[0] == 't' || t[0] == 'T') {
			struct Operation *op = MY_MALLOC(1, struct Operation);
			op->operation = OP_THRESHOLD;
			op->threshold = my_parse_fraction(t + 1);
			my_vectorObj_add(es->operations, op);
		} else if (my_string_equals_ignorecase(t, "EQUALIZE")) {
			struct Operation *op = MY_MALLOC(1, struct Operation);
			op->operation = OP_EQUALIZE;
			my_vectorObj_add(es->operations, op);
		} else if (my_string_equals_ignorecase(t, "ERODE")) {
			struct Operation *op = MY_MALLOC(1, struct Operation);
			op->operation = OP_ERODE;
			op->kernel = cvCreateStructuringElementEx(3, 3, 1, 1,
					CV_SHAPE_ELLIPSE, NULL);
			my_vectorObj_add(es->operations, op);
		} else if (my_string_equals_ignorecase(t, "DILATE")) {
			struct Operation *op = MY_MALLOC(1, struct Operation);
			op->operation = OP_DILATE;
			op->kernel = cvCreateStructuringElementEx(3, 3, 1, 1,
					CV_SHAPE_ELLIPSE, NULL);
			my_vectorObj_add(es->operations, op);
		} else
			my_log_error("unknown parameter %s\n", t);
	}
	my_tokenizer_releaseValidateEnd(tk);
	*out_state = es;
}
static void tra_apply_gray(IplImage *imgGray, struct State_GRAY *es) {
	int64_t i;
	for (i = 0; i < my_vectorObj_size(es->operations); ++i) {
		struct Operation *op = my_vectorObj_get(es->operations, i);
		switch (op->operation) {
		case OP_EQUALIZE:
			cvEqualizeHist(imgGray, imgGray);
			break;
		case OP_THRESHOLD:
			cvThreshold(imgGray, imgGray, op->threshold, 255, CV_THRESH_BINARY);
			break;
		case OP_DILATE:
			cvDilate(imgGray, imgGray, op->kernel, 1);
			break;
		case OP_ERODE:
			cvErode(imgGray, imgGray, op->kernel, 1);
			break;
		case OP_PIXEL: {
			int64_t x, y;
			for (y = 0; y < imgGray->height; ++y) {
				uchar *ptr = (uchar*) (imgGray->imageData
						+ imgGray->widthStep * y);
				for (x = 0; x < imgGray->width; ++x) {
					double newVal = op->contrast * (ptr[x]) + op->brightness;
					if (op->isAbs && newVal < 0)
						newVal = -newVal;
					ptr[x] = my_math_round_uint8(newVal);
				}
			}
			break;
		}
		}

	}
}
static IplImage *tra_transform_gray(IplImage *image, int64_t numFrame,
		void* state) {
	struct State_GRAY *es = state;
	if (es->imgGray == NULL)
		es->imgGray = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	if (image->nChannels > 1) {
		cvCvtColor(image, es->imgGray, CV_BGR2GRAY);
		tra_apply_gray(es->imgGray, es);
		return es->imgGray;
	} else {
		tra_apply_gray(image, es);
		return image;
	}
}
static void tra_release_gray(void *state) {
	struct State_GRAY *es = state;
	my_image_release(es->imgGray);
	int64_t i;
	for (i = 0; i < my_vectorObj_size(es->operations); ++i) {
		struct Operation *op = my_vectorObj_get(es->operations, i);
		if (op->kernel != NULL)
			cvReleaseStructuringElement(&op->kernel);
		MY_FREE(op);
	}
	my_vectorObj_release(es->operations, 0);
	MY_FREE(es);
}

void tra_reg_gris() {
	Transform_Def *def = newTransformDef("GRAY",
			"[cContrast]_[bBrigthness]_[ABS]_[tThreshold]_[EQUALIZE]");
	def->func_new = tra_new_gray;
	def->func_transform_frame = tra_transform_gray;
	def->func_release = tra_release_gray;
}
#endif
