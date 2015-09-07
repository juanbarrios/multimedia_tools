/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_IMAGE_PAIR_H
#define MYUTILSIMAGE_LOCAL_IMAGE_PAIR_H

#include "local.h"

#define MY_IMAGEPAIR_HORIZONTAL_TOP 1
#define MY_IMAGEPAIR_HORIZONTAL_MIDDLE 2
#define MY_IMAGEPAIR_HORIZONTAL_BOTTOM 3
#define MY_IMAGEPAIR_VERTICAL_LEFT 4
#define MY_IMAGEPAIR_VERTICAL_CENTER 5
#define MY_IMAGEPAIR_VERTICAL_RIGHT 6

typedef struct MyImagePair MyImagePair;

#ifndef NO_OPENCV

MyImagePair *my_imagePair_new(IplImage *image1, IplImage *image2, int join_type,
		int64_t margin, bool isWhiteBackgroud);

void my_imagePair_updateImages(MyImagePair *image_pair, IplImage *image1,
		IplImage *image2);

IplImage *my_imagePair_getIplImage(MyImagePair *image_pair);

void my_imagePair_drawMatches(MyImagePair* image_pair,
		MyLocalDescriptors *descOrigen, MyLocalDescriptors *descDestino,
		int64_t numPairs, int64_t *idSrc, int64_t *idDes);

void my_imagePair_drawKeypoints(MyImagePair* image_pair,
		MyLocalDescriptors *descriptor1, MyLocalDescriptors *descriptor2);

void my_imagePair_release(MyImagePair* image_pair);

IplImage *my_imagePair_release_returnIplImage(MyImagePair* image_pair);

#endif
#endif

