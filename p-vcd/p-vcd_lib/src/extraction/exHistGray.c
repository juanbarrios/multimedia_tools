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
struct State_HistGray {
	uchar useOpenCV, normalize;
	int64_t numBins;
	int64_t *val2bin;
	int64_t descriptorLength;
	struct Quantization quant;
	char *zoning_text;
	struct Zoning *zoning;
	double *bins, *current_bins;
	void *descriptor;
	CvHistogram *cv_hist;
};

static void ext_new_histg(const char *code, const char *parameters, void **out_state,
		DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	struct State_HistGray *es = MY_MALLOC(1, struct State_HistGray);
	es->zoning_text = my_newString_string(my_tokenizer_nextToken(tk));
	es->zoning = parseZoning(es->zoning_text);
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	es->numBins = my_tokenizer_nextInt(tk);
	es->normalize = my_tokenizer_isNext(tk, "NoNorm") ? 0 : 1;
	es->useOpenCV = my_tokenizer_isNext(tk, "UseOpenCV") ? 1 : 0;
	my_tokenizer_releaseValidateEnd(tk);
	es->descriptorLength = es->zoning->numZones * es->numBins;
	es->bins = MY_MALLOC_NOINIT(es->descriptorLength, double);
	es->descriptor = newDescriptorQuantize(es->quant, es->descriptorLength);
	if (es->useOpenCV) {
		float cv_rangos0[2] = { 0, 256 };
		float *cv_rangos[1] = { cv_rangos0 };
		int hist_size[1] = { (int) es->numBins };
		es->cv_hist = cvCreateHist(1, hist_size, CV_HIST_ARRAY, cv_rangos, 1);
	} else {
		es->val2bin = hist_val2bin(es->numBins, 256);
	}
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->descriptorLength,
			es->zoning->numZones, es->numBins);
	*out_useImgGray = true;
}

static void histGray_opencv(IplImage *image, CvRect rect,
		struct State_HistGray *es) {
	IplImage *cv_planos[1] = { image };
	cvCalcHist(cv_planos, es->cv_hist, 0, NULL);
	if (es->normalize)
		cvNormalizeHist(es->cv_hist, 1.0);
	int64_t i;
	for (i = 0; i < es->numBins; i++)
		es->bins[i] = cvGetReal1D(es->cv_hist->bins, i);
}
static void histGray_accumBins(uchar pixelVal, void *state) {
	struct State_HistGray *es = state;
	int64_t bin = es->val2bin[pixelVal];
	es->current_bins[bin]++;
}
static void histGray_processZone(IplImage *image, struct Zone *zone,
		void *state) {
	struct State_HistGray *es = state;
	if (es->useOpenCV) {
		histGray_opencv(image, zone->rect, es);
	} else {
		es->current_bins = es->bins + zone->numZone * es->numBins;
		hist_getBins1Channels(image, zone, histGray_accumBins, es);
		if (es->normalize)
			my_math_normalizeSum1_double(es->current_bins, es->numBins);
	}
}
static void *ext_extract_histg(IplImage *image, void *state) {
	struct State_HistGray *es = state;
	MY_SETZERO(es->bins, es->descriptorLength, double);
	processZoning(image, es->zoning, histGray_processZone, es);
	quantize(es->quant, es->bins, es->descriptor, es->descriptorLength);
	return es->descriptor;
}
struct ColorBin *getColorBins_gray(void *state) {
	struct State_HistGray *es = state;
	struct ColorBin *cbins = MY_MALLOC(es->numBins, struct ColorBin);
	for (int64_t nb = 0; nb < es->numBins; ++nb) {
		int64_t minVal = 0, maxVal = 0, avgVal = 0;
		hist_getValsWithBin(nb, es->val2bin, 256, &minVal, &maxVal, &avgVal);
		struct ColorBin *c = cbins + nb;
		hist_setCols(c->col_low, minVal, minVal, minVal);
		hist_setCols(c->col_high, maxVal, maxVal, maxVal);
		hist_setCols(c->col_avg, avgVal, avgVal, avgVal);
	}
	return cbins;
}
char *getImageZoningText_gray(void *state) {
	struct State_HistGray *es = state;
	return es->zoning_text;
}
static void ext_release_histg(void *estado) {
	struct State_HistGray *es = estado;
	MY_FREE(es->zoning_text);
	releaseZoning(es->zoning);
	if (es->useOpenCV)
		cvReleaseHist(&es->cv_hist);
	MY_FREE_MULTI(es->val2bin, es->bins, es->descriptor, es);
}
void ext_reg_histGray() {
	addExtractorGlobalDef(false, "HISTGRAY",
			"(zones:2x2+1x1+.)_(quant:1U|4F|.)_numBins_(opt:NoNorm)_(opt:useOpenCV)",
			ext_new_histg, ext_extract_histg, ext_release_histg);
}
#endif
