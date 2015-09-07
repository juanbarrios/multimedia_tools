/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
struct Estado_KF {
	MyImageResizer *resizer;
	MyImageColor *converter;
	struct Quantization quant;
	void *descriptor;
};

static void ext_config_kf(const char *code, const char *parameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct Estado_KF *es = MY_MALLOC(1, struct Estado_KF);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	int64_t width = my_tokenizer_nextInt(tk2);
	int64_t height = my_tokenizer_nextInt(tk2);
	my_tokenizer_release(tk2);
	struct MyColorSpace colorSpace = my_colorSpace_getColorSpace(
			my_tokenizer_nextToken(tk));
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	my_tokenizer_releaseValidateEnd(tk);
	es->resizer = my_imageResizer_newFixedSize(width, height);
	int64_t length = width * height * colorSpace.numChannels;
	es->descriptor = newDescriptorQuantize(es->quant, length);
	es->converter = my_imageColor_newConverter(colorSpace);
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, length, width * height,
			colorSpace.numChannels);
	*out_useImgGray = false;
}

static void *ext_extraer_kf(IplImage *image, void *state) {
	struct Estado_KF *es = state;
	IplImage *img = my_imageResizer_resizeImage(image, es->resizer);
	IplImage *imgConv =
			(img->nChannels == 1) ?
					img : my_imageColor_convertFromBGR(img, es->converter);
	int64_t x, y, nc, nChannels = imgConv->nChannels, cont = 0;
	if (es->quant.dtype == DTYPE_ARRAY_UCHAR) {
		uchar *array = (uchar*) es->descriptor;
		for (y = 0; y < imgConv->height; ++y) {
			uchar *ptr = (uchar*) (imgConv->imageData + imgConv->widthStep * y);
			for (x = 0; x < imgConv->width; ++x)
				for (nc = 0; nc < nChannels; ++nc)
					array[cont++] = ptr[nChannels * x + nc];
		}
	} else if (es->quant.dtype == DTYPE_ARRAY_FLOAT) {
		float *array = (float*) es->descriptor;
		for (y = 0; y < imgConv->height; ++y) {
			uchar *ptr = (uchar*) (imgConv->imageData + imgConv->widthStep * y);
			for (x = 0; x < imgConv->width; ++x)
				for (nc = 0; nc < nChannels; ++nc)
					array[cont++] = ptr[nChannels * x + nc] / 255.0;
		}
	} else
		my_log_error("unsupported quantization\n");
	return es->descriptor;
}
static void ext_release_kf(void *estado) {
	struct Estado_KF *es = estado;
	my_imageResizer_release(es->resizer);
	my_imageColor_release(es->converter);
	MY_FREE_MULTI(es->descriptor, es);
}
void ext_reg_kf() {
	addExtractorGlobalDef(false, "KF",
			"(width)x(height)_(colorspace)_(quant:1U|4F)", ext_config_kf,
			ext_extraer_kf, ext_release_kf);
}
#endif
