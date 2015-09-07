/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef METRICKNN_OPENCV_H_
#define METRICKNN_OPENCV_H_

#include "../metricknn_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>

IplImage *mknn_dataset_convertToIplImage(MknnDataset *dataset);

#endif

#ifdef __cplusplus
}
#endif

#endif
