/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILS_MYUTILSIMAGE_CPP_IMGRESIZER_HPP_
#define MYUTILS_MYUTILSIMAGE_CPP_IMGRESIZER_HPP_

#include "../myutilsimage_cpp.hpp"

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>

namespace my {

class MyImageResizer {
public:

	static std::string getParametersHelp();
	static MyImageResizer fixed(int newWidth, int newHeight);
	static MyImageResizer limit(int newWidth, int newHeight);
	static MyImageResizer scale(float factorWidth, float factorHeight);
	static MyImageResizer fixedToScreenSize();
	static MyImageResizer limitToScreenSize();
	static MyImageResizer parameters(std::string option);
	static cv::Size getScreenSize();

	cv::Mat &resize(cv::Mat &image);
	MyImageResizer();

private:
	bool isFixed, isLimit, isScale;
	int width, height;
	float factorWidth, factorHeight;
	cv::Mat mat;

	cv::Size computeNewSize(int currentW, int currentH);
};

}
#endif
#endif
