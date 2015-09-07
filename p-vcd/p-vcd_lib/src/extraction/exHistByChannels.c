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
struct State_HistByChannel {
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
	CvHistogram *cv_hist1, *cv_hist2, *cv_hist3;
};

static void ext_new_hist(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct State_HistByChannel *es = MY_MALLOC(1, struct State_HistByChannel);
	es->zoning_text = my_newString_string(my_tokenizer_nextToken(tk));
	es->zoning = parseZoning(es->zoning_text);
	es->colorSpace = my_colorSpace_getColorSpace(my_tokenizer_nextToken(tk));
	my_assert_equalInt("numChannels", es->colorSpace.numChannels, 3);
	MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), '+');
	es->numBins1 = my_tokenizer_nextInt(tk2);
	es->numBins2 = my_tokenizer_nextInt(tk2);
	es->numBins3 = my_tokenizer_nextInt(tk2);
	my_tokenizer_releaseValidateEnd(tk2);
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	es->normalize = my_tokenizer_isNext(tk, "NoNorm") ? 0 : 1;
	es->useOpenCV = my_tokenizer_isNext(tk, "useOpenCV") ? 1 : 0;
	my_tokenizer_releaseValidateEnd(tk);
	es->converter = my_imageColor_newConverter(es->colorSpace);
	es->totalBins = es->numBins1 + es->numBins2 + es->numBins3;
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
		float *cv_rangos[1] = { cv_rangos0 };
		int hist_size[1] = { (int) es->numBins1 };
		es->cv_hist1 = cvCreateHist(1, hist_size, CV_HIST_ARRAY, cv_rangos, 1);
		hist_size[0] = es->numBins2;
		cv_rangos[0] = cv_rangos1;
		es->cv_hist2 = cvCreateHist(1, hist_size, CV_HIST_ARRAY, cv_rangos, 1);
		hist_size[0] = es->numBins3;
		cv_rangos[0] = cv_rangos2;
		es->cv_hist3 = cvCreateHist(1, hist_size, CV_HIST_ARRAY, cv_rangos, 1);
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

static void histByChannel_opencv(IplImage *image, CvRect rect,
		struct State_HistByChannel *es) {
	IplImage *cv_planos[1] = { es->cv_plano0 };
	cvCalcHist(cv_planos, es->cv_hist1, 0, 0);
	cv_planos[0] = es->cv_plano1;
	cvCalcHist(cv_planos, es->cv_hist2, 0, 0);
	cv_planos[0] = es->cv_plano2;
	cvCalcHist(cv_planos, es->cv_hist3, 0, 0);
	if (es->normalize) {
		cvNormalizeHist(es->cv_hist1, 1.0);
		cvNormalizeHist(es->cv_hist2, 1.0);
		cvNormalizeHist(es->cv_hist3, 1.0);
	}
	int64_t i, cont = 0;
	for (i = 0; i < es->numBins1; i++)
		es->bins[cont++] = cvGetReal1D(es->cv_hist1->bins, i);
	for (i = 0; i < es->numBins2; i++)
		es->bins[cont++] = cvGetReal1D(es->cv_hist2->bins, i);
	for (i = 0; i < es->numBins3; i++)
		es->bins[cont++] = cvGetReal1D(es->cv_hist3->bins, i);
}
static void histByChannel_accumBins(uchar pixelVal1, uchar pixelVal2,
		uchar pixelVal3, void *state) {
	struct State_HistByChannel *es = state;
	if (pixelVal1 >= es->colorSpace.channel0_max)
		pixelVal1 = es->colorSpace.channel0_max - 1;
	if (pixelVal2 >= es->colorSpace.channel1_max)
		pixelVal2 = es->colorSpace.channel1_max - 1;
	if (pixelVal3 >= es->colorSpace.channel2_max)
		pixelVal3 = es->colorSpace.channel2_max - 1;
	int64_t bin1 = es->val2bin_1[pixelVal1];
	int64_t bin2 = es->val2bin_2[pixelVal2];
	int64_t bin3 = es->val2bin_3[pixelVal3];
	es->current_bins[bin1]++;
	es->current_bins[bin2 + es->numBins1]++;
	es->current_bins[bin3 + es->numBins1 + es->numBins2]++;
}
static void histByChannel_processZone(IplImage *image, struct Zone *zone,
		void *state) {
	struct State_HistByChannel *es = state;
	if (es->useOpenCV) {
		histByChannel_opencv(image, zone->rect, es);
	} else {
		es->current_bins = es->bins + zone->numZone * es->totalBins;
		hist_getBins3Channels(image, zone, histByChannel_accumBins, es);
		if (es->normalize)
			my_math_normalizeSum1_double(es->current_bins, es->totalBins);
	}
}
static void *ext_extract_hist(IplImage *image, void *state) {
	struct State_HistByChannel *es = state;
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
	processZoning(frame, es->zoning, histByChannel_processZone, es);
	quantize(es->quant, es->bins, es->descriptor, es->descriptorLength);
	return es->descriptor;
}
static void ext_release_hist(void *estado) {
	struct State_HistByChannel *es = estado;
	my_imageColor_release(es->converter);
	releaseZoning(es->zoning);
	releaseZoning(es->zoning);
	if (es->useOpenCV) {
		my_image_release(es->cv_plano0);
		my_image_release(es->cv_plano1);
		my_image_release(es->cv_plano2);
		cvReleaseHist(&es->cv_hist1);
		cvReleaseHist(&es->cv_hist2);
		cvReleaseHist(&es->cv_hist3);
	}
	MY_FREE_MULTI(es->val2bin_1, es->val2bin_2, es->val2bin_3, es->bins,
			es->descriptor, es);
}
void ext_reg_histByChannel() {
	addExtractorGlobalDef(false, "HISTBYCHANNEL",
			"(zones:2x2+1x1)_(colorSpace)_(numBins1)+(numBins2)+(numBins3)_(quant:1U|4F...)_(opt:NoNorm)_(opt:useOpenCV)",
			ext_new_hist, ext_extract_hist, ext_release_hist);
}
struct ColorBin *getColorBins_byChannel(void *state) {
	struct State_HistByChannel *es = state;
	struct ColorBin *cbins = MY_MALLOC(es->totalBins, struct ColorBin);
	/*
	int64_t nb1, nb2, nb3, cont = 0;
	for (nb1 = 0; nb1 < es->numBins1; ++nb1) {
		int64_t minVal = 0, maxVal = 0, avgVal = 0;
		hist_getValsWithBin(nb1, es->val2bin_1, 256, &minVal, &maxVal, &avgVal);
		struct ColorBin *c = cbins + cont;
		if (es->colorSpaceCode == CS_RGB) {
			hist_setCols(c->col_low, es->colorSpace->channel0_min, 0, 0);
			hist_setCols(c->col_high, es->colorSpace->channel0_max, 0, 0);
			hist_setCols(c->col_avg, (es->colorSpace->channel0_min, 0, 0);
		} else if (es->colorSpaceCode == CS_HSV) {
			hist_setCols(c->col_low, minVal, 255, 255);
			hist_setCols(c->col_high, maxVal, 255, 255);
			hist_setCols(c->col_avg, avgVal, 255, 255);
		} else if (es->colorSpaceCode == CS_HLS) {
			hist_setCols(c->col_low, minVal, 127, 255);
			hist_setCols(c->col_high, maxVal, 127, 255);
			hist_setCols(c->col_avg, avgVal, 127, 255);
		} else {
			hist_setCols(c->col_low, minVal, 0, 0);
			hist_setCols(c->col_high, maxVal, 0, 0);
			hist_setCols(c->col_avg, avgVal, 0, 0);
		}
		cont++;
	}
	for (nb2 = 0; nb2 < es->numBins2; ++nb2) {
		int64_t minVal = 0, maxVal = 0, avgVal = 0;
		hist_getValsWithBin(nb2, es->val2bin_2, 256, &minVal, &maxVal, &avgVal);
		struct ColorBin *c = cbins + cont;
		if (es->colorSpaceCode == CS_RGB) {
			hist_setCols(c->col_low, 0, minVal, 0);
			hist_setCols(c->col_high, 0, maxVal, 0);
			hist_setCols(c->col_avg, 0, avgVal, 0);
		} else if (es->colorSpaceCode == CS_HSV) {
			hist_setCols(c->col_low, 0, minVal, 255);
			hist_setCols(c->col_high, 0, maxVal, 255);
			hist_setCols(c->col_avg, 0, avgVal, 255);
		} else if (es->colorSpaceCode == CS_HLS) {
			hist_setCols(c->col_low, 0, minVal, 0);
			hist_setCols(c->col_high, 0, maxVal, 0);
			hist_setCols(c->col_avg, 0, avgVal, 0);
		} else {
			hist_setCols(c->col_low, 0, minVal, 0);
			hist_setCols(c->col_high, 0, maxVal, 0);
			hist_setCols(c->col_avg, 0, avgVal, 0);
		}
		cont++;
	}
	for (nb3 = 0; nb3 < es->numBins3; ++nb3) {
		int64_t minVal = 0, maxVal = 0, avgVal = 0;
		hist_getValsWithBin(nb3, es->val2bin_3, 256, &minVal, &maxVal, &avgVal);
		struct ColorBin *c = cbins + cont;
		if (es->colorSpaceCode == CS_RGB) {
			hist_setCols(c->col_low, 0, 0, minVal);
			hist_setCols(c->col_high, 0, 0, maxVal);
			hist_setCols(c->col_avg, 0, 0, avgVal);
		} else if (es->colorSpaceCode == CS_HSV) {
			hist_setCols(c->col_low, 0, 255, minVal);
			hist_setCols(c->col_high, 0, 255, maxVal);
			hist_setCols(c->col_avg, 0, 255, avgVal);
		} else if (es->colorSpaceCode == CS_HLS) {
			hist_setCols(c->col_low, 0, 0, minVal);
			hist_setCols(c->col_high, 0, 0, maxVal);
			hist_setCols(c->col_avg, 0, 0, avgVal);
		} else {
			hist_setCols(c->col_low, 0, 0, minVal);
			hist_setCols(c->col_high, 0, 0, maxVal);
			hist_setCols(c->col_avg, 0, 0, avgVal);
		}
		cont++;
	}
	assertEqual_int("nbins", cont, es->totalBins);*/
	return cbins;
}
char *getImageZoningText_byChannel(void *state) {
	struct State_HistByChannel *es = state;
	return es->zoning_text;
}
struct MyColorSpace getColorSpace_byChannel(void *state) {
	struct State_HistByChannel *es = state;
	return es->colorSpace;
}

#if 0
static pthread_mutex_t mutex_saveColors = PTHREAD_MUTEX_INITIALIZER;
static void saveColorBins(struct State_HistByChannel *es, char *filename);
static void saveColorBins(struct State_HistByChannel *es, char *filename) {
	struct ColorBin *cbins = histByChannel_getColorBins(es);
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
static void a() {
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
