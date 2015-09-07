/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "image.hpp"
#include <myutils/myutils_c.h>
#include <fstream>

#ifndef NO_OPENCV
using namespace my;

void ImageUtil::my_image_fill_color(cv::Mat &image, cv::Scalar color) {
	cv::rectangle(image, cv::Rect(0, 0, image.cols, image.rows), color, -1);
}
void ImageUtil::my_image_fill_white(cv::Mat &image) {
	my_image_fill_color(image, cv::Scalar(255, 255, 255));
}
void ImageUtil::my_image_fill_squares_color(cv::Mat &image, cv::Scalar color1,
		cv::Scalar color2) {
	int size_squares = 15;
	bool useCol1_first = true;
	for (int start_h = 0; start_h < image.rows; start_h += size_squares) {
		bool useCol1 = useCol1_first;
		for (int start_w = 0; start_w < image.cols; start_w += size_squares) {
			cv::rectangle(image,
					cv::Rect(start_w, start_h, size_squares, size_squares),
					(useCol1 ? color1 : color2), -1);
			useCol1 = !useCol1;
		}
		useCol1_first = !useCol1_first;
	}
}
void ImageUtil::my_image_fill_squares(cv::Mat &image) {
	my_image_fill_squares_color(image, cv::Scalar(240, 200, 200),
			cv::Scalar(200, 240, 200));
}

void ImageUtil::saveMatBinary(cv::Mat &mat, std::string filename) {
	int header[3] = { mat.rows, mat.cols, mat.type() };
	FILE *out = my_io_openFileWrite1(filename.c_str());
	int64_t n = fwrite(header, sizeof(int), 3, out);
	my_assert_equalInt("written bytes", n, 3);
	size_t size_pixel = 0;
	if (mat.type() == CV_8S || mat.type() == CV_8U)
		size_pixel = 1;
	else if (mat.type() == CV_16S || mat.type() == CV_16U)
		size_pixel = 2;
	else if (mat.type() == CV_32S || mat.type() == CV_32F)
		size_pixel = 4;
	else if (mat.type() == CV_64F)
		size_pixel = 8;
	int length = mat.rows * mat.cols;
	int64_t n2 = fwrite(mat.data, size_pixel, length, out);
	my_assert_equalInt("written bytes", n2, length);
	fclose(out);
}
void ImageUtil::saveMatText(cv::Mat &mat, std::string filename) {
	std::ofstream out(filename);
	out << mat.rows << " " << mat.cols << std::endl;
	out << mat << std::endl;
	out.close();
}
#endif
