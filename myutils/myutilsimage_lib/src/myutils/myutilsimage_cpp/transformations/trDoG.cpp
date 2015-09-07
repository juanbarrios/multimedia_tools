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

class GaussBlur {
public:
	GaussBlur(cv::Size size) :
			varX(0), varY(0), size(size) {
	}
	GaussBlur(double varX, double varY) :
			varX(varX), varY(varY), size(cv::Size(0, 0)) {
	}
	void apply(cv::Mat &src, cv::Mat &dst) {
		cv::GaussianBlur(src, dst, size, varX, varY);
	}
private:
	double varX, varY;
	cv::Size size;
};

class DogItem: public my::PipeItem {
public:

	DogItem(GaussBlur gb1, GaussBlur gb2, double threshold) :
			gb1(gb1), gb2(gb2), threshold(threshold) {
	}
	cv::Mat &transform(cv::Mat &image) {
		if (image.channels() == 3) {
			cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		} else if (image.channels() == 4) {
			cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
		} else {
			gray = image;
		}
		gb1.apply(gray, imgBuff1);
		gb2.apply(gray, imgBuff2);
		cv::subtract(imgBuff1, imgBuff2, imgDiff,cv::noArray(), CV_16SC1);
		if (threshold > 0) {
			cv::threshold(imgDiff, imgBin, threshold, 255, cv::THRESH_BINARY);
			return imgBin;
		} else {
			double max_val = 0;
			cv::minMaxLoc(imgDiff, NULL, &max_val);
			if (max_val > 0)
				cv::convertScaleAbs(imgDiff, imgBin, 255.0 / max_val, 0);
			return imgBin;
		}
	}
	~DogItem() {
	}
private:
	GaussBlur gb1, gb2;
	double threshold;
	cv::Mat gray, imgBuff1, imgBuff2, imgDiff, imgBin;
};
class DogFactory: public my::PipeItemFactory {
public:
	DogFactory() :
			my::PipeItemFactory("DOG") {
	}
	std::string getParametersHelp() {
		return "[Width1xHeight1|var_VXxVY]_[Width2xHeight2|var_VXxVY]_[threshold]";
	}

	GaussBlur parseGB(std::vector<std::string> &p, size_t &start,
			const std::string &parameters) {
		bool useVar = false;
		if (my::string::equals_ignorecase(p.at(start), "var")) {
			start++;
			useVar = true;
		}
		std::vector<std::string> v = my::string::split(p.at(start), 'x');
		if (v.size() != 2)
			throw std::runtime_error("error parsing " + parameters);
		start++;
		if (useVar) {
			return GaussBlur(my::parse::stringToDouble(v[0]),
					my::parse::stringToDouble(v[1]));
		} else {
			return GaussBlur(
					cv::Size(my::parse::stringToInt(v[0]),
							my::parse::stringToInt(v[1])));
		}
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		size_t start = 0;
		GaussBlur gb1 = parseGB(p, start, parameters);
		GaussBlur gb2 = parseGB(p, start, parameters);
		double threshold = 0;
		if (start < p.size())
			threshold = my::parse::stringToDouble(p[start]);
		return new DogItem(gb1, gb2, threshold);
	}
	~DogFactory() {
	}
};
#endif

void tra_reg_dog() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new DogFactory());
#endif
}
