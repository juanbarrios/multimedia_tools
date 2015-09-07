/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_EXTRACT_OPENCV_H_
#define MYUTILSIMAGE_LOCAL_EXTRACT_OPENCV_H_

#include "../../../myutilsimage_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

typedef void MyOpencvOptions;

MyOpencvOptions *my_local_opencv_newOptionsParse(const char *parameters);

const char *my_local_opencv_helpOptions();

void my_local_opencv_releaseOptions(MyOpencvOptions *options);

void my_local_opencv_extract(IplImage *image, MyLocalDescriptors *descriptors,
		MyOpencvOptions *options);
#endif

#ifdef __cplusplus
}
#endif

#endif
