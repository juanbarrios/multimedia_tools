/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"
#include "exHist.h"

#ifndef NO_OPENCV
//HAMPAPUR and BOLLE 2001
//GRADIENT DIRECTION HISTOGRAM
struct Estado_GDH {
	struct Quantization quant;
	char *zoning_text;
	struct Zoning *zoning;
	double thresholdSobel;
	int64_t numBins;
	int64_t descriptorLength;
	void *descriptor;
	double *bins;
	double *limitesHist;
	IplImage *imageGris16bitsX, *imageGris16bitsY;
};

static void ext_config_gdh(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct Estado_GDH *es = MY_MALLOC(1, struct Estado_GDH);
	es->zoning_text = my_newString_string(my_tokenizer_nextToken(tk));
	es->zoning = parseZoning(es->zoning_text);
	es->numBins = my_tokenizer_nextInt(tk);
	es->thresholdSobel = my_tokenizer_nextDouble(tk);
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	my_tokenizer_releaseValidateEnd(tk);
	es->descriptorLength = es->zoning->numZones * es->numBins;
	es->bins = MY_MALLOC_NOINIT(es->descriptorLength, double);
	es->descriptor = newDescriptorQuantize(es->quant, es->descriptorLength);
	es->limitesHist = my_math_partitionDoubleUB(es->numBins,
	M_PI);
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->descriptorLength,
			es->zoning->numZones, es->numBins);
	*out_useImgGray = true;
}

static void gdh_calcular_histogram(IplImage *image, struct Zone *zone,
		void *state) {
	struct Estado_GDH *es = state;
	double *current_bins = es->bins + zone->numZone * es->numBins;
	int64_t x, y;
	for (y = zone->pix_start_h; y <= zone->pix_end_h; ++y) {
		short *ptr1 = (short*) (es->imageGris16bitsX->imageData
				+ es->imageGris16bitsX->widthStep * y);
		short *ptr2 = (short*) (es->imageGris16bitsY->imageData
				+ es->imageGris16bitsY->widthStep * y);
		for (x = zone->pix_start_w; x < zone->pix_end_w; ++x) {
			double magnitude = sqrt(
					((double) ptr1[x]) * ptr1[x]
							+ ((double) ptr2[x]) * ptr2[x]);
			if (magnitude < es->thresholdSobel)
				continue;
			double angle;
			if (ptr2[x] == 0)
				angle = 0;
			else if (ptr1[x] == 0)
				angle = ptr2[x] < 0 ? -M_PI_2 : M_PI_2;
			else
				angle = atan(ptr2[x] / ((double) ptr1[x]));
			angle += M_PI_2; //angle in  [0,pi]
			int64_t i;
			for (i = 0; i < es->numBins; ++i) {
				if (angle < es->limitesHist[i]) {
					current_bins[i]++;
					break;
				}
			}
		}
	}
	my_math_normalizeSum1_double(current_bins, es->numBins);
}
static void *ext_extraer_gdh(IplImage *image, void *state) {
	struct Estado_GDH *es = state;
	if (es->imageGris16bitsX == NULL
			|| es->imageGris16bitsX->width != image->width
			|| es->imageGris16bitsX->height != image->height) {
		my_image_release(es->imageGris16bitsX);
		my_image_release(es->imageGris16bitsY);
		es->imageGris16bitsX = cvCreateImage(cvGetSize(image), IPL_DEPTH_16S,
				1);
		es->imageGris16bitsY = cvCreateImage(cvGetSize(image), IPL_DEPTH_16S,
				1);
	}
	cvSobel(image, es->imageGris16bitsX, 1, 0, 3);
	cvSobel(image, es->imageGris16bitsY, 0, 1, 3);
	MY_SETZERO(es->bins, es->descriptorLength, double);
	processZoning(image, es->zoning, gdh_calcular_histogram, es);
	quantize(es->quant, es->bins, es->descriptor, es->descriptorLength);
	return es->descriptor;
}

static void ext_release_gdh(void *state) {
	struct Estado_GDH *es = state;
	my_image_release(es->imageGris16bitsX);
	my_image_release(es->imageGris16bitsY);
	MY_FREE_MULTI(es->limitesHist, es->bins, es);
}
void ext_reg_gdh() {
	addExtractorGlobalDef(false, "GDH",
			"(zones:2x2+1x1)_[numBins]_[thresholdSobel]_(quant:1U|4F...)",
			ext_config_gdh, ext_extraer_gdh, ext_release_gdh);
}
struct ColorBin *getColorBins_gradient(void *state) {
	struct Estado_GDH *es = state;
	char *opt = my_newString_format("1x1_1U_%"PRIi64"", es->numBins);
	Extractor *ex = getExtractor2("HISTGRAY", opt);
	struct ColorBin *cbins = getColorBins_gray(getExtractorState(ex));
	releaseExtractor(ex);
	free(opt);
	return cbins;
}
char *getImageZoningText_gradient(void *state) {
	struct Estado_GDH *es = state;
	return es->zoning_text;
}
#endif
