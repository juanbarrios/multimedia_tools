/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
//HAMPAPUR and BOLLE 2001
//LOCAL EDGE REPRESENTATION
struct Estado_LE {
	IplImage *imgGris2, *imageGris16bitsX, *imageGris16bitsY;
	double thresholdSobel;
	int64_t numDivisionsW, numDivisionsH, numSubDivisionesW, numSubDivisionesH;
	int64_t lastW, lastH, *limitsW, *limitsH;
	uchar *bins;
	MyImageResizer *resizer;
};

static void ext_config_le(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	my_tokenizer_addDelimiter(tk, 'x');
	struct Estado_LE *es = MY_MALLOC(1, struct Estado_LE);
	es->resizer = my_imageResizer_newTextOptions(my_tokenizer_nextToken(tk));
	es->thresholdSobel = my_tokenizer_nextDouble(tk);
	es->numDivisionsW = my_tokenizer_nextInt(tk);
	es->numDivisionsH = my_tokenizer_nextInt(tk);
	es->numSubDivisionesW = my_tokenizer_nextInt(tk);
	es->numSubDivisionesH = my_tokenizer_nextInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
	int64_t largo = 2 * es->numDivisionsW * es->numDivisionsH;
	es->bins = MY_MALLOC_NOINIT(largo, uchar);
	*out_state = es;
	*out_td = descriptorType(DTYPE_ARRAY_UCHAR, largo, es->numDivisionsW,
			es->numDivisionsH);
	*out_useImgGray = true;
}

static void calcular_centroide(IplImage *img, int64_t desde_w, int64_t hasta_w,
		int64_t desde_h, int64_t hasta_h, int64_t numSubDivisionesW,
		int64_t numSubDivisionesH, int64_t *pos_w, int64_t *pos_h) {
	double centroide_w = 0, centroide_h = 0;
	int64_t cont = 0;
	int64_t x, y;
	uchar *ptr1;
	for (y = desde_h; y < hasta_h; ++y) {
		ptr1 = (uchar*) (img->imageData + img->widthStep * y);
		for (x = desde_w; x < hasta_w; ++x) {
			if (ptr1[x]) {
				centroide_w += x;
				centroide_h += y;
				cont++;
			}
		}
	}
	*pos_w = numSubDivisionesW;
	*pos_h = numSubDivisionesH;
	if (cont > 0) {
		centroide_w = (centroide_w / cont) - desde_w;
		centroide_h = (centroide_h / cont) - desde_h;
		double size_w = (hasta_w - desde_w) / ((double) numSubDivisionesW);
		double size_h = (hasta_h - desde_h) / ((double) numSubDivisionesH);
		int64_t i;
		for (i = 0; i < numSubDivisionesW; ++i) {
			if (centroide_w < size_w * (i + 1)) {
				*pos_w = i;
				break;
			}
		}
		for (i = 0; i < numSubDivisionesH; ++i) {
			if (centroide_h < size_h * (i + 1)) {
				*pos_h = i;
				break;
			}
		}
	}
	if (0) {
		my_log_info("width=[%"PRIi64"..%"PRIi64") height=[%"PRIi64"..%"PRIi64") ",
				desde_w, hasta_w, desde_h, hasta_h);
		my_log_info(
				"width=[0..%"PRIi64") = %lf = %"PRIi64"   height=[0..%"PRIi64") = %lf = %"PRIi64"\n",
				numSubDivisionesW, centroide_w, *pos_w, numSubDivisionesH,
				centroide_h, *pos_h);
	}
}

static void *ext_extraer_le(IplImage *image, void *state) {
	struct Estado_LE *es = state;
	IplImage *imgGris = my_imageResizer_resizeImage(image, es->resizer);
	if (es->lastW != image->width || es->lastH != image->height) {
		my_image_release(es->imgGris2);
		my_image_release(es->imageGris16bitsX);
		my_image_release(es->imageGris16bitsY);
		MY_FREE_MULTI(es->limitsW, es->limitsH);
		es->imgGris2 = cvCreateImage(cvGetSize(imgGris), 8, 1);
		es->imageGris16bitsX = cvCreateImage(cvGetSize(imgGris), IPL_DEPTH_16S,
				1);
		es->imageGris16bitsY = cvCreateImage(cvGetSize(imgGris), IPL_DEPTH_16S,
				1);
		es->limitsW = my_math_partitionIntUB(es->numDivisionsW,
				imgGris->width);
		es->limitsH = my_math_partitionIntUB(es->numDivisionsH,
				imgGris->height);
		es->lastW = image->width;
		es->lastH = image->height;
	}
	cvSobel(imgGris, es->imageGris16bitsX, 1, 0, 3);
	cvSobel(imgGris, es->imageGris16bitsY, 0, 1, 3);
	int64_t x, y;
	for (y = 0; y < imgGris->height; ++y) {
		short *ptr1 = (short*) (es->imageGris16bitsX->imageData
				+ es->imageGris16bitsX->widthStep * y);
		short *ptr2 = (short*) (es->imageGris16bitsY->imageData
				+ es->imageGris16bitsY->widthStep * y);
		uchar *ptr = (uchar*) (es->imgGris2->imageData
				+ es->imgGris2->widthStep * y);
		for (x = 0; x < imgGris->width; ++x) {
			double magnitud = sqrt(
					((double) ptr1[x]) * ptr1[x]
							+ ((double) ptr2[x]) * ptr2[x]);
			ptr[x] = magnitud < es->thresholdSobel ? 0 : 255;
		}
	}
	int64_t cent1_w = 0, cent1_h = 0;
	int64_t i, j, pos = 0;
	uchar *byte_array = es->bins;
	for (j = 0; j < es->numDivisionsW; ++j) {
		for (i = 0; i < es->numDivisionsH; ++i) {
			calcular_centroide(es->imgGris2, j == 0 ? 0 : es->limitsW[j - 1],
					es->limitsW[j], i == 0 ? 0 : es->limitsH[i - 1],
					es->limitsH[i], es->numSubDivisionesW,
					es->numSubDivisionesH, &cent1_w, &cent1_h);
			byte_array[pos++] = cent1_w;
			byte_array[pos++] = cent1_h;
		}
	}
	return es->bins;
}
static void ext_release_le(void *estado) {
	struct Estado_LE *es = estado;
	my_imageResizer_release(es->resizer);
	my_image_release(es->imgGris2);
	my_image_release(es->imageGris16bitsX);
	my_image_release(es->imageGris16bitsY);
	MY_FREE_MULTI(es->limitsW, es->limitsH, es->bins, es);
}
void ext_reg_le() {
	addExtractorGlobalDef(false, "HAMP",
			"[resizeOptions]_[thresholdSobel]_[numDivisionsW]x[numDivisionsH]_[numSubDivisionesW]x[numSubDivisionesH]",
			ext_config_le, ext_extraer_le, ext_release_le);
}
#endif
