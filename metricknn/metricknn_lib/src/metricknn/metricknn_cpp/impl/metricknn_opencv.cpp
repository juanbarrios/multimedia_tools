/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "metricknn_opencv.hpp"

#ifndef NO_OPENCV

#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>

static int convertInternalToCv(const MyDatatype type) {
	if (my_datatype_isInt8(type))
		return CV_8S;
	else if (my_datatype_isUInt8(type))
		return CV_8U;
	else if (my_datatype_isInt16(type))
		return CV_16S;
	else if (my_datatype_isUInt16(type))
		return CV_16U;
	else if (my_datatype_isInt32(type))
		return CV_32S;
	else if (my_datatype_isFloat(type))
		return CV_32F;
	else if (my_datatype_isDouble(type))
		return CV_64F;
	throw std::runtime_error(
			"opencv does not support "
					+ std::string(my_datatype_codeToDescription(type)));
	return -1;
}

void mknn_dataset_convertToMat(MknnDataset *dataset, cv::Mat &image) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	int64_t num_dimensions = mknn_domain_vector_getNumDimensions(domain);
	MyDatatype type = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(domain));
	int64_t num_objects = mknn_dataset_getNumObjects(dataset);
	int cv_type = convertInternalToCv(type);
	void *pointer = mknn_dataset_getCompactVectors(dataset);
	image = cv::Mat(num_objects, num_dimensions, cv_type, pointer);
}

extern "C" IplImage *mknn_dataset_convertToIplImage(MknnDataset *dataset) {
	cv::Mat image;
	mknn_dataset_convertToMat(dataset, image);
	IplImage *img = cvCreateImageHeader(cvSize(image.cols, image.rows),
			image.depth(), image.channels());
	cvSetData(img, image.data, CV_AUTOSTEP);
	return img;
}

#endif
