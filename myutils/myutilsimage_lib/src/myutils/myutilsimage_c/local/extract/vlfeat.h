/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_EXTRACT_VLFEAT_H
#define MYUTILSIMAGE_LOCAL_EXTRACT_VLFEAT_H

#include "../../../myutilsimage_c.h"

#ifndef NO_VLFEAT
#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

struct MyVlfeatOptions;

struct MyVlfeatOptions *my_local_vlfeat_newOptionsParse(const char *parameters);

const char *my_local_vlfeat_helpOptions();

void my_local_vlfeat_releaseOptions(struct MyVlfeatOptions *options);

void my_local_vlfeat_extract(IplImage *image, MyLocalDescriptors *descriptors,
		struct MyVlfeatOptions *options);

#endif
#endif

#endif
