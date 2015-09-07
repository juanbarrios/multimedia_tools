/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_IMAGE_UTIL_H
#define MYUTILSIMAGE_IMAGE_UTIL_H

#include "../myutilsimage_c.h"

void my_image_setSystemImpl(
		int (*function_io_system)(const char *command_line));

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

void my_image_save(IplImage *imageSrc, const char *filename);
IplImage *my_image_duplicate(IplImage *imageSrc);
void my_image_release(IplImage *img);

void my_image_copyPixels(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y);
void my_image_copyWeightedPixels(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y, double weigthSrc);

void my_image_fill_color(IplImage *img, CvScalar colFondo);
void my_image_fill_white(IplImage *img);
void my_image_fill_squares_color(IplImage *img, CvScalar colFondo1,
		CvScalar colFondo2);
void my_image_fill_squares(IplImage *img);

struct MyConvolutionKernel {
	int64_t size_x, size_y, center_x, center_y, size;
	double **valores;
};
struct MyConvolutionKernel *my_convolution_newKernel(int64_t width,
		int64_t height);
void my_convolution_initKernel(struct MyConvolutionKernel *kc, double *valores);
void my_convolution_initAverageKernel(struct MyConvolutionKernel *kc);
void my_convolution_releaseKernel(struct MyConvolutionKernel *kc);
void my_convolution_perform(IplImage *imgIn, struct MyConvolutionKernel *kc,
		double ***my_convolution_perform);

IplImage *my_image_newFromArray(double **matriz, int64_t width, int64_t height,
		double max_val);
void my_image_copyArrayToPixels(double ***my_convolution_perform,
		double max_val, uchar abs, IplImage *imgOut);

int64_t my_image_copyPixelsToUcharArray(IplImage *imgGray, uchar **prt_buffer);
int64_t my_image_copyPixelsToFloatArray(IplImage *imgGray, float **prt_buffer);

void my_image_pgm_saveBuffer(const char *filePgm, IplImage *imgGris,
		uchar **prt_buffer);
uchar my_image_pgm_loadBuffer(const char *filePgm, uchar **prt_buffer,
		IplImage **ptr_imgGris);
void my_image_pgm_saveImage(const char *filePgm, IplImage *imgGris);
IplImage *my_image_pgm_loadImage(const char *filePgm);

typedef struct MyImageResizer MyImageResizer;
MyImageResizer* my_imageResizer_newFixedSize(int64_t newWidth,
		int64_t newHeight);
MyImageResizer* my_imageResizer_newScreenSize();
char *my_imageResizer_getTextOptions();
bool my_imageResizer_testTextOptions(const char *option);
MyImageResizer* my_imageResizer_newTextOptions(const char *option);
IplImage* my_imageResizer_resizeImage(IplImage *imageSrc,
		MyImageResizer *resizer);
void my_imageResizer_release(MyImageResizer *resizer);

#endif
#endif
