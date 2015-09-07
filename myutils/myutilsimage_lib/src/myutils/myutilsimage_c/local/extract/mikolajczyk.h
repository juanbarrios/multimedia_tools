/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_EXTRACT_MIKOLAJCZYK_H
#define MYUTILSIMAGE_LOCAL_EXTRACT_MIKOLAJCZYK_H

#include "../../../myutilsimage_c.h"

struct MyMikolajczykOptions *my_local_mikolajczyk_newOptionsCsift();

struct MyMikolajczykOptions *my_local_mikolajczyk_newOptionsParse(
		const char *parameters);

const char *my_local_mikolajczyk_helpOptions();

void my_local_mikolajczyk_releaseOptions(struct MyMikolajczykOptions *options);

void my_local_mikolajczyk_extract_filename(const char *filename,
		MyLocalDescriptors *descriptors, struct MyMikolajczykOptions *options);

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

void my_local_mikolajczyk_extract(IplImage *image,
		MyLocalDescriptors *descriptors, struct MyMikolajczykOptions *options);
#endif

#endif
