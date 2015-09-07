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

class SobelItem: public my::PipeItem {
public:

	SobelItem(int xorder, int yorder, double threshold, bool isAprox) :
			xorder(xorder), yorder(yorder), threshold(threshold), isAprox(
					isAprox) {
	}
	cv::Mat &transform(cv::Mat &image) {
		if (image.channels() == 3) {
			cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		} else if (image.channels() == 4) {
			cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
		} else {
			gray = image;
		}
		if (xorder == 0 || yorder == 0) {
			cv::Sobel(gray, imgSobelX, CV_32F, xorder, yorder);
			if (threshold > 0) {
				cv::threshold(imgSobelX, imgSobel, threshold, 255, CV_8UC1);
			} else {
				cv::abs(imgSobelX);
				cv::normalize(imgSobelX, imgSobel, 0, 255, cv::NORM_MINMAX,
						CV_8UC1);
			}
			return imgSobel;
		} else {
			cv::Sobel(gray, imgSobelX, CV_32F, xorder, 0);
			cv::Sobel(gray, imgSobelY, CV_32F, 0, yorder);
			cv::convertScaleAbs(imgSobelX, imgSobelX);
			cv::convertScaleAbs(imgSobelY, imgSobelY);
			cv::addWeighted(imgSobelX, 0.5, imgSobelX, 0.5, 0, imgSobel,
			CV_8UC1);
			cv::normalize(imgSobel, imgSobel, 0, 1, cv::NORM_MINMAX, CV_8UC1);
			return imgSobel;
		}
#if 0
		double min = DBL_MAX, max = -DBL_MAX;
		for (int y = 0; y < imgSobelX.rows; ++y) {
			for (int x = 0; x < imgSobelX.cols; ++x) {
				double val;
				if (xorder > 0 && yorder == 0) {
					val = std::abs(imgSobelX.at<float>(y, x));
				} else if (xorder == 0 && yorder > 0) {
					val = std::abs(imgSobelY.at<float>(y, x));
				} else if (isAprox) {
					float v1 = std::abs(imgSobelX.at<float>(y, x));
					float v2 = std::abs(imgSobelY.at<float>(y, x));
					val = v1 + v2;
				} else {
					float v1 = imgSobelX.at<float>(y, x);
					float v2 = imgSobelY.at<float>(y, x);
					val = std::sqrt(v1 * v1 + v2 * v2);
				}
				if (threshold == 0) {
					val = std::min(255.0, std::max(0.0, val));
					imgSobel.at<uchar>(y, x) = (uchar) std::round(val);
				} else {
					imgSobel.at<uchar>(y, x) = (val >= threshold) ? 255 : 0;
				}
			}
		}
		//veo la direccion del gradiente
		if (val >= threshold) {
			val = ptrX[x] == 0 ? (ptrY[x] < 0 ? -M_PI_2 : M_PI_2) : atan(
					ptrY[x] / ptrX[x]);
			if (val < min)
			min = val;
			if (val > max)
			max = val;
		}
		min += M_PI_2;
		max += M_PI_2;
		if (min < 0 || min > M_PI)
		log_info("min=%lf\n", min);
		if (max < 0 || max > M_PI)
		log_info("max=%lf\n", max);
#endif
	}
	~SobelItem() {
	}
private:
	int xorder, yorder;
	double threshold;
	bool isAprox;
	cv::Mat gray, imgSobelX, imgSobelY, imgSobel;
};
class SobelFactory: public my::PipeItemFactory {
public:
	SobelFactory() :
			my::PipeItemFactory("SOBEL") {
	}
	std::string getParametersHelp() {
		return "xorder_yorder_[_threshold][_APROX]";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		if (p.size() < 2)
			throw std::runtime_error("error parsing " + parameters);
		int xorder = my::parse::stringToInt(p[0]);
		int yorder = my::parse::stringToInt(p[1]);
		double threshold = 0;
		if (p.size() > 3)
			threshold = my::parse::stringToDouble(p[3]);
		bool isApprox = false;
		if (p.size() > 4 && my::string::equals_ignorecase(p[4], "APROX"))
			isApprox = true;
		return new SobelItem(xorder, yorder, threshold, isApprox);
	}
	~SobelFactory() {
	}
};
#endif

void tra_reg_sobel() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new SobelFactory());
#endif
}
