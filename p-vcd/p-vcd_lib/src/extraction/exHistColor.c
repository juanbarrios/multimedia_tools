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
struct State_HistColor {
	uchar useOpenCV, normalize;
	struct MyColorSpace colorSpace;
	int64_t numBins1, numBins2, numBins3, totalBins;
	int64_t *val2bin_1, *val2bin_2, *val2bin_3;
	int64_t descriptorLength;
	struct Quantization quant;
	char *zoning_text;
	struct Zoning *zoning;
	MyImageColor *converter;
	double *bins, *current_bins;
	void *descriptor;
	IplImage *cv_plano0, *cv_plano1, *cv_plano2;
	CvHistogram *cv_hist;
};

static void ext_new_hist(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct State_HistColor *es = MY_MALLOC(1, struct State_HistColor);
	es->zoning_text = my_newString_string(my_tokenizer_nextToken(tk));
	es->zoning = parseZoning(es->zoning_text);
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	es->colorSpace = my_colorSpace_getColorSpace(my_tokenizer_nextToken(tk));
	my_assert_equalInt("numChannels", es->colorSpace.numChannels, 3);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), 'x');
	es->numBins1 = my_tokenizer_nextInt(tk2);
	es->numBins2 = my_tokenizer_nextInt(tk2);
	es->numBins3 = my_tokenizer_nextInt(tk2);
	my_tokenizer_releaseValidateEnd(tk2);
	es->normalize = my_tokenizer_isNext(tk, "NoNorm") ? 0 : 1;
	es->useOpenCV = my_tokenizer_isNext(tk, "useOpenCV") ? 1 : 0;
	my_tokenizer_releaseValidateEnd(tk);
	es->converter = my_imageColor_newConverter(es->colorSpace);
	es->totalBins = es->numBins1 * es->numBins2 * es->numBins3;
	es->descriptorLength = es->zoning->numZones * es->totalBins;
	es->bins = MY_MALLOC_NOINIT(es->descriptorLength, double);
	es->descriptor = newDescriptorQuantize(es->quant, es->descriptorLength);
	if (es->useOpenCV) {
		float cv_rangos0[2] = { es->colorSpace.channel0_min,
				es->colorSpace.channel0_max };
		float cv_rangos1[2] = { es->colorSpace.channel1_min,
				es->colorSpace.channel1_max };
		float cv_rangos2[2] = { es->colorSpace.channel2_min,
				es->colorSpace.channel2_max };
		float *cv_rangos[3] = { cv_rangos0, cv_rangos1, cv_rangos2 };
		int hist_size[3] = { (int) es->numBins1, (int) es->numBins2,
				(int) es->numBins3 };
		es->cv_hist = cvCreateHist(3, hist_size, CV_HIST_ARRAY, cv_rangos, 1);
	} else {
		es->val2bin_1 = hist_val2bin(es->numBins1, es->colorSpace.channel0_max);
		es->val2bin_2 = hist_val2bin(es->numBins2, es->colorSpace.channel1_max);
		es->val2bin_3 = hist_val2bin(es->numBins3, es->colorSpace.channel2_max);
	}
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->descriptorLength,
			es->zoning->numZones, es->totalBins);
	*out_useImgGray = false;
}

static void histColor_opencv(IplImage *image, CvRect rect,
		struct State_HistColor *es) {
	IplImage *cv_planos[3] = { es->cv_plano0, es->cv_plano1, es->cv_plano2 };
	cvCalcHist(cv_planos, es->cv_hist, 0, NULL);
	if (es->normalize)
		cvNormalizeHist(es->cv_hist, 1.0);
	int64_t cont = 0;
	for (int64_t k = 0; k < es->numBins3; k++)
		for (int64_t j = 0; j < es->numBins2; j++)
			for (int64_t i = 0; i < es->numBins1; i++)
				es->bins[cont++] = cvGetReal3D(es->cv_hist->bins, i, j, k);
}
static void histColor_accumBins(uchar pixelVal1, uchar pixelVal2,
		uchar pixelVal3, void *state) {
	struct State_HistColor *es = state;
	if (pixelVal1 >= es->colorSpace.channel0_max)
		pixelVal1 = es->colorSpace.channel0_max - 1;
	if (pixelVal2 >= es->colorSpace.channel1_max)
		pixelVal2 = es->colorSpace.channel1_max - 1;
	if (pixelVal3 >= es->colorSpace.channel2_max)
		pixelVal3 = es->colorSpace.channel2_max - 1;
	int64_t bin1 = es->val2bin_1[pixelVal1];
	int64_t bin2 = es->val2bin_2[pixelVal2];
	int64_t bin3 = es->val2bin_3[pixelVal3];
	int64_t bin = bin1 + bin2 * es->numBins1 + bin3 * es->numBins1 * es->numBins2;
	es->current_bins[bin]++;
}
static void histColor_processZone(IplImage *image, struct Zone *zone,
		void *state) {
	struct State_HistColor *es = state;
	if (es->useOpenCV) {
		histColor_opencv(image, zone->rect, es);
	} else {
		es->current_bins = es->bins + zone->numZone * es->totalBins;
		hist_getBins3Channels(image, zone, histColor_accumBins, es);
		if (es->normalize)
			my_math_normalizeSum1_double(es->current_bins, es->totalBins);
	}
}
static void *ext_extract_hist(IplImage *image, void *state) {
	struct State_HistColor *es = state;
	MY_SETZERO(es->bins, es->descriptorLength, double);
	IplImage *frame = my_imageColor_convertFromBGR(image, es->converter);
	if (es->useOpenCV) {
		if (es->cv_plano0 == NULL || es->cv_plano0->width != frame->width
				|| es->cv_plano0->height != frame->height) {
			my_image_release(es->cv_plano0);
			my_image_release(es->cv_plano1);
			my_image_release(es->cv_plano2);
			es->cv_plano0 = cvCreateImage(cvGetSize(frame), 8, 1);
			es->cv_plano1 = cvCreateImage(cvGetSize(frame), 8, 1);
			es->cv_plano2 = cvCreateImage(cvGetSize(frame), 8, 1);
		}
		cvSplit(frame, es->cv_plano0, es->cv_plano1, es->cv_plano2, 0);
	}
	processZoning(frame, es->zoning, histColor_processZone, es);
	quantize(es->quant, es->bins, es->descriptor, es->descriptorLength);
	return es->descriptor;
}

static void ext_release_hist(void *estado) {
	struct State_HistColor *es = estado;
	my_imageColor_release(es->converter);
	releaseZoning(es->zoning);
	if (es->useOpenCV) {
		cvReleaseHist(&es->cv_hist);
		my_image_release(es->cv_plano0);
		my_image_release(es->cv_plano1);
		my_image_release(es->cv_plano2);
	}
	MY_FREE_MULTI(es->val2bin_1, es->val2bin_2, es->val2bin_3, es->bins,
			es->descriptor, es);
}
void ext_reg_histColFixed() {
	addExtractorGlobalDef(false, "HISTCOLOR",
			"(zones:2x2+1x1+.)_(quant:1U|4F|.)_(colorSpace)_(numBins1)x(numBins2)x(numBins3)_(opt:NoNorm)_(opt:useOpenCV)",
			ext_new_hist, ext_extract_hist, ext_release_hist);
}
struct ColorBin *getColorBins_color(void *state) {
	struct State_HistColor *es = state;
	struct ColorBin *cbins = MY_MALLOC(es->totalBins, struct ColorBin);
	int64_t cont = 0;
	for (int64_t nb3 = 0; nb3 < es->numBins3; ++nb3) {
		int64_t minVal3 = 0, maxVal3 = 0, avgVal3 = 0;
		hist_getValsWithBin(nb3, es->val2bin_3, 256, &minVal3, &maxVal3,
				&avgVal3);
		for (int64_t nb2 = 0; nb2 < es->numBins2; ++nb2) {
			int64_t minVal2 = 0, maxVal2 = 0, avgVal2 = 0;
			hist_getValsWithBin(nb2, es->val2bin_2, 256, &minVal2, &maxVal2,
					&avgVal2);
			for (int64_t nb1 = 0; nb1 < es->numBins1; ++nb1) {
				int64_t minVal1 = 0, maxVal1 = 0, avgVal1 = 0;
				hist_getValsWithBin(nb1, es->val2bin_1, 256, &minVal1, &maxVal1,
						&avgVal1);
				struct ColorBin *c = cbins + cont;
				hist_setCols(c->col_low, minVal1, minVal2, minVal3);
				hist_setCols(c->col_high, maxVal1, maxVal2, maxVal3);
				hist_setCols(c->col_avg, avgVal1, avgVal2, avgVal3);
				cont++;
			}
		}
	}
	my_assert_equalInt("nbins", cont, es->totalBins);
	return cbins;
}
char *getImageZoningText_color(void *state) {
	struct State_HistColor *es = state;
	return es->zoning_text;
}
struct MyColorSpace getColorSpace_color(void *state) {
	struct State_HistColor *es = state;
	return es->colorSpace;
}
#if 0
static pthread_mutex_t mutex_saveColors = PTHREAD_MUTEX_INITIALIZER;
static void saveColorBins(struct State_HistColor *es, char *filename);
static void saveColorBins(struct State_HistColor *es, char *filename) {
	struct ColorBin *cbins = histColor_getColorBins(es);
	FILE *out = openFileWrite1Config(filename, "ColorBins", 1, 0);
	fprintf(out, "num_bins=%"PRIi64"\n", es->totalBins);
	fprintf(out, "color_space=%s\n", es->colorSpace.code);
	int64_t i;
	for (i = 0; i < es->totalBins; ++i) {
		struct ColorBin cb = cbins[i];
		fprintf(out, "bin_%02"PRIi64"=%u,%u,%u\t%u,%u,%u\t%u,%u,%u\n", i,
				cb.col_low[0], cb.col_low[1], cb.col_low[2], cb.col_avg[0],
				cb.col_avg[1], cb.col_avg[2], cb.col_high[0], cb.col_high[1],
				cb.col_high[2]);
	}
	fclose(out);
}
static void save() {
	if (ip->path_descriptorOut != NULL) {
		char *filename = my_newString_format("%s/color_bins.txt", ip->path_descriptorOut);
		MY_MUTEX_LOCK(mutex_saveColors);
		if (!existsFile(filename))
		saveColorBins(es, filename);
		MY_MUTEX_UNLOCK(mutex_saveColors);
		MY_FREE(filename);
	}

}

#endif
#endif
