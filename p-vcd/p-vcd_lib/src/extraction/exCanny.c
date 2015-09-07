/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
struct Estado_CANNY {
	IplImage *imgGris2;
	int64_t width, height;
	double threshold1, threshold2;
	int64_t apertureSize;
	uchar *byte_array;
	MyImageResizer *resizer;
};

static void ext_config_canny(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct Estado_CANNY *es = MY_MALLOC(1, struct Estado_CANNY);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	es->width = my_tokenizer_nextInt(tk2);
	es->height = my_tokenizer_nextInt(tk2);
	my_tokenizer_release(tk2);
	es->threshold1 = my_tokenizer_nextDouble(tk);
	es->threshold2 = my_tokenizer_nextDouble(tk);
	es->apertureSize = my_tokenizer_nextInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
	es->resizer = my_imageResizer_newFixedSize(es->width, es->height);
	int64_t numBytes = (es->width * es->height) / 8;
	if ((es->width * es->height) % 8 > 0)
		numBytes++;
	es->byte_array = MY_MALLOC_NOINIT(numBytes, uchar);
	*out_state = es;
	*out_td = descriptorType(DTYPE_MATRIX_BITS, numBytes, es->width,
			es->height);
	*out_useImgGray = true;
}
static void *ext_extraer_canny(IplImage *image, void *state) {
	struct Estado_CANNY *es = state;
	IplImage *imgGray = my_imageResizer_resizeImage(image, es->resizer);
	if (es->imgGris2 == NULL || es->imgGris2->width != imgGray->width
			|| es->imgGris2->height != imgGray->height) {
		my_image_release(es->imgGris2);
		es->imgGris2 = my_image_duplicate(imgGray);
	}
	cvCanny(imgGray, es->imgGris2, es->threshold1, es->threshold2,
			es->apertureSize);
	int64_t x, y, contBytes = 0, contBits = 0;
	for (y = 0; y < es->height; ++y) {
		uchar *ptr1 = (uchar*) (es->imgGris2->imageData
				+ es->imgGris2->widthStep * y);
		for (x = 0; x < es->width; ++x) {
			if (contBits == 0)
				es->byte_array[contBytes] = 0;
			if (ptr1[x] > 127)
				es->byte_array[contBytes] |= 1 << contBits;
			contBits++;
			if (contBits == 8) {
				contBits = 0;
				contBytes++;
			}
		}
	}
	return es->byte_array;
}
static void ext_release_canny(void *estado) {
	struct Estado_CANNY *es = estado;
	my_imageResizer_release(es->resizer);
	my_image_release(es->imgGris2);
	MY_FREE(es->byte_array);
	MY_FREE(es);
}
void ext_reg_canny() {
	addExtractorGlobalDef(false, "CANNY",
			"(width)x(height)_threshold1_threshold2_apertureSize",
			ext_config_canny, ext_extraer_canny, ext_release_canny);
}
#endif
