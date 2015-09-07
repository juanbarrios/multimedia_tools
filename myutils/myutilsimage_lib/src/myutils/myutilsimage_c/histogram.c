/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "histogram.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

static void image_drawBlackHistogram(IplImage *img, CvRect roi,
		int64_t num_bins, double *bins, int64_t step_hist, int64_t bin_width) {
	int64_t image_height = roi.height;
	//int64_t image_width = roi.width;
	int64_t init_x = roi.x;
	int64_t bottom_y = roi.y + roi.height - 1;
	int64_t pos_x = init_x;
	double norm_value = 0;
	for (int64_t i = 0; i < num_bins; ++i) {
		if (bins[i] > norm_value)
			norm_value = bins[i];
	}
	for (int64_t i = 0; i < num_bins; i += step_hist) {
		double bin_val = (bins[i] / norm_value) * image_height;
		int64_t height_bin = my_math_ceil_int(bin_val);
		if (height_bin > image_height)
			height_bin = image_height;
		if (height_bin < 0)
			height_bin = 0;
		for (int64_t j = 0; j < bin_width; ++j) {
			CvPoint columnBottom = cvPoint(pos_x, bottom_y);
			CvPoint columnTop = cvPoint(pos_x, bottom_y - height_bin + 1);
			cvRectangle(img, columnBottom, columnTop, cvScalarAll(0), CV_FILLED,
					8, 0);
			pos_x += 1;
		}
	}
}

#define IMAGE_MIN_WIDTH 500
#define IMAGE_MAX_WIDTH 1000
#define IMAGE_WH_RATIO 0.5

IplImage *my_image_newImageHistogram(int64_t num_bins, double *bins) {
	int64_t step_hist = 1;
	int64_t bin_width = 1;
	int64_t w = num_bins;
	while (w < IMAGE_MIN_WIDTH || w > IMAGE_MAX_WIDTH) {
		if (w < IMAGE_MIN_WIDTH) {
			bin_width++;
		} else if (w > IMAGE_MAX_WIDTH) {
			step_hist++;
		}
		w = (num_bins / step_hist) * bin_width;
	}
	CvSize size = cvSize(w, my_math_round_int(w * IMAGE_WH_RATIO));
	IplImage *img = cvCreateImage(size, IPL_DEPTH_8U, 1);
	my_image_fill_color(img, cvScalarAll(255));
	image_drawBlackHistogram(img, cvRect(0, 0, size.width, size.height),
			num_bins, bins, step_hist, bin_width);
	return img;
}

/*
 void saveHistogram(MknnHistogram *ch, char *filename) {
 if (ch == NULL)
 return;
 FILE *out = mknn_histogram_save(ch, filename);
 mknn_histogram_saveBins(ch, out);
 fclose(out);
 saveHistogramImg(ch, filename);
 }
 void saveHistogramImg(MknnHistogram *ch, char *filename) {
 IplImage *img = hist_drawHistogram(ch);
 char *f2 =
 my_string_endsWith(filename, ".png") ?
 my_newString_string(filename) : my_newString_concat(filename, ".png");
 my_log_info("writing %s\n", f2);
 if (!cvSaveImage(f2, img, 0))
 my_log_error("error saving %s\n", f2);
 my_image_release(img);
 MY_FREE(f2);
 }
 void showHistogram(MknnHistogram *ch) {
 IplImage *img = hist_drawHistogram(ch);
 cvNamedWindow("histogram", CV_WINDOW_AUTOSIZE);
 cvShowImage("histogram", img);
 cvWaitKey(0);
 cvDestroyWindow("histogram");
 cvReleaseImage(&img);
 }
 */
#endif
