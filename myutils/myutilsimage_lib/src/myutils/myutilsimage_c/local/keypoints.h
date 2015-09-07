/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_KEYPOINTS_H
#define MYUTILSIMAGE_LOCAL_KEYPOINTS_H

#include "local.h"

struct MyLocalKeypoint {
	double x;
	double y;
	double radius;
	double angle;
};
struct MyLocalKeypoint myLocalKeypoint(double x, double y);

#ifndef NO_OPENCV

void my_local_drawKeypoint(IplImage* img, struct MyLocalKeypoint kp);
void my_local_drawKeypoints_offset(IplImage* img, MyLocalDescriptors *ldes,
		int64_t offset_x, int64_t offset_y);
void my_local_drawKeypoints(IplImage* img, MyLocalDescriptors *ldes);

#endif

#endif
