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

class CropItem: public my::PipeItem {
public:
	CropItem(int width, int height, int x, int y) :
			width(width), height(height), x(x), y(y), autoCenter(false) {
	}
	CropItem(int width, int height) :
			width(width), height(height), x(0), y(0), autoCenter(true) {
	}
	cv::Mat &transform(cv::Mat &image) {
		cv::Rect zone(x, y, width, height);
		if (autoCenter) {
			zone.x = (image.cols - width) / 2;
			zone.y = (image.rows - height) / 2;
		}
		mat = image(zone);
		return mat;
	}
	~CropItem() {
	}

private:
	int width, height, x, y;
	bool autoCenter;
	cv::Mat mat;
}
;
class CropFactory: public my::PipeItemFactory {
public:
	CropFactory() :
			my::PipeItemFactory("CROP") {
	}
	std::string getParametersHelp() {
		return "WidthxHeight_[x,y]";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		if (p.size() != 1 && p.size() != 2)
			throw std::runtime_error("error parsing " + parameters);
		std::vector<std::string> v1 = my::string::split(p[0], 'x');
		if (v1.size() != 2)
			throw std::runtime_error("error parsing " + parameters);
		int w = my::parse::stringToInt(v1[0]);
		int h = my::parse::stringToInt(v1[1]);
		if (p.size() == 1)
			return new CropItem(w, h);
		std::vector<std::string> v2 = my::string::split(p[1], ',');
		if (v2.size() != 2)
			throw std::runtime_error("error parsing " + parameters);
		int x = my::parse::stringToInt(v2[0]);
		int y = my::parse::stringToInt(v2[1]);
		return new CropItem(w, h, x, y);
	}
	~CropFactory() {
	}
};
#endif

void tra_reg_crop() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new CropFactory());
#endif
}
