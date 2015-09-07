/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_EXTRACT_BAY_SURF_H
#define MYUTILSIMAGE_LOCAL_EXTRACT_BAY_SURF_H

#include "../../../myutilsimage_c.h"

void my_local_bay_extract_filenamePgm(const char *filename,
		MyLocalDescriptors *descriptors);

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

void my_local_bay_extract(IplImage *image, MyLocalDescriptors *descriptors);
#endif

#endif
