/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_HISTOGRAM_H
#define MYUTILSIMAGE_HISTOGRAM_H

#include "../myutilsimage_c.h"

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>

IplImage *my_image_newImageHistogram(int64_t num_bins, double *bin_values);

#endif
#endif
