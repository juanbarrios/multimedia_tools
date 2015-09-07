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

class EqualizeItem: public my::PipeItem {
public:

	cv::Mat &transform(cv::Mat &image) {
		if (image.channels() == 3) {
			cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		} else if (image.channels() == 4) {
			cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
		} else {
			gray = image;
		}
		cv::equalizeHist(gray, mat);
		return mat;
	}
	~EqualizeItem() {
	}
private:
	cv::Mat gray;
	cv::Mat mat;
};
class EqualizeFactory: public my::PipeItemFactory {
public:
	EqualizeFactory() :
			my::PipeItemFactory("EQUALIZE") {
	}
	std::string getParametersHelp() {
		return "";
	}

	my::PipeItem *newItem(std::string parameters) {
		if (parameters != "")
			throw std::runtime_error("invalid parameters " + parameters);
		return new EqualizeItem();
	}
	~EqualizeFactory() {
	}
};
#endif

void tra_reg_equalize() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new EqualizeFactory());
#endif
}
