/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "../../myutilsimage_cpp.hpp"
#include <myutils/myutils_cpp.hpp>
#include <stdexcept>

#ifndef NO_OPENCV
#include <opencv2/imgproc/imgproc.hpp>

#define OP_THRESHOLD 1
#define OP_EQUALIZE 2
#define OP_ERODE 3
#define OP_DILATE 4
#define OP_PIXEL 5

class Operation {
public:
	int64_t operation;
	double threshold;
	IplConvKernel* kernel;
	double contrast, brightness;
	uchar isAbs;
};

class GrayItem: public my::PipeItem {
public:

	GrayItem(double a, double b) :
			a(a), b(b) {
	}
	cv::Mat &transform(cv::Mat &image) {
		if (image.channels() == 3) {
			cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		} else if (image.channels() == 4) {
			cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
		} else {
			gray = image;
		}
		cv::convertScaleAbs(gray, mat, a, b);
		return mat;
	}
	~GrayItem() {
	}
private:
	double a, b;
	cv::Mat gray;
	cv::Mat mat;
};
class GrayFactory: public my::PipeItemFactory {
public:
	GrayFactory() :
			my::PipeItemFactory("GRAY") {
	}
	std::string getParametersHelp() {
		return "A_B  (A*i+B)";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		if (p.size() > 2)
			throw std::runtime_error("error parsing " + parameters);
		double a = 1, b = 0;
		if (p.size() == 2) {
			a = my::parse::stringToDouble(p[0]);
			b = my::parse::stringToDouble(p[1]);
		} else if (p.size() == 1) {
			if (p[0] != "")
				a = my::parse::stringToDouble(p[0]);
		}
		if (a == 0)
			a = 1;
		return new GrayItem(a, b);
	}
	~GrayFactory() {
	}
};
#endif

void tra_reg_gray() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new GrayFactory());
#endif
}
