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

class HistGrayItem: public my::PipeItem {
public:
	HistGrayItem(int num_bins, bool isNormL1, bool isNormMax, bool isPrint) :
			num_bins(num_bins), isNormL1(isNormL1), isNormMax(isNormMax), isPrint(
					isPrint) {
		histogram.reserve(num_bins);
		histogram_colors.reserve(num_bins);
		histImage = cv::Mat(cv::Size(512, 300), CV_8UC3, cv::Scalar(0, 0, 0));
		getColors();
	}
	void getColors() {
		histogram_colors.clear();
		for (int i = 0; i < num_bins; i++) {
			double gg = (i * 255.0 / (num_bins - 1));
			cv::Scalar col(gg, gg, gg);
			histogram_colors.push_back(col);
		}
	}

	void computeHistogram(cv::Mat &image) {
		if (image.channels() == 3) {
			cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		} else if (image.channels() == 4) {
			cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
		} else {
			gray = image;
		}
		float range[] = { 0, 256 };
		const float* histRange = { range };
		int channels = 0;
		bool uniform = true;
		bool accumulate = false;
		cv::calcHist(&gray, 1, &channels, cv::Mat(), b_hist, 1, &num_bins,
				&histRange, uniform, accumulate);
		if (isNormL1)
			cv::normalize(b_hist, b_hist_norm, 1, 0, cv::NORM_L1);
		if (isNormMax)
			cv::normalize(b_hist, b_hist_norm, 0, 1, cv::NORM_MINMAX);
		histogram.clear();
		for (int i = 0; i < num_bins; i++) {
			float val = b_hist_norm.at<float>(i);
			histogram.push_back(val);
		}
		if (isPrint)
			std::cout << "histogram="
					<< my::toString::collection(histogram, "{", ",", "}")
					<< std::endl;
	}
	void computeImageHistogram() {
		my::ImageUtil::my_image_fill_squares(histImage);
		int bin_w = std::round(histImage.cols / ((double) num_bins));
		cv::Point bottom_left(0, histImage.rows - 1);
		cv::Point top_right(0, histImage.rows - 1);
		for (int i = 0; i < num_bins; i++) {
			top_right.x = bottom_left.x + bin_w;
			top_right.y = std::round((1.0 - histogram[i]) * bottom_left.y);
			top_right.y = std::min(std::max(0, top_right.y), bottom_left.y);
			cv::rectangle(histImage, bottom_left, top_right,
					histogram_colors[i], CV_FILLED);
			bottom_left.x += bin_w;
		}
	}
	cv::Mat &transform(cv::Mat &image) {
		computeHistogram(image);
		computeImageHistogram();
		return histImage;
	}
	~HistGrayItem() {
	}
private:
	int num_bins;
	bool isNormL1;
	bool isNormMax;
	bool isPrint;
	std::vector<float> histogram;
	std::vector<cv::Scalar> histogram_colors;
	cv::Mat b_hist;
	cv::Mat b_hist_norm;
	cv::Mat gray;
	cv::Mat histImage;
};
class HistGrayFactory: public my::PipeItemFactory {
public:
	HistGrayFactory() :
			my::PipeItemFactory("HISTGRAY") {
	}
	std::string getParametersHelp() {
		return "bins_[normL1|normMax]_[print]";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::cout << "1a" << std::endl;
		std::vector<std::string> p = my::string::split(parameters, '_');
		std::cout << "1b" << std::endl;
		if (p.size() < 1)
			throw std::runtime_error("error parsing " + parameters);
		std::cout << "1c" << std::endl;
		int num_bins = my::parse::stringToInt(p[0]);
		std::cout << "1d" << std::endl;
		bool isNormL1 = false, isNormMax = false, isPrint = false;
		for (size_t i = 1; i < p.size(); ++i) {
			std::cout << "a" << i << std::endl;
			if (my::string::equals_ignorecase(p[i], "normL1"))
				isNormL1 = true;
			else if (my::string::equals_ignorecase(p[i], "normMax"))
				isNormMax = true;
			else if (my::string::equals_ignorecase(p[i], "print"))
				isPrint = true;
			else
				throw std::runtime_error("unknown " + p[i]);
		}
		if (!isNormL1 && !isNormMax)
			throw std::runtime_error("normalization is required");
		return new HistGrayItem(num_bins, isNormL1, isNormMax, isPrint);
	}
	~HistGrayFactory() {
	}
};
#endif

void tra_reg_histgray() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new HistGrayFactory());
#endif
}
