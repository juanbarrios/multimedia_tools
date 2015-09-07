/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_IMAGE_STITCHING_H
#define MYUTILSIMAGE_IMAGE_STITCHING_H

#include "../myutilsimage_c.h"

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>

IplImage *generateStitching(IplImage *img_src1, IplImage *img_src2,
		CvMat *tmatrix, uchar with_borders, double blend_weight);

#endif
#endif
