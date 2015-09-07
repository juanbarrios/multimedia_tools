/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_COLOR_H
#define MYUTILSIMAGE_COLOR_H

#include "../myutilsimage_c.h"

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>

struct MyColorSpace {
	const char *code;
	int64_t openCvCode_bgr2this, openCvCode_this2bgr;
	int64_t numChannels;
	int64_t channel0_min, channel0_max;
	int64_t channel1_min, channel1_max;
	int64_t channel2_min, channel2_max;
	int64_t uid;
};
struct MyColorSpace my_colorSpace_getColorSpace(const char *name);

double **my_colorSpace_computeDistancesInCIELAB(CvScalar *colors,
		int64_t num_colors, struct MyColorSpace colorSpaceInput);

typedef struct MyImageColor MyImageColor;

MyImageColor* my_imageColor_newConverter(struct MyColorSpace colorSpace);
MyImageColor* my_imageColor_newConverterToGray();
IplImage* my_imageColor_convertFromBGR(IplImage *imageSrc,
		MyImageColor *converter);
IplImage* my_imageColor_convertToBGR(IplImage *imageSrc,
		MyImageColor *converter);
void my_imageColor_release(MyImageColor *converter);

#endif
#endif
