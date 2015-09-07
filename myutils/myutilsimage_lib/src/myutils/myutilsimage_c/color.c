/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "color.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV
#include <opencv2/imgproc/imgproc_c.h>

MY_MUTEX_NEWSTATIC(mutex_cs);
static struct MyColorSpace *priv_colorSpaces = NULL;

#define CS_UID_GRAY 0
#define CS_UID_LAB 4
#define CS_UID_RGB 1

static void cs(int64_t uid, const char *code, int64_t openCvCode_bgr2this,
		int64_t openCvCode_this2bgr, int64_t numChannels, int64_t channel0_min,
		int64_t channel0_max, int64_t channel1_min, int64_t channel1_max,
		int64_t channel2_min, int64_t channel2_max) {
	struct MyColorSpace c;
	c.uid = uid;
	c.code = code;
	c.openCvCode_bgr2this = openCvCode_bgr2this;
	c.openCvCode_this2bgr = openCvCode_this2bgr;
	c.numChannels = numChannels;
	c.channel0_min = channel0_min;
	c.channel0_max = channel0_max;
	c.channel1_min = channel1_min;
	c.channel1_max = channel1_max;
	c.channel2_min = channel2_min;
	c.channel2_max = channel2_max;
	priv_colorSpaces[uid] = c;
}
static void createColorSpaces() {
	MY_MUTEX_LOCK(mutex_cs);
	if (priv_colorSpaces == NULL) {
		priv_colorSpaces = MY_MALLOC(10, struct MyColorSpace);
		cs(CS_UID_GRAY, "GRAY", CV_BGR2GRAY, CV_GRAY2BGR, 1, 0, 256, 0, 0, 0,
				0);
		cs(CS_UID_RGB, "RGB", CV_BGR2RGB, CV_RGB2BGR, 3, 0, 256, 0, 256, 0,
				256);
		cs(2, "HSV", CV_BGR2HSV, CV_HSV2BGR, 3, 0, 180, 0, 256, 0, 256);
		cs(3, "HLS", CV_BGR2HLS, CV_HLS2BGR, 3, 0, 180, 0, 256, 0, 256);
		cs(CS_UID_LAB, "LAB", CV_BGR2Lab, CV_Lab2BGR, 3, 0, 256, 0, 256, 0,
				256);
		cs(5, "LUV", CV_BGR2Luv, CV_Luv2BGR, 3, 0, 256, 0, 256, 0, 256);
		cs(6, "XYZ", CV_BGR2XYZ, CV_XYZ2BGR, 3, 0, 256, 0, 256, 0, 256);
		cs(7, "YCrCb", CV_BGR2YCrCb, CV_YCrCb2BGR, 3, 0, 256, 0, 256, 0, 256);
	}
	MY_MUTEX_UNLOCK(mutex_cs);
}
static struct MyColorSpace priv_getColorspace(int64_t uid) {
	createColorSpaces();
	return priv_colorSpaces[uid];
}
struct MyColorSpace my_colorSpace_getColorSpace(const char *name) {
	createColorSpaces();
	int64_t i;
	for (i = 0; i < 8; ++i) {
		if (my_string_equals_ignorecase(priv_colorSpaces[i].code, name))
			return priv_colorSpaces[i];
	}
	my_log_error("unknown colorspace %s\n", name);
	return priv_colorSpaces[0];
}

struct MyImageColor {
	struct MyColorSpace colorSpace;
	IplImage *imgBuffer;
};
MyImageColor* my_imageColor_newConverter(struct MyColorSpace colorSpace) {
	struct MyImageColor *converter = MY_MALLOC(1,
			struct MyImageColor);
	converter->colorSpace = colorSpace;
	return converter;
}
MyImageColor* my_imageColor_newConverterToGray() {
	return my_imageColor_newConverter(priv_getColorspace(CS_UID_GRAY));
}
void my_imageColor_release(MyImageColor *converter) {
	my_image_release(converter->imgBuffer);
	MY_FREE(converter);
}
IplImage* my_imageColor_convertFromBGR(IplImage *imageSrc, MyImageColor *converter) {
	if (converter->imgBuffer == NULL
			|| imageSrc->width != converter->imgBuffer->width
			|| imageSrc->height != converter->imgBuffer->height
			|| imageSrc->depth != converter->imgBuffer->depth) {
		if (converter->imgBuffer != NULL)
			cvReleaseImage(&converter->imgBuffer);
		converter->imgBuffer = cvCreateImage(cvGetSize(imageSrc),
				imageSrc->depth, converter->colorSpace.numChannels);
	}
	cvCvtColor(imageSrc, converter->imgBuffer,
			converter->colorSpace.openCvCode_bgr2this);
	return converter->imgBuffer;
}
IplImage* my_imageColor_convertToBGR(IplImage *imageSrc, MyImageColor *converter) {
	if (converter->imgBuffer == NULL
			|| imageSrc->width != converter->imgBuffer->width
			|| imageSrc->height != converter->imgBuffer->height) {
		if (converter->imgBuffer != NULL)
			cvReleaseImage(&converter->imgBuffer);
		converter->imgBuffer = cvCreateImage(cvGetSize(imageSrc),
				imageSrc->depth, 3);
	}
	cvCvtColor(imageSrc, converter->imgBuffer,
			converter->colorSpace.openCvCode_this2bgr);
	return converter->imgBuffer;
}
static CvScalar convertOneColor(CvScalar inputColor,
		struct MyColorSpace inputColorSpace, struct MyColorSpace outputColorSpace) {
	if (inputColorSpace.uid == outputColorSpace.uid)
		return inputColor;
	IplImage *ttSrc = cvCreateImage(cvSize(1, 1), IPL_DEPTH_64F,
			inputColorSpace.numChannels);
	IplImage *ttBgr = cvCreateImage(cvSize(1, 1), IPL_DEPTH_64F, 3);
	IplImage *ttDst = cvCreateImage(cvSize(1, 1), IPL_DEPTH_64F,
			outputColorSpace.numChannels);
	double *ptrS = (double*) ttSrc->imageData;
	ptrS[0] = inputColor.val[0];
	if (inputColorSpace.numChannels >= 1)
		ptrS[1] = inputColor.val[1];
	if (inputColorSpace.numChannels >= 2)
		ptrS[2] = inputColor.val[2];
	if (inputColorSpace.numChannels >= 3)
		ptrS[3] = inputColor.val[3];
	cvCvtColor(ttSrc, ttBgr, inputColorSpace.openCvCode_this2bgr);
	cvCvtColor(ttBgr, ttDst, outputColorSpace.openCvCode_bgr2this);
	double *ptrD = (double*) ttDst->imageData;
	CvScalar col = cvScalarAll(0);
	col.val[0] = ptrD[0];
	if (inputColorSpace.numChannels >= 1)
		col.val[1] = ptrD[1];
	if (inputColorSpace.numChannels >= 2)
		col.val[2] = ptrD[2];
	if (inputColorSpace.numChannels >= 2)
		col.val[3] = ptrD[3];
	my_image_release(ttSrc);
	my_image_release(ttBgr);
	my_image_release(ttDst);
	return col;
}
static CvScalar convertToLab(CvScalar colorSrc,
		struct MyColorSpace colorSpaceInput) {
	CvScalar colLab = convertOneColor(colorSrc, colorSpaceInput,
			priv_getColorspace(CS_UID_LAB));
	CvScalar col = cvScalar(colLab.val[0] * 100.0 / 255.0, colLab.val[1] - 128,
			colLab.val[2] - 128, 0);
	return col;
}
double **my_colorSpace_computeDistancesInCIELAB(CvScalar *colors, int64_t num_colors,
		struct MyColorSpace colorSpaceInput) {
	my_log_info("calculating matrix %"PRIi64" CIE\n", num_colors);
	int64_t i, j;
	double **matrix = MY_MALLOC_MATRIX(num_colors, num_colors, double);
	//euclidean distances between colors in LAB colorspace
	for (i = 0; i < num_colors; ++i) {
		CvScalar colorLab1 = convertToLab(colors[i], colorSpaceInput);
		for (j = 0; j < num_colors; ++j) {
			CvScalar colorLab2 = convertToLab(colors[j], colorSpaceInput);
			double d1 = colorLab1.val[0] - colorLab2.val[0];
			double d2 = colorLab1.val[1] - colorLab2.val[1];
			double d3 = colorLab1.val[2] - colorLab2.val[2];
			matrix[i][j] = sqrt(d1 * d1 + d2 * d2 + d3 * d3);
		}
	}
	return matrix;
}
#endif
