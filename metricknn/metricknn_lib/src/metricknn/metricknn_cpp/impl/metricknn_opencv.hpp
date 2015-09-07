/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef METRICKNN_OPENCV_HPP_
#define METRICKNN_OPENCV_HPP_

#include "../metricknn_impl.hpp"

#ifndef NO_OPENCV

#include <opencv2/core/core.hpp>

void mknn_dataset_convertToMat(MknnDataset *dataset, cv::Mat &image);

#endif

#endif
