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

class ResizeItem: public my::PipeItem {
public:

	ResizeItem(std::string parameters) {
		resizer = my::MyImageResizer::parameters(parameters);
	}
	cv::Mat &transform(cv::Mat &image) {
		return resizer.resize(image);
	}
	~ResizeItem() {
	}
private:
	my::MyImageResizer resizer;
};
class ResizeFactory: public my::PipeItemFactory {
public:
	ResizeFactory() :
			my::PipeItemFactory("RESIZE") {
	}
	std::string getParametersHelp() {
		return my::MyImageResizer::getParametersHelp();
	}

	my::PipeItem *newItem(std::string parameters) {
		return new ResizeItem(parameters);
	}
	~ResizeFactory() {
	}
};
#endif

void tra_reg_resize() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new ResizeFactory());
#endif
}
