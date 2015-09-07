/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "opencv.h"
#include <vector>
#include <iostream>
#include <string>
#include <myutils/myutils_cpp.hpp>
#include <stdexcept>

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>

class MyOptions {
public:
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorExtractor> extractor;bool isDebug, isUchar;
	MyOptions() :
			detector(NULL), extractor(NULL), isDebug(false), isUchar(false) {
	}
};

extern "C" MyOpencvOptions *my_local_opencv_newOptionsParse(
		const char *parameters_st) {
	std::string parameters = "";
	if (parameters_st != NULL)
		parameters = parameters_st;
	cv::initModule_nonfree();
	MyOptions *opt = new MyOptions();
	std::vector<std::string> parts = my::string::split(parameters, '_');
	if (parts.size() < 2)
		throw std::runtime_error(
				"invalid detector and descriptor (" + parameters + ") help: "
						+ std::string(my_local_opencv_helpOptions()));
	opt->detector = cv::FeatureDetector::create(parts[0]);
	if (opt->detector.empty())
		throw std::runtime_error(
				"can't find detector " + parts[0] + " help: "
						+ std::string(my_local_opencv_helpOptions()));
	opt->extractor = cv::DescriptorExtractor::create(parts[1]);
	if (opt->extractor.empty())
		throw std::runtime_error(
				"can't find extractor " + parts[1] + " help: "
						+ std::string(my_local_opencv_helpOptions()));
	for (std::vector<std::string>::size_type i = 2; i < parts.size(); ++i) {
		if (parts[i] == "DEBUG")
			opt->isDebug = true;
		else if (parts[i] == "UCHAR")
			opt->isUchar = true;
		else
			throw std::runtime_error("unknown parameter " + parts[i]);
	}
	return opt;
}

extern "C" const char *my_local_opencv_helpOptions() {
	return "[Grid][Pyramid](FAST|STAR|SIFT|SURF|ORB|BRISK|MSER|GFTT|HARRIS|Dense|SimpleBlob)_[Opponent](SIFT|SURF|BRIEF|BRISK|ORB|FREAK)_(DEBUG)_(UCHAR)";
}

extern "C" void my_local_opencv_releaseOptions(MyOpencvOptions *options) {
	MyOptions *opt = (MyOptions*) options;
	delete opt;
}

static MyDatatype opencvType2MyType(cv::Mat &mat) {
	switch (mat.depth()) {
	case CV_8U:
		return MY_DATATYPE_UINT8;
	case CV_8S:
		return MY_DATATYPE_INT8;
	case CV_16U:
		return MY_DATATYPE_UINT16;
	case CV_16S:
		return MY_DATATYPE_INT16;
	case CV_32S:
		return MY_DATATYPE_INT32;
	case CV_32F:
		return MY_DATATYPE_FLOAT32;
	case CV_64F:
		return MY_DATATYPE_FLOAT64;
	}
	std::cout << "unknown depth " << mat.depth() << std::endl;
	MyDatatype d = { 0 };
	return d;
}
inline double convert_size_to_radius(double size) {
	return size / 2.0;
}
inline double convert_angle_to_radians(double angle) {
	if (angle < 0) {
		return 0;
	} else if (angle > 180) {
		return (angle - 360) / 180.0 * M_PI;
	} else {
		return angle / 180.0 * M_PI;
	}
}
extern "C" void my_local_opencv_extract(IplImage *ipl_image,
		MyLocalDescriptors *ldes, MyOpencvOptions *options) {
	MyOptions *opt = (MyOptions*) options;
	cv::Mat img = ipl_image;
	cv::Mat descriptors;
	std::vector<cv::KeyPoint> keypoints;
	opt->detector->detect(img, keypoints);
	opt->extractor->compute(img, keypoints, descriptors);
	my::assert::equalInt("keypoints and descriptors", keypoints.size(),
			descriptors.rows);
	my::assert::equalInt("channels", descriptors.channels(), 1);
	my_localDescriptors_redefineVectorDatatype(ldes,
			opencvType2MyType(descriptors), descriptors.cols, keypoints.size());
	for (int i = 0; i < (int) keypoints.size(); ++i) {
		cv::KeyPoint kp = keypoints[i];
		my_localDescriptors_setKeypoint(ldes, i, kp.pt.x, kp.pt.y,
				convert_size_to_radius(kp.size),
				convert_angle_to_radians(kp.angle));
		void *data = descriptors.row(i).data;
		my_localDescriptors_setVector(ldes, i, data);
	}
	if (opt->isUchar)
		my_localDescriptors_scaleConvertVectors(ldes, 1, MY_DATATYPE_UINT8);
	if (opt->isDebug) {
		cv::Mat image2;
		cv::drawKeypoints(img, keypoints, image2, cv::Scalar::all(-1),
				cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		cv::imshow("drawKeypoints", image2);
		my_function_to_string fToString = my_datatype_getFunctionToString(
				my_localDescriptors_getVectorDatatype(ldes));
		for (std::vector<cv::KeyPoint>::size_type i = 0;
				i < keypoints.size() && i < 5; ++i) {
			cv::KeyPoint kp = keypoints[i];
			void *vector = my_localDescriptors_getVector(ldes, i);
			std::cout << i << ") " << kp.pt << " size=" << kp.size << " angle="
					<< kp.angle << " response=" << kp.response << " octave="
					<< kp.octave << std::endl;
			char *st = fToString(vector,
					my_localDescriptors_getVectorDimensions(ldes), "{", ",",
					"}");
			std::cout << std::string(st) << std::endl;
			free(st);
		}
	}
}

#endif
