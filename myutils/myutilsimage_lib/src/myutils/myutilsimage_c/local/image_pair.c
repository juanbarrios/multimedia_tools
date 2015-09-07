/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "image_pair.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV

struct MyImagePair {
	CvRect zone1;
	CvRect zone2;
	bool isWhiteBackgroud;
	IplImage *image;
};

MyImagePair *my_imagePair_new(IplImage *image1, IplImage *image2, int join_type,
		int64_t margin, bool isWhiteBackgroud) {
	CvSize size1 = cvGetSize(image1);
	CvSize size2 = cvGetSize(image2);
	CvSize new_size = cvSize(0, 0);
	int64_t offset_x1 = 0, offset_y1 = 0, offset_x2 = 0, offset_y2 = 0;
	if (join_type == MY_IMAGEPAIR_HORIZONTAL_TOP
			|| join_type == MY_IMAGEPAIR_HORIZONTAL_MIDDLE
			|| join_type == MY_IMAGEPAIR_HORIZONTAL_BOTTOM) {
		new_size.width = size1.width + size2.width + margin;
		new_size.height = MAX(size1.height, size2.height);
		offset_x2 = size1.width + margin;
		int64_t dy = 0;
		if (join_type == MY_IMAGEPAIR_HORIZONTAL_BOTTOM)
			dy = MAX(size1.height, size2.height) - MIN(size1.height,
					size2.height);
		else if (join_type == MY_IMAGEPAIR_HORIZONTAL_MIDDLE)
			dy = (MAX(size1.height, size2.height)
					- MIN(size1.height, size2.height)) / 2;
		if (MIN(size1.height, size2.height) == size1.height)
			offset_y1 = dy;
		else
			offset_y2 = dy;
	} else if (join_type == MY_IMAGEPAIR_VERTICAL_LEFT
			|| join_type == MY_IMAGEPAIR_VERTICAL_CENTER
			|| join_type == MY_IMAGEPAIR_VERTICAL_RIGHT) {
		new_size.width = MAX(size1.width, size2.width);
		new_size.height = size1.height + size2.height + margin;
		offset_y2 = size1.height + margin;
		int64_t dx = 0;
		if (join_type == MY_IMAGEPAIR_VERTICAL_RIGHT)
			dx = MAX(size1.width, size2.width) - MIN(size1.width, size2.width);
		else if (join_type == MY_IMAGEPAIR_VERTICAL_CENTER)
			dx = (MAX(size1.width, size2.width) - MIN(size1.width, size2.width))
					/ 2;
		if (MIN(size1.width, size2.width) == size1.width)
			offset_x1 = dx;
		else
			offset_x2 = dx;
	}
	if (image1->nChannels != image2->nChannels)
		my_log_error("images must have equal channels\n");
	if (image1->depth != image2->depth)
		my_log_error("images must have equal depth\n");
	MyImagePair* image_pair = MY_MALLOC_NOINIT(1, MyImagePair);
	image_pair->zone1 = cvRect(offset_x1, offset_y1, size1.width, size1.height);
	image_pair->zone2 = cvRect(offset_x2, offset_y2, size2.width, size2.height);
	image_pair->isWhiteBackgroud = isWhiteBackgroud;
	image_pair->image = cvCreateImage(new_size, image1->depth,
			image1->nChannels);
	my_imagePair_updateImages(image_pair, image1, image2);
	return image_pair;
}
void my_imagePair_updateImages(MyImagePair *image_pair, IplImage *image1,
		IplImage *image2) {
	IplImage *newImg = image_pair->image;
	if (image_pair->isWhiteBackgroud)
		my_image_fill_white(newImg);
	else
		my_image_fill_squares(newImg);
	cvSetImageROI(newImg, image_pair->zone1);
	cvCopy(image1, newImg, NULL);
	cvRectangleR(newImg, image_pair->zone1, cvScalarAll(0), 1, 8, 0);
	cvSetImageROI(newImg, image_pair->zone2);
	cvCopy(image2, newImg, NULL);
	cvRectangleR(newImg, image_pair->zone2, cvScalarAll(0), 1, 8, 0);
	cvResetImageROI(newImg);
}

IplImage *my_imagePair_getIplImage(MyImagePair *image_pair) {
	return image_pair->image;
}

static int my_imagePair_getMatchLineWidth() {
	return 2;
}
static CvScalar my_imagePair_getMatchRandomColor() {
	int64_t r = my_random_int(160, 240);
	int64_t g = my_random_int(160, 240);
	int64_t b = my_random_int(160, 240);
	return cvScalar(b, g, r, 0);
}

void my_imagePair_drawMatches(MyImagePair* image_pair,
		MyLocalDescriptors *descOrigen, MyLocalDescriptors *descDestino,
		int64_t numPairs, int64_t *idSrc, int64_t *idDes) {
	for (int64_t i = 0; i < numPairs; ++i) {
		struct MyLocalKeypoint pos1 = my_localDescriptors_getKeypoint(
				descOrigen, idSrc[i]);
		struct MyLocalKeypoint pos2 = my_localDescriptors_getKeypoint(
				descDestino, idDes[i]);
		int ix = image_pair->zone1.x + my_math_round_int(pos1.x);
		int iy = image_pair->zone1.y + my_math_round_int(pos1.y);
		int dx = image_pair->zone2.x + my_math_round_int(pos2.x);
		int dy = image_pair->zone2.y + my_math_round_int(pos2.y);
		cvLine(image_pair->image, cvPoint(ix, iy), cvPoint(dx, dy),
				my_imagePair_getMatchRandomColor(),
				my_imagePair_getMatchLineWidth(), 8, 0);
	}
}
void my_imagePair_drawKeypoints(MyImagePair* image_pair,
		MyLocalDescriptors *descriptor1, MyLocalDescriptors *descriptor2) {
	my_local_drawKeypoints_offset(image_pair->image, descriptor1,
			image_pair->zone1.x, image_pair->zone1.y);
	my_local_drawKeypoints_offset(image_pair->image, descriptor2,
			image_pair->zone2.x, image_pair->zone2.y);
}

void my_imagePair_release(MyImagePair* image_pair) {
	my_image_release(image_pair->image);
	free(image_pair);
}
IplImage *my_imagePair_release_returnIplImage(MyImagePair* image_pair) {
	IplImage *img = image_pair->image;
	free(image_pair);
	return img;
}
#endif
