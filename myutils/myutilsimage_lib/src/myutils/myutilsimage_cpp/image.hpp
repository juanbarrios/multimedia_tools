/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILS_MYUTILSIMAGE_CPP_IMAGE_HPP_
#define MYUTILS_MYUTILSIMAGE_CPP_IMAGE_HPP_

#include "../myutilsimage_cpp.hpp"

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>

namespace my {

class ImageUtil {
public:
	static void my_image_fill_color(cv::Mat &image, cv::Scalar color);
	static void my_image_fill_white(cv::Mat &image);
	static void my_image_fill_squares_color(cv::Mat &image, cv::Scalar color1,
			cv::Scalar color2);
	static void my_image_fill_squares(cv::Mat &image);
	static void saveMatBinary(cv::Mat &mat, std::string filename);
	static void saveMatText(cv::Mat &mat, std::string filename);
};

}

#endif
#endif
