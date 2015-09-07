/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"
#ifndef NO_OPENCV
#include "../extraction/exHist.h"

#if 0
#define USE_BLACK_FRAME 0
#define USE_FOOTER 1
#define USE_PLAIN_BACKGROUND 0
#define USE_DEGRADATION 1
#define BLACK_FRAME_SIZE 2
#define FOOTER_SIZE 10
#define PRINT_HIST 0

#define SCALE_ZONE_W 1
#define SCALE_ZONE_H 1

struct Proceso_HIST {
	uchar isHistG, isHist1x3D, isHist3x1D;
	Extractor *ex;
	int64_t csHist, totalBinsHist, lengthDesc, zonesH, zonesW;
	double *histogram;
	IplImage *img_cshistOne, *img_cshistAll, *img_bgrAll, *bg_cshistOne;
	struct ColorBin *colBins;
};
static int64_t getCvtCode_cshist_bgr(struct Proceso_HIST *es) {
	if (es->isHistG)
	return CV_RGB2BGR;
	struct State_HistColor *esHistC = es->ex->state;
	switch (esHistC->colorSpace) {
		case CS_RGB:
		return CV_RGB2BGR;
		case CS_HSV:
		return CV_HSV2BGR;
		case CS_HLS:
		return CV_HLS2BGR;
		case CS_LAB:
		return CV_Lab2BGR;
		case CS_LUV:
		return CV_Luv2BGR;
		case CS_XYZ:
		return CV_XYZ2BGR;
		case CS_YCrCb:
		return CV_YCrCb2BGR;
		default:
		my_log_info("unknown colorspace %"PRIi64"\n", esHistC->colorSpace);
		return 0;
	}
}
static int64_t getCvtCode_rgb_cshist(struct Proceso_HIST *es) {
	if (es->isHistG)
	return -1;
	struct State_HistColor *esHistC = es->ex->state;
	switch (esHistC->colorSpace) {
		case CS_RGB:
		return -1;
		case CS_HSV:
		return CV_RGB2HSV;
		case CS_HLS:
		return CV_RGB2HLS;
		case CS_LAB:
		return CV_RGB2Lab;
		case CS_LUV:
		return CV_RGB2Luv;
		case CS_XYZ:
		return CV_RGB2XYZ;
		case CS_YCrCb:
		return CV_RGB2YCrCb;
		default:
		my_log_info("unknown colorspace %"PRIi64"\n", esHistC->colorSpace);
		return 0;
	}
}
static void calculateBackground(struct Proceso_HIST *es) {
	IplImage *bg_rgb = cvCreateImage(cvGetSize(es->img_cshistOne), IPL_DEPTH_8U,
			3);
	if (USE_PLAIN_BACKGROUND) {
		CvScalar colRgb = cvScalar(255, 255, 255, 0);
		limpiarImg_color(bg_rgb, colRgb);
	} else {
		CvScalar colRgb1 = cvScalar(250, 200, 200, 0);
		CvScalar colRgb2 = cvScalar(200, 250, 200, 0);
		limpiarImg_cuadriculadoColor(bg_rgb, colRgb1, colRgb2);
	}
	int64_t code = getCvtCode_rgb_cshist(es);
	if (code == -1) {
		es->bg_cshistOne = bg_rgb;
	} else {
		es->bg_cshistOne = cvCreateImage(cvGetSize(bg_rgb), IPL_DEPTH_8U, 3);
		cvCvtColor(bg_rgb, es->bg_cshistOne, code);
		releaseImagen(bg_rgb);
	}
}
static void tra_new_hist(const char *trCode, const char *trParameters, void **out_state) {
	struct Proceso_HIST *es = MY_MALLOC(1, struct Proceso_HIST);
	if (my_string_equals(trCode,"HISTG"))
	es->isHistG = 1;
	else if (my_string_equals(trCode,"HIST1X3D"))
	es->isHist1x3D = 1;
	else if (my_string_equals(trCode,"HIST3X1D"))
	es->isHist3x1D = 1;
	es->ex = getExtractor2(trCode, trParameters);
	*out_state = es;
}
static void tra_init_hist(IplImage *imagen, void *estado) {
	struct Proceso_HIST *es = estado;
	if (es->isHistG) {
		struct State_HistGray *esHistG = es->ex->state;
		es->zonesW = esHistG->numZonesW;
		es->zonesH = esHistG->numZonesH;
		es->totalBinsHist = esHistG->numBins;
		es->lengthDesc = es->zonesW * es->zonesH * es->totalBinsHist;
		es->colBins = getColorBins_gray(esHistG);
	} else {
		struct State_HistColor *esHistC = es->ex->state;
		es->zonesW = esHistC->numZonesW;
		es->zonesH = esHistC->numZonesH;
		es->totalBinsHist = esHistC->totalBins;
		es->lengthDesc = es->zonesW * es->zonesH * es->totalBinsHist;
		es->colBins =
		es->isHist1x3D ?
		getColorBins_1x3d(esHistC) : getColorBins_3x1d(esHistC);
	}
	int64_t w = imagen->width / es->zonesW;
	int64_t h = imagen->height / es->zonesH;
	if (w < es->totalBinsHist)
	w = es->totalBinsHist;
	if (h < 50)
	h = 50;
	w = round_int(w * SCALE_ZONE_W);
	h = round_int(h * SCALE_ZONE_H);
	es->img_cshistOne = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	calculateBackground(es);
	w *= es->zonesW;
	h *= es->zonesH;
	es->img_cshistAll = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	es->img_bgrAll = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
	es->histogram = MY_MALLOC_NOINIT(es->lengthDesc, double);
	if (PRINT_HIST) {
		int64_t i;
		for (i = 0; i < es->totalBinsHist; i++) {
			my_log_info(
					"bin %3"PRIi64") low=%3u %3u %3u  high=%3u %3u %3u  avg=%3u %3u %3u\n",
					i, es->colBins[i].col_low[0], es->colBins[i].col_low[1],
					es->colBins[i].col_low[2], es->colBins[i].col_high[0],
					es->colBins[i].col_high[1], es->colBins[i].col_high[2],
					es->colBins[i].col_avg[0], es->colBins[i].col_avg[1],
					es->colBins[i].col_avg[2]);
		}
	}
}
static void generateImgHistogram(struct Proceso_HIST *es, double *histogram,
		double max_value) {
	IplImage *img = es->img_cshistOne;
	int64_t hist_height = img->height;
	int64_t hist_width = img->width;
	int64_t init_x = 0;
	int64_t bottom_y = img->height - 1;
	if (USE_BLACK_FRAME) {
		hist_height -= 2 * BLACK_FRAME_SIZE;
		hist_width -= 2 * BLACK_FRAME_SIZE;
		init_x = BLACK_FRAME_SIZE;
		bottom_y -= BLACK_FRAME_SIZE;
	}
	int64_t bin_width = hist_width / es->totalBinsHist;
	if (USE_FOOTER)
	hist_height -= FOOTER_SIZE;
	int64_t i, pos_x = init_x;
	for (i = 0; i < es->totalBinsHist; i++) {
		double bin_val = (histogram[i] / max_value) * hist_height;
		int64_t height_bin = my_math_ceil_int(bin_val);
		if (height_bin > hist_height)
		height_bin = hist_height;
		if (height_bin < 0)
		height_bin = 0;
		//degradation inside a bin
		int64_t j;
		for (j = 0; j < bin_width; ++j) {
			CvScalar col = cvScalarAll(0);
			if (USE_DEGRADATION) {
				double fraction = (j == 0) ? 0 : j / ((double) bin_width - 1);
				int64_t k;
				for (k = 0; k < 3; ++k) //by channel
				col.val[k] = es->colBins[i].col_low[k]
				+ (fraction
						* (es->colBins[i].col_high[k]
								- es->colBins[i].col_low[k]));
			} else {
				col.val[0] = es->colBins[i].col_avg[0];
				col.val[1] = es->colBins[i].col_avg[1];
				col.val[2] = es->colBins[i].col_avg[2];
			}
			CvPoint posBottom = cvPoint(pos_x, bottom_y);
			CvPoint posTop = cvPoint(pos_x, bottom_y - height_bin + 1);
			if (USE_FOOTER)
			posTop.y -= FOOTER_SIZE;
			cvRectangle(img, posBottom, posTop, col, CV_FILLED, 8, 0);
			pos_x += 1;
		}
	}
	if (USE_FOOTER) {
		bottom_y -= FOOTER_SIZE - 1;
		cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
				cvScalarAll(0), 1, 8, 0);
		bottom_y++;
		cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
				cvScalarAll(0), 1, 8, 0);
		bottom_y++;
		cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
				cvScalarAll(0), 1, 8, 0);
	}
	if (USE_BLACK_FRAME) {
		int64_t dx = 0, dy = 0, hx = pos_x + BLACK_FRAME_SIZE - 1, hy =
		img->height - 1;
		int64_t w;
		for (w = 0; w < BLACK_FRAME_SIZE; ++w) {
			CvPoint a = cvPoint(dx + w, dy + w);
			CvPoint b = cvPoint(dx + w, hy - w);
			CvPoint c = cvPoint(hx - w, hy - w);
			CvPoint d = cvPoint(hx - w, dy + w);
			cvLine(img, a, b, cvScalarAll(0), 1, 8, 0);
			cvLine(img, b, c, cvScalarAll(0), 1, 8, 0);
			cvLine(img, c, d, cvScalarAll(0), 1, 8, 0);
			cvLine(img, a, d, cvScalarAll(0), 1, 8, 0);
		}
	}
}
static IplImage *tra_transformar_hist(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_HIST *es = estado;
	void *descriptor = extractor_getDescriptor_IplImage(es->ex, imagen);
	histogram2double(es, descriptor, es->histogram);
	if (PRINT_HIST) {
		char *st = my_newString_arrayDouble(es->histogram, es->lengthDesc, ' ');
		my_log_info("hist %s\n", st);
		MY_FREE(st);
	}
	double *histogram = es->histogram;
	double max_value = maxVal(histogram, es->lengthDesc);
	int64_t i, j;
	for (j = 0; j < es->zonesH; ++j) {
		for (i = 0; i < es->zonesW; ++i) {
			cvCopy(es->bg_cshistOne, es->img_cshistOne, NULL );
			//double max_value = maxVal(histogram, es->totalBinsHist);
			generateImgHistogram(es, histogram, max_value);
			histogram += es->totalBinsHist;
			cvSetImageROI(es->img_cshistAll,
					cvRect(es->img_cshistOne->width * i,
							es->img_cshistOne->height * j,
							es->img_cshistOne->width,
							es->img_cshistOne->height));
			cvCopy(es->img_cshistOne, es->img_cshistAll, NULL );
			cvResetImageROI(es->img_cshistAll);
		}
	}
	int code = getCvtCode_cshist_bgr(es);
	cvCvtColor(es->img_cshistAll, es->img_bgrAll, code);
	return es->img_bgrAll;
}
static void tra_release_hist(void *estado) {
	struct Proceso_HIST *es = estado;
	releaseImagen(es->img_cshistOne);
	releaseImagen(es->img_cshistAll);
	releaseImagen(es->img_bgrAll);
	MY_FREE(es->colBins);
	releaseExtractor(es->ex );
	MY_FREE(es);
}
static void tra_base_def(char *extCode, char *helpText) {
	Transform_Def *def = newTransformDef(extCode, helpText);
	def->func_new = tra_new_hist;
	def->func_init = tra_init_hist;
	def->func_transform_frame = tra_transformar_hist;
	def->func_release = tra_release_hist;
}
#endif
struct State_HIST {
	Extractor *ex;
	double *histogram_full;
	struct ColorBin *colorBins;
	MyImageColor *converter;
	int64_t descriptor_length, numZones, numBins;
	IplImage *image_empty, *image;
	struct Zoning *zoning;
};
static IplImage *newEmptyImage(int64_t full_width, int64_t full_height) {
	IplImage *image = cvCreateImage(cvSize(full_width, full_height),
	IPL_DEPTH_8U, 3);
	if (0) {
		CvScalar colRgb = cvScalar(255, 255, 255, 0);
		my_image_fill_color(image, colRgb);
	} else {
		CvScalar colRgb1 = cvScalar(250, 200, 200, 0);
		CvScalar colRgb2 = cvScalar(200, 250, 200, 0);
		my_image_fill_squares_color(image, colRgb1, colRgb2);
	}
	/*
	 int64_t code = getCvtCode_rgb_cshist(es);
	 if (code == -1) {
	 es->bg_cshistOne = bg_rgb;
	 } else {
	 es->bg_cshistOne = cvCreateImage(cvGetSize(bg_rgb), IPL_DEPTH_8U, 3);
	 cvCvtColor(bg_rgb, es->bg_cshistOne, code);
	 releaseImagen(bg_rgb);
	 }*/
	return image;
}

static void tra_new_histGray(const char *trCode, const char *trParameters,
		void **out_state_tr) {
	struct State_HIST *state = MY_MALLOC(1, struct State_HIST);
	state->ex = getExtractor2(trCode, trParameters);
	DescriptorType td = getDescriptorType(state->ex);
	state->descriptor_length = td.array_length;
	state->numZones = td.num_subarrays;
	state->numBins = td.subarray_length;
	state->histogram_full = MY_MALLOC_NOINIT(state->descriptor_length, double);
	char *zoning_text = NULL;
	if (my_string_equals("HISTGRAY", trCode)) {
		state->colorBins = getColorBins_gray(getExtractorState(state->ex));
		zoning_text = getImageZoningText_gray(getExtractorState(state->ex));
	} else if (my_string_equals("HISTCOLOR", trCode)) {
		state->colorBins = getColorBins_color(getExtractorState(state->ex));
		zoning_text = getImageZoningText_color(getExtractorState(state->ex));
		state->converter = my_imageColor_newConverter(
				getColorSpace_color(getExtractorState(state->ex)));
	} else if (my_string_equals("HISTBYCHANNEL", trCode)) {
		state->colorBins = getColorBins_byChannel(getExtractorState(state->ex));
		zoning_text = getImageZoningText_byChannel(
				getExtractorState(state->ex));
		state->converter = my_imageColor_newConverter(
				getColorSpace_byChannel(getExtractorState(state->ex)));
	} else if (my_string_equals("GDH", trCode)) {
		state->colorBins = getColorBins_gradient(getExtractorState(state->ex));
		zoning_text = getImageZoningText_gradient(getExtractorState(state->ex));
	} else {
		my_log_error("unknown %s\n", trCode);
	}
	state->zoning = parseZoning(zoning_text);
	*out_state_tr = state;
}

#define FOOTER_SIZE 10
#define BLACK_FRAME_SIZE 3

static CvScalar getColumnColor(int64_t i, int64_t j, int64_t bin_width,
		struct ColorBin *color_bins, bool withColorDegradation) {
	CvScalar col = cvScalarAll(0);
	if (withColorDegradation) {
		double fraction = (j == 0) ? 0 : j / ((double) bin_width - 1);
		for (int64_t k = 0; k < 3; ++k) //by channel
			col.val[k] = color_bins[i].col_low[k]
					+ (fraction
							* (color_bins[i].col_high[k]
									- color_bins[i].col_low[k]));
	} else {
		col.val[0] = color_bins[i].col_avg[0];
		col.val[1] = color_bins[i].col_avg[1];
		col.val[2] = color_bins[i].col_avg[2];
	}
	return col;
}

static void drawHistogram(IplImage *img, CvRect roi, int64_t num_bins,
		double *bins, struct ColorBin *color_bins, bool withFooter,
		bool withColorDegradation,
		bool withBlackFrame) {
	int64_t image_height = roi.height;
	int64_t image_width = roi.width;
	int64_t init_x = roi.x;
	int64_t bottom_y = roi.y + roi.height - 1;
	if (withBlackFrame) {
		image_height -= 2 * BLACK_FRAME_SIZE;
		image_width -= 2 * BLACK_FRAME_SIZE;
		init_x += BLACK_FRAME_SIZE;
		bottom_y -= BLACK_FRAME_SIZE;
	}
	int64_t bin_width = image_width / num_bins;
	if (withFooter)
		image_height -= FOOTER_SIZE;
	int64_t pos_x = init_x;
	for (int64_t i = 0; i < num_bins; i++) {
		double bin_val = bins[i] * image_height;
		int64_t height_bin = my_math_ceil_int(bin_val);
		if (height_bin > image_height)
			height_bin = image_height;
		if (height_bin < 0)
			height_bin = 0;
		for (int64_t j = 0; j < bin_width; ++j) {
			CvPoint columnBottom = cvPoint(pos_x, bottom_y);
			CvPoint columnTop = cvPoint(pos_x, bottom_y - height_bin + 1);
			if (withFooter)
				columnTop.y -= FOOTER_SIZE;
			CvScalar col = getColumnColor(i, j, bin_width, color_bins,
					withColorDegradation);
			cvRectangle(img, columnBottom, columnTop, col, CV_FILLED, 8, 0);
			pos_x += 1;
		}
	}
	if (withFooter) {
		bottom_y -= FOOTER_SIZE - 1;
		cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
				cvScalarAll(0), 1, 8, 0);
		bottom_y++;
		cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
				cvScalarAll(0), 1, 8, 0);
		//bottom_y++;
		//cvLine(img, cvPoint(init_x, bottom_y), cvPoint(pos_x - 1, bottom_y),
		//	cvScalarAll(0), 1, 8, 0);
	}
	if (withBlackFrame) {
		int64_t dx = roi.x, dy = roi.y;
		int64_t hx = pos_x + BLACK_FRAME_SIZE - 1;
		int64_t hy = roi.y + roi.height - 1;
		int64_t w;
		for (w = 0; w < BLACK_FRAME_SIZE; ++w) {
			CvPoint a = cvPoint(dx + w, dy + w);
			CvPoint b = cvPoint(dx + w, hy - w);
			CvPoint c = cvPoint(hx - w, hy - w);
			CvPoint d = cvPoint(hx - w, dy + w);
			cvLine(img, a, b, cvScalarAll(0), 1, 8, 0);
			cvLine(img, b, c, cvScalarAll(0), 1, 8, 0);
			cvLine(img, c, d, cvScalarAll(0), 1, 8, 0);
			cvLine(img, a, d, cvScalarAll(0), 1, 8, 0);
		}
	}
}
void printHistogram(double *histogram_full, int64_t numZones, int64_t numBins) {
	int64_t n, i, pos = 0;
	for (n = 0; n < numZones; ++n) {
		MyStringBuffer *sb = my_stringbuf_new();
		for (i = 0; i < numBins; ++i) {
			char *st = my_newString_format(" %1.2lf", histogram_full[pos++]);
			my_stringbuf_appendString(sb, st);
			MY_FREE(st);
		}
		my_log_info("hist %"PRIi64")%s\n", n,
				my_stringbuf_getCurrentBuffer(sb));
		my_stringbuf_release(sb);
	}
}
static void histGray_processZone(IplImage *image, struct Zone *zone,
		void *state_tr) {
	struct State_HIST *state = state_tr;
	double *histogram = state->histogram_full + zone->numZone * state->numBins;
	drawHistogram(state->image, zone->rect, state->numBins, histogram,
			state->colorBins, true, true, false);
}
static IplImage* tra_transform_histGray(IplImage *image, int64_t numFrame,
		void* state_tr) {
	struct State_HIST *state = state_tr;
	void *descriptor = extractVolatileDescriptor(state->ex, image);
	descriptor2doubleArray(getDescriptorType(state->ex), descriptor,
			state->histogram_full);
	//printHistogram(state->histogram_full, state->numZones, state->numBins);
	int64_t i;
	for (i = 0; i < state->numZones; ++i)
		my_math_normalizeMax1_double(state->histogram_full + i * state->numBins,
				state->numBins);
	if (state->image_empty == NULL) {
		int64_t full_width = image->width;
		int64_t full_height = image->height;
		if (state->numZones > 1) {
			full_width *= 1;
			full_height *= 1;
		}
		state->image_empty = newEmptyImage(full_width, full_height);
		state->image = my_image_duplicate(state->image_empty);
	} else {
		cvCopy(state->image_empty, state->image, NULL);
	}
	processZoning(state->image, state->zoning, histGray_processZone, state);
	if (state->converter != NULL) {
		return my_imageColor_convertToBGR(state->image, state->converter);
	} else {
		return state->image;
	}
}
static void tra_release_histGray(void *state_tr) {
	struct State_HIST *state = state_tr;
	releaseExtractor(state->ex);
	my_image_release(state->image_empty);
	my_image_release(state->image);
	MY_FREE(state);
}
static void reg(const char *code) {
	const char *help = getExtractorHelp(code);
	Transform_Def *def = newTransformDef(code, help);
	def->func_new = tra_new_histGray;
	def->func_transform_frame = tra_transform_histGray;
	def->func_release = tra_release_histGray;
}
void tra_reg_hist() {
	reg("HISTGRAY");
	reg("HISTCOLOR");
	reg("HISTBYCHANNEL");
	reg("GDH");
}
#endif
