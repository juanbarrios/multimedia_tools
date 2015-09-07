/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "imgResizer.hpp"
#include <myutils/myutils_cpp.hpp>

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if WIN32
#include "wtypes.h"
#endif

// Get the horizontal and vertical screen sizes in pixel
static void internal_getScreenResolution(int *out_width, int *out_height) {
#if WIN32
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	*out_width = desktop.right;
	*out_height = desktop.bottom;
#else
	/*
	 char *command = "xrandr | grep '*'";
	 FILE *fpipe = (FILE*) popen(command, "r");
	 char line[256];
	 while (fgets(line, sizeof(line), fpipe)) {
	 printf("%s", line);
	 }
	 pclose(fpipe);
	 */
	*out_width = 1024;
	*out_height = 768;
#endif
}

using namespace my;

std::string MyImageResizer::getParametersHelp() {
	return "FIXED_width[x][height]|LIMIT_width[x][height]|SCALE_factorW[x][factorH]";
}
MyImageResizer MyImageResizer::fixed(int newWidth, int newHeight) {
	MyImageResizer resizer;
	resizer.isFixed = true;
	resizer.width = newWidth;
	resizer.height = newHeight;
	return resizer;
}
MyImageResizer MyImageResizer::limit(int newWidth, int newHeight) {
	MyImageResizer resizer;
	resizer.isLimit = true;
	resizer.width = newWidth;
	resizer.height = newHeight;
	return resizer;
}
MyImageResizer MyImageResizer::scale(float factorWidth, float factorHeight) {
	MyImageResizer resizer;
	resizer.isScale = true;
	resizer.factorWidth = factorWidth;
	resizer.factorHeight = factorHeight;
	return resizer;
}
cv::Size MyImageResizer::getScreenSize() {
	int maxW = 0, maxH = 0;
	internal_getScreenResolution(&maxW, &maxH);
	if (maxW == 0 || maxH == 0)
		throw std::runtime_error("cannot detect screen size");
	return cv::Size(maxW - 10, maxH - 50);
}
MyImageResizer MyImageResizer::fixedToScreenSize() {
	cv::Size s = MyImageResizer::getScreenSize();
	return MyImageResizer::fixed(s.width, s.height);
}
MyImageResizer MyImageResizer::limitToScreenSize() {
	cv::Size s = MyImageResizer::getScreenSize();
	return MyImageResizer::limit(s.width, s.height);
}
static cv::Size parseSize(std::string &s) {
	if (s == "SCREEN")
		return MyImageResizer::getScreenSize();
	std::vector<int> v = my::parse::stringListToInt(my::string::split(s, 'x'));
	if (v.size() == 1)
		return cv::Size(v[0], 0);
	else if (v.size() == 2)
		return cv::Size(v[0], v[1]);
	else
		throw std::runtime_error("invalid resize options");
}
MyImageResizer MyImageResizer::parameters(std::string options) {
	std::vector<std::string> p = my::string::split(options, '_');
	if (p.size() != 2)
		throw std::runtime_error("invalid resize options " + options);
	std::string type = p[0];
	std::string strsize = p[1];
	if (type == "FIXED") {
		cv::Size s = parseSize(strsize);
		return MyImageResizer::fixed(s.width, s.height);
	} else if (type == "LIMIT") {
		cv::Size s = parseSize(strsize);
		return MyImageResizer::limit(s.width, s.height);
	} else if (type == "SCALE") {
		std::vector<float> sv = my::parse::stringListToFloat(
				my::string::split(strsize, 'x'));
		if (sv.size() == 1)
			return MyImageResizer::scale(sv[0], sv[0]);
		else if (sv.size() == 2)
			return MyImageResizer::scale(sv[0], sv[1]);
	}
	throw std::runtime_error("invalid resize options " + options);
}
cv::Size MyImageResizer::computeNewSize(int currentW, int currentH) {
	if (currentW <= 0 || currentH <= 0)
		throw std::runtime_error("invalid image size");
	if (isFixed) {
		if (width > 0 && height <= 0) {
			float hwRatio = currentH / (float) currentW;
			return cv::Size(width, (int) std::round(width * hwRatio));
		} else if (width <= 0 && height > 0) {
			float whRatio = currentW / (float) currentH;
			return cv::Size(std::round(height * whRatio), height);
		} else {
			return cv::Size(width, height);
		}
	} else if (isLimit) {
		if (currentW <= width && currentH <= height)
			return cv::Size(currentW, currentH);
		float scaleW = width / (float) currentW;
		float scaleH = height / (float) currentH;
		float scale = std::min(scaleW, scaleH);
		return cv::Size(std::round(currentW * scale),
				std::round(currentH * scale));
	} else if (isScale) {
		return cv::Size(currentW * factorWidth, currentH * factorHeight);
	}
	return cv::Size(currentW, currentH);
}
MyImageResizer::MyImageResizer() :
		isFixed(false), isLimit(false), isScale(false), width(0), height(0), factorWidth(
				0), factorHeight(0) {

}
cv::Mat & MyImageResizer::resize(cv::Mat &image) {
	cv::Size newSize = computeNewSize(image.cols, image.rows);
	if (newSize.width == image.cols && newSize.height == image.rows)
		return image;
	//CV_INTER_AREA is recommended for shrinking
	//CV_INTER_CUBIC is recommended for enlargement
	bool isSmaller = (newSize.width < image.cols && newSize.height < image.rows);
	cv::resize(image, mat, newSize, 0, 0,
			isSmaller ? cv::INTER_AREA : cv::INTER_CUBIC);
	return mat;
}
#endif
