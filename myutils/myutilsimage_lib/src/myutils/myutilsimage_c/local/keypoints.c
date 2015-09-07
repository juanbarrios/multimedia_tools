/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "keypoints.h"

struct MyLocalKeypoint myLocalKeypoint(double x, double y) {
	struct MyLocalKeypoint kp = { x, y, 0, 0 };
	return kp;
}

#define FIXED_RADIUS 5
#define THICKNESS 2
#define RADIUS_ADD 0
#define RADIUS_SCALE 1

#ifndef NO_OPENCV

void my_local_drawKeypoint(IplImage* img, struct MyLocalKeypoint kp) {
	int px = my_math_round_int(kp.x);
	int py = my_math_round_int(kp.y);
	if (px == img->width)
		px = img->width - 1;
	if (py == img->height)
		py = img->height - 1;
	if (!MY_BETWEEN(px, 0,
			img->width - 1) || !MY_BETWEEN(py, 0, img->height-1)) {
		my_log_info("point (%i,%i) out of range %ix%i\n", px, py, img->width,
				img->height);
		return;
	}
	CvPoint orig = cvPoint(px, py);
	if (kp.radius == 0) {
		cvCircle(img, orig, FIXED_RADIUS, cvScalar(0, 255, 255, 255), THICKNESS,
				8, 0);
	} else {
		double radius = (RADIUS_ADD + kp.radius) * RADIUS_SCALE;
		if (radius < 2)
			radius = 2;
		int rad = my_math_round_int(radius);
		cvCircle(img, orig, rad, cvScalar(0, 255, 255, 0),
		THICKNESS, 8, 0);
		if (kp.angle != 0) {
			int dx = my_math_round_int(radius * cos(kp.angle));
			int dy = my_math_round_int(radius * sin(kp.angle));
			CvPoint dest = cvPoint(orig.x + dx, orig.y + dy);
			cvLine(img, orig, dest, cvScalar(0, 255, 255, 0), THICKNESS, 8, 0);
			cvCircle(img, dest, 1, cvScalar(0, 0, 255, 0), 1, 8, 0);
		}
		cvCircle(img, orig, 1, cvScalar(0, 255, 0, 0), 1, 8, 0);
	}
}

void my_local_drawKeypoints_offset(IplImage* img, MyLocalDescriptors *ldes,
		int64_t offset_x, int64_t offset_y) {
	for (int64_t i = 0; i < my_localDescriptors_getNumDescriptors(ldes); ++i) {
		struct MyLocalKeypoint kp = my_localDescriptors_getKeypoint(ldes, i);
		kp.x += offset_x;
		kp.y += offset_y;
		my_local_drawKeypoint(img, kp);
	}
}
void my_local_drawKeypoints(IplImage* img, MyLocalDescriptors *ldes) {
	my_local_drawKeypoints_offset(img, ldes, 0, 0);
}
#endif
