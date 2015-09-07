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

class FilterAvgMedianItem: public my::PipeItem {
public:
	FilterAvgMedianItem(int width, int height, bool isMedian) :
			width(width), height(height), isMedian(isMedian) {
	}
	cv::Mat & transform(cv::Mat & image) {
		if (isMedian) {
			if (width != height)
				throw std::runtime_error("only squared median filter");
			cv::medianBlur(image, mat, width);
		} else {
			cv::blur(image, mat, cv::Size(width, height));
		}
		return mat;
	}
private:
	int width, height;
	bool isMedian;
	cv::Mat mat;
};
class FilterMorphItem: public my::PipeItem {
public:
	FilterMorphItem(int width, int height, bool isErode) :
			isErode(isErode) {
		structuringElement = cv::getStructuringElement(cv::MORPH_RECT,
				cv::Size(width, height));
	}
	cv::Mat & transform(cv::Mat & image) {
		if (isErode)
			cv::erode(image, mat, structuringElement);
		else
			cv::dilate(image, mat, structuringElement);
		return mat;
	}
private:
	bool isErode;
	cv::Mat structuringElement;
	cv::Mat mat;
};

class FilterConvItem: public my::PipeItem {
public:
	FilterConvItem(int width, int height, std::vector<double> vals) {
		kernel = cv::Mat(height, width, CV_64FC1);
		for (std::vector<double>::iterator it = vals.begin(); it != vals.end();
				++it) {
			kernel.push_back(*it);
		}
	}
	cv::Mat & transform(cv::Mat & image) {
		cv::filter2D(image, mat, -1, kernel);
		return mat;
	}
private:
	cv::Mat kernel;
	cv::Mat mat;
};
#if 0
uchar tra_calcular_filtro(IplImage *imagen, int64_t x, int64_t y, int64_t nc,
		struct Proceso_FILTRO *es) {
	int64_t tipo = es->filtro_tipo, size = es->filtro_size;
	if (tipo == F_TYPE_MEDIAN && es->ventana == NULL)
	es->ventana = MY_MALLOC_NOINIT(size, uchar);
	int64_t i, j, pos = 0;
	uchar min = 255, max = 0;
	for (i = 0; i < es->filtro_width; ++i) {
		int64_t posx = x - es->filtro_width / 2 + i;
		if (posx < 0)
		posx = 0;
		if (posx >= imagen->width)
		posx = imagen->width - 1;
		for (j = 0; j < es->filtro_height; ++j) {
			int64_t posy = y - es->filtro_height / 2 + j;
			if (posy < 0)
			posy = 0;
			if (posy >= imagen->height)
			posy = imagen->height - 1;
			uchar pix = PIXC_8U(imagen, posx, posy, nc);
			switch (tipo) {
				case F_TYPE_MAX:
				if (pix > max)
				max = pix;
				break;
				case F_TYPE_MIN:
				if (pix < min)
				min = pix;
				break;
				case F_TYPE_MEDIAN:
				es->ventana[pos++] = pix;
				break;
				default:
				my_log_error("unsupported\n");
				break;
			}
		}
	}
	switch (tipo) {
		case F_TYPE_MAX:
		return max;
		case F_TYPE_MIN:
		return min;
		case F_TYPE_MEDIAN:
		my_qsort_uint8_array(es->ventana, size);
		return es->ventana[size / 2];
		default:
		return 0;
	}
}

cv::Mat &transform(cv::Mat &image) {
	if (es->useOpenCV) {
		switch (es->filtro_tipo) {
			case F_TYPE_AVERAGE:
			cv::blur(image, mat, cv::Size(w, h));
			break;
			case F_TYPE_MEDIAN:
			cv::medianBlur(image, mat, w);
			break;
			default:
			my_log_error("unsupported by opencv\n");
			break;
		}
	} else if (es->filtro_tipo == F_TYPE_AVERAGE
			|| es->filtro_tipo == F_TYPE_CONVOLUTION) {
		if (es->kc == NULL) {
			es->kc = my_convolution_newKernel(es->filtro_width, es->filtro_height);
			if (es->filtro_tipo == F_TYPE_AVERAGE)
			my_convolution_initAverageKernel(es->kc);
			else if (es->filtro_tipo == F_TYPE_CONVOLUTION)
			my_convolution_initKernel(es->kc, es->valores_kernel);
		}
		my_convolution_perform(imagen, es->kc, es->my_convolution_perform);
		my_image_copyArrayToPixels(es->my_convolution_perform, es->convol_max, es->convol_abs,
				es->imgOut);
	} else {
		int64_t x, y, nc;
		for (y = 0; y < imagen->height; ++y) {
			for (x = 0; x < imagen->width; ++x) {
				for (nc = 0; nc < imagen->nChannels; ++nc) {
					PIXC_8U(es->imgOut, x, y, nc) = tra_calcular_filtro(imagen,
							x, y, nc, es);
				}
			}
		}
	}
	if (es->addToImg) {
		int64_t x, y;
		for (y = 0; y < imagen->height; ++y) {
			uchar *ptrIn = (uchar*) (imagen->imageData + imagen->widthStep * y);
			uchar *ptrOut = (uchar*) (es->imgOut->imageData
					+ es->imgOut->widthStep * y);
			for (x = 0; x < imagen->width; ++x) {
				int64_t val = ptrIn[x] + ptrOut[x];
				if (val < 0)
				val = 0;
				if (val > 255)
				val = 255;
				ptrOut[x] = val;
			}
		}
	}
	return mat;
}
~FilterItem() {
}
private:
int64_t filtro_width, filtro_height, filtro_tipo, filtro_size;
double *valores_kernel;
int64_t array_dim1, array_dim2, array_dim3;
double ***my_convolution_perform;
uchar convol_abs;
double convol_max;
uchar *ventana;
uchar useOpenCV, addToImg;
struct MyConvolutionKernel *kc;

cv::Mat mat;
};
#endif

#define F_TYPE_AVERAGE 1
#define F_TYPE_MEDIAN 2
#define F_TYPE_ERODE 3
#define F_TYPE_DILATE 4
#define F_TYPE_CONVOLUTION 5

class FilterFactory: public my::PipeItemFactory {
public:
	FilterFactory() :
			my::PipeItemFactory("FILTER") {
	}
	std::string getParametersHelp() {
		return "[AVG|MEDIAN|ERODE|DILATE|CONV]_WidthxHeight_[ConvVal1,ConvVal2,...])[_NOOPENCV]";
	}

	my::PipeItem *newItem(std::string parameters) {
		std::vector<std::string> p = my::string::split(parameters, '_');
		if (p.size() < 3)
			throw std::runtime_error("error parsing " + parameters);
		std::vector<std::string>::iterator it = p.begin();
		std::string name = *it;
		int ftype = 0;
		if (name == "AVG")
			ftype = F_TYPE_AVERAGE;
		else if (name == "MEDIAN")
			ftype = F_TYPE_MEDIAN;
		else if (name == "ERODE")
			ftype = F_TYPE_ERODE;
		else if (name == "DILATE")
			ftype = F_TYPE_DILATE;
		else if (name == "CONV")
			ftype = F_TYPE_CONVOLUTION;
		else
			throw std::runtime_error("unknown " + name);
		it++;
		std::vector<std::string> v = my::string::split(*it, '_');
		if (v.size() != 2)
			throw std::runtime_error("error parsing " + parameters);
		int width = my::parse::stringToInt(v[0]);
		int height = my::parse::stringToInt(v[1]);
		std::vector<double> vals;
		if (ftype == F_TYPE_CONVOLUTION) {
			it++;
			vals = my::parse::stringListToDouble(my::string::split(*it, ','));
			if ((int) vals.size() != width * height)
				throw std::runtime_error("error parsing " + parameters);
		}
		it++;
		bool useOpenCV = true;
		if (*it == "NOOPENCV")
			useOpenCV = false;
		if (it != p.end())
			throw std::runtime_error("error parsing " + parameters);
		if (useOpenCV) {
			if (ftype == F_TYPE_AVERAGE) {
				return new FilterAvgMedianItem(width, height, false);
			} else if (ftype == F_TYPE_MEDIAN) {
				return new FilterAvgMedianItem(width, height, true);
			} else if (ftype == F_TYPE_ERODE) {
				return new FilterMorphItem(width, height, true);
			} else if (ftype == F_TYPE_DILATE) {
				return new FilterMorphItem(width, height, false);
			} else if (ftype == F_TYPE_CONVOLUTION) {
				return new FilterConvItem(width, height, vals);
			}
		}
		return NULL;
	}
	~FilterFactory() {
	}
};
#endif

void tra_reg_filter() {
#ifndef NO_OPENCV
	my::PipeItemAbstractFactory::registerFactory(new FilterFactory());
#endif
}

