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
#include <iostream>

#ifndef NO_OPENCV
#include <opencv2/imgproc/imgproc.hpp>

class ThresholdItem: public my::PipeItem {
public:

	ThresholdItem(double threshold) :
			threshold(threshold) {
	}
	cv::Mat &transform(cv::Mat &image) {
		if (threshold == 0) {
			double th = cv::threshold(image, mat, 0, 255,
					cv::THRESH_BINARY | cv::THRESH_OTSU);
			std::cout << "OTSU threshold=" << th << std::endl;
		} else {
			cv::threshold(image, mat, threshold, 255, cv::THRESH_BINARY);
		}
		return mat;
	}
	~ThresholdItem() {
	}
private:
	double threshold;
	cv::Mat mat;
};
class ThresholdFactory: public my::PipeItemFactory {
public:
	ThresholdFactory() :
			my::PipeItemFactory("THRESHOLD") {
	}
	std::string getParametersHelp() {
		return "threshold (0=otsu)";
	}

	my::PipeItem *newItem(std::string parameters) {
		return new ThresholdItem(my::parse::stringToDouble(parameters, true, 0));
	}
	~ThresholdFactory() {
	}
};
#endif

void tra_reg_threshold() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new ThresholdFactory());
#endif
}
