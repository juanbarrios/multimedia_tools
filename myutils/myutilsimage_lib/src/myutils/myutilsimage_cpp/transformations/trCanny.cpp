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

class CannyItem: public my::PipeItem {
public:

	CannyItem(double threshold1, double threshold2, int aperture_size) :
			threshold1(threshold1), threshold2(threshold2), aperture_size(
					aperture_size) {
	}
	cv::Mat &transform(cv::Mat &image) {
		cv::Canny(image, mat, threshold1, threshold2, aperture_size);
		return mat;
	}
	~CannyItem() {
	}
private:
	double threshold1, threshold2;
	int aperture_size;
	cv::Mat mat;
};
class CannyFactory: public my::PipeItemFactory {
public:
	CannyFactory() :
			my::PipeItemFactory("CANNY") {
	}
	std::string getParametersHelp() {
		return "threshold1_threshold2_[apertureSize]";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		if (p.size() < 2)
			throw std::runtime_error("error parsing " + parameters);
		return new CannyItem(my::parse::stringToDouble(p[0]),
				my::parse::stringToDouble(p[1]),
				(p.size() > 2) ? my::parse::stringToInt(p[2]) : 3);
	}
	~CannyFactory() {
	}
};
#endif
void tra_reg_canny() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new CannyFactory());
#endif
}
