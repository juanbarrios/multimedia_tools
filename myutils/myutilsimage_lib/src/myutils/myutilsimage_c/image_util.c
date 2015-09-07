/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "image_util.h"
#include <myutils/myutils_c.h>

void my_image_setSystemImpl(int (*function_system)(const char *command_line)) {
	my_io_system_setSystemImpl(function_system);
}

#ifndef NO_OPENCV
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

void my_image_save(IplImage *image, const char *filename) {
	bool ov = my_io_existsFile(filename);
	if (ov)
		my_log_info("overwriting %s (%ix%i)\n", filename, image->width,
				image->height);
	if (cvSaveImage(filename, image, NULL)) {
		if (!ov) {
			my_log_info("saved image %s (%ix%i)\n", filename, image->width,
					image->height);
		}
	} else {
		my_log_info("error saving %s\n", filename);
	}
}
IplImage *my_image_duplicate(IplImage *imageSrc) {
	IplImage *imageDest = cvCreateImage(cvGetSize(imageSrc), imageSrc->depth,
			imageSrc->nChannels);
	cvCopy(imageSrc, imageDest, NULL);
	return imageDest;
}
void my_image_release(IplImage *img) {
	if (img != NULL)
		cvReleaseImage(&img);
}
static void copyHardWithAlphaChannel(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y) {
	IplImage *alpha = cvCreateImage(cvGetSize(image_src), image_src->depth, 1);
	cvSplit(image_src, NULL, NULL, NULL, alpha);
	cvSetImageROI(image_dst,
			cvRect(dst_x, dst_y, image_src->width, image_src->height));
	cvCopy(image_src, image_dst, alpha);
	cvResetImageROI(image_dst);
	cvReleaseImage(&alpha);
}
static void copySoftWithAlphaChannel(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y, double weigthSrc) {
	double w1 = weigthSrc;
	double w2 = 1 - weigthSrc;
	int64_t nc = image_src->nChannels;
	for (int64_t y = 0; y < image_src->height; ++y) {
		int64_t dy = y + dst_y;
		if (dy < 0 || dy >= image_dst->height)
			continue;
		uchar *ptrO = (uchar*) (image_src->imageData + image_src->widthStep * y);
		uchar *ptrD =
				(uchar*) (image_dst->imageData + image_dst->widthStep * dy);
		for (int64_t x = 0; x < image_src->width; ++x) {
			int64_t dx = x + dst_x;
			if (dx < 0 || dx >= image_dst->width)
				continue;
			uchar src1 = ptrO[x * nc + 0];
			uchar src2 = ptrO[x * nc + 1];
			uchar src3 = ptrO[x * nc + 2];
			uchar src_alpha = ptrO[x * nc + 3];
			uchar dst1 = ptrD[dx * nc + 0];
			uchar dst2 = ptrD[dx * nc + 1];
			uchar dst3 = ptrD[dx * nc + 2];
			uchar dst_alpha = ptrD[dx * nc + 3];
			if (src_alpha == 0 || src_alpha < dst_alpha)
				continue;
			else if (dst_alpha == 0 || src_alpha > dst_alpha) {
				ptrD[dx * nc + 0] = src1;
				ptrD[dx * nc + 1] = src2;
				ptrD[dx * nc + 2] = src3;
				ptrD[dx * nc + 3] = src_alpha;
			} else {
				ptrD[dx * nc + 0] = my_math_round_uint8(src1 * w1 + dst1 * w2);
				ptrD[dx * nc + 1] = my_math_round_uint8(src2 * w1 + dst2 * w2);
				ptrD[dx * nc + 2] = my_math_round_uint8(src3 * w1 + dst3 * w2);
				ptrD[dx * nc + 3] = MAX(src_alpha, dst_alpha);
			}
		}
	}
}
static void copyHard(IplImage *image_src, IplImage *image_dst, int64_t dst_x,
		int64_t dst_y) {
	cvSetImageROI(image_dst,
			cvRect(dst_x, dst_y, image_src->width, image_src->height));
	cvCopy(image_src, image_dst, NULL);
	cvResetImageROI(image_dst);
}
static void copySoft(IplImage *image_src, IplImage *image_dst, int64_t dst_x,
		int64_t dst_y, double weigthSrc) {
	double w1 = weigthSrc;
	double w2 = 1 - weigthSrc;
	int64_t nc = image_src->nChannels;
	for (int64_t y = 0; y < image_src->height; ++y) {
		int64_t dy = y + dst_y;
		if (dy < 0 || dy >= image_dst->height)
			continue;
		uchar *ptrO = (uchar*) (image_src->imageData + image_src->widthStep * y);
		uchar *ptrD =
				(uchar*) (image_dst->imageData + image_dst->widthStep * dy);
		for (int64_t x = 0; x < image_src->width; ++x) {
			int64_t dx = x + dst_x;
			if (dx < 0 || dx >= image_dst->width)
				continue;
			for (int64_t z = 0; z < nc; ++z)
				ptrD[dx * nc + z] = my_math_round_uint8(
						ptrO[x * nc + z] * w1 + ptrD[dx * nc + z] * w2);
		}
	}
}
void my_image_copyWeightedPixels(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y, double weigthSrc) {
	if (weigthSrc == 0)
		return;
	if (image_src->nChannels == 4) {
		if (weigthSrc == 1) {
			copyHardWithAlphaChannel(image_src, image_dst, dst_x, dst_y);
		} else {
			copySoftWithAlphaChannel(image_src, image_dst, dst_x, dst_y,
					weigthSrc);
		}
	} else {
		if (weigthSrc == 1) {
			copyHard(image_src, image_dst, dst_x, dst_y);
		} else {
			copySoft(image_src, image_dst, dst_x, dst_y, weigthSrc);
		}
	}
}
void my_image_copyPixels(IplImage *image_src, IplImage *image_dst,
		int64_t dst_x, int64_t dst_y) {
	my_image_copyWeightedPixels(image_src, image_dst, dst_x, dst_y, 1);
}
void my_image_fill_color(IplImage *img, CvScalar colFondo) {
	cvRectangle(img, cvPoint(0, 0), cvPoint(img->width - 1, img->height - 1),
			colFondo, CV_FILLED, 8, 0);
}
void my_image_fill_white(IplImage *img) {
	my_image_fill_color(img, cvScalar(255, 255, 255, 0));
}
void my_image_fill_squares_color(IplImage *img, CvScalar colFondo1,
		CvScalar colFondo2) {
	int64_t size_cuadrado = 15;
	int64_t desde_w, desde_h = 0, col, col_ini = 0;
	while (desde_h < img->height) {
		col = col_ini;
		desde_w = 0;
		while (desde_w < img->width) {
			cvRectangle(img, cvPoint(desde_w, desde_h),
					cvPoint(desde_w + size_cuadrado - 1,
							desde_h + size_cuadrado - 1),
					col ? colFondo1 : colFondo2, CV_FILLED, 8, 0);
			desde_w += size_cuadrado;
			col = !col;
		}
		desde_h += size_cuadrado;
		col_ini = !col_ini;
	}
}
void my_image_fill_squares(IplImage *img) {
	my_image_fill_squares_color(img, cvScalar(240, 200, 200, 0),
			cvScalar(200, 240, 200, 0));
}
struct MyConvolutionKernel *my_convolution_newKernel(int64_t width,
		int64_t height) {
	struct MyConvolutionKernel *kc = MY_MALLOC(1, struct MyConvolutionKernel);
	kc->size_x = width;
	kc->size_y = height;
	kc->size = width * height;
	kc->center_x = kc->size_x / 2;
	kc->center_y = kc->size_y / 2;
	kc->valores = MY_MALLOC_MATRIX(kc->size_x, kc->size_y, double);
	return kc;
}
void my_convolution_initKernel(struct MyConvolutionKernel *kc, double *valores) {
	int64_t i, j, pos = 0;
	for (j = 0; j < kc->size_y; ++j)
		for (i = 0; i < kc->size_x; ++i)
			kc->valores[i][j] = valores[pos++];
}
void my_convolution_initAverageKernel(struct MyConvolutionKernel *kc) {
	int64_t i, j;
	for (i = 0; i < kc->size_x; ++i)
		for (j = 0; j < kc->size_y; ++j)
			kc->valores[i][j] = 1.0 / kc->size;
}
void my_convolution_releaseKernel(struct MyConvolutionKernel *kc) {
	if (kc == NULL)
		return;
	MY_FREE_MATRIX(kc->valores, kc->size_x);
	MY_FREE(kc);
}

#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static double priv_calcular_kernel(IplImage *imgIn, int64_t pix_x,
		int64_t pix_y, int64_t nc, struct MyConvolutionKernel *kc) {
	double sum = 0;
	int64_t i, j;
	for (j = 0; j < kc->size_y; ++j) {
		int64_t posy = pix_y - kc->center_y + j;
		if (posy < 0)
			posy = 0;
		else if (posy >= imgIn->height)
			posy = imgIn->height - 1;
		for (i = 0; i < kc->size_x; ++i) {
			int64_t posx = pix_x - kc->center_x + i;
			if (posx < 0)
				posx = 0;
			else if (posx >= imgIn->width)
				posx = imgIn->width - 1;
			double value = 0;
			if (imgIn->depth == IPL_DEPTH_8U)
				value = PIXC_8U(imgIn, posx, posy, nc);
			else
				my_log_error("no soportado por my_convolution_perform\n");
			sum += value * kc->valores[i][j];
		}
	}
	return sum;
}
void my_convolution_perform(IplImage *imgIn, struct MyConvolutionKernel *kc,
		double ***my_convolution_perform) {
	int64_t x, y, nc;
	for (nc = 0; nc < imgIn->nChannels; ++nc) {
		for (y = 0; y < imgIn->height; ++y) {
			for (x = 0; x < imgIn->width; ++x) {
				my_convolution_perform[nc][x][y] = priv_calcular_kernel(imgIn,
						x, y, nc, kc);
			}
		}
	}
}
IplImage *my_image_newFromArray(double **matriz, int64_t width, int64_t height,
		double max_val) {
	IplImage *imagen = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	int64_t x, y;
	if (max_val <= 0) {
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				if (matriz[x][y] > max_val)
					max_val = matriz[x][y];
			}
		}
	}
	for (y = 0; y < height; ++y) {
		uchar *ptr = (uchar*) (imagen->imageData + imagen->widthStep * y);
		for (x = 0; x < width; ++x) {
			if (max_val == 255)
				ptr[x] = (uchar) matriz[x][y];
			else
				ptr[x] = (uchar) (255 * matriz[x][y] / max_val);
			if (matriz[x][y] > max_val)
				my_log_info("atencion, %lf mayor que max_val=%lf\n",
						matriz[x][y], max_val);
		}
	}
	return imagen;
}
void my_image_copyArrayToPixels(double ***my_convolution_perform,
		double max_val, uchar abs, IplImage *imgOut) {
	int64_t x, y, nc;
	if (max_val == 0) {
		for (nc = 0; nc < imgOut->nChannels; ++nc) {
			for (y = 0; y < imgOut->height; ++y) {
				for (x = 0; x < imgOut->width; ++x) {
					double v = my_convolution_perform[nc][x][y];
					if (v > max_val)
						max_val = v;
					if (abs && -v > max_val)
						max_val = -v;
				}
			}
		}
	}
	double escala = max_val == 0 ? 1 : 256.0 / max_val;
	for (nc = 0; nc < imgOut->nChannels; ++nc) {
		for (y = 0; y < imgOut->height; ++y) {
			for (x = 0; x < imgOut->width; ++x) {
				double v = my_convolution_perform[nc][x][y];
				if (abs && v < 0)
					v = -v;
				v *= escala;
				PIXC_8U(imgOut, x, y, nc) = my_math_round_uint8(v);
			}
		}
	}
}
int64_t my_image_copyPixelsToUcharArray(IplImage *imgGray, uchar **prt_buffer) {
	my_assert_equalInt("depth", imgGray->depth, 8);
	my_assert_equalInt("nChannels", imgGray->nChannels, 1);
	MY_REALLOC(*prt_buffer, imgGray->width * imgGray->height, uchar);
	uchar *buffer = *prt_buffer;
	int64_t x, y, cont = 0;
	for (y = 0; y < imgGray->height; ++y) {
		uchar *ptr = (uchar*) (imgGray->imageData + imgGray->widthStep * y);
		for (x = 0; x < imgGray->width; ++x)
			buffer[cont++] = ptr[x];
	}
	return cont;
}
int64_t my_image_copyPixelsToFloatArray(IplImage *imgGray, float **prt_buffer) {
	my_assert_equalInt("depth", imgGray->depth, 8);
	my_assert_equalInt("nChannels", imgGray->nChannels, 1);
	MY_REALLOC(*prt_buffer, imgGray->width * imgGray->height, float);
	float *buffer = *prt_buffer;
	int64_t x, y, cont = 0;
	for (y = 0; y < imgGray->height; ++y) {
		uchar *ptr = (uchar*) (imgGray->imageData + imgGray->widthStep * y);
		for (x = 0; x < imgGray->width; ++x)
			buffer[cont++] = ptr[x] / 255.0;
	}
	return cont;
}
void my_image_pgm_saveBuffer(const char *filePgm, IplImage *imgGray,
		uchar **prt_buffer) {
	int64_t nBytes = my_image_copyPixelsToUcharArray(imgGray, prt_buffer);
	FILE* out = my_io_openFileWrite1(filePgm);
	fprintf(out, "P5\n%i %i\n255\n", imgGray->width, imgGray->height);
	fwrite(*prt_buffer, sizeof(uchar), nBytes, out);
	fclose(out);
}
uchar my_image_pgm_loadBuffer(const char *filePgm, uchar **prt_buffer,
		IplImage **ptr_imgGris) {
	FILE* in = my_io_openFileRead1(filePgm, 0);
	if (in == NULL)
		return 0;
	int64_t w = 0, h = 0, max = 0;
	if (fscanf(in, "P5\n%"PRIi64" %"PRIi64"\n%"PRIi64"\n", &w, &h, &max) != 3) {
		if (fscanf(in, "P5 %"PRIi64" %"PRIi64" %"PRIi64" ", &w, &h, &max) != 3)
			my_log_error("pgm invalid. %s\n", filePgm);
	}
	if (w == 0 || h == 0)
		my_log_error("pgm invalid. %"PRIi64"x%"PRIi64". %s\n", w, h, filePgm);
	if (max < 1 || max > 255)
		my_log_error("pgm invalid. maximo %"PRIi64" [1,255]. %s\n", max,
				filePgm);
	int64_t numBytes = w * h;
	MY_REALLOC(*prt_buffer, numBytes, uchar);
	uchar *buffer = *prt_buffer;
	my_io_readBytesFile(in, buffer, numBytes, 1);
	fclose(in);
	IplImage *img = *ptr_imgGris;
	if (img == NULL || img->width != w || img->height != h || img->depth != 8
			|| img->nChannels != 1) {
		my_image_release(img);
		*ptr_imgGris = cvCreateImage(cvSize(w, h), 8, 1);
		img = *ptr_imgGris;
	}
	int64_t x, y, cont = 0;
	uchar *ptr;
	for (y = 0; y < h; ++y) {
		ptr = (uchar*) (img->imageData + img->widthStep * y);
		for (x = 0; x < w; ++x)
			ptr[x] = buffer[cont++];
	}
	return 1;
}
void my_image_pgm_saveImage(const char *filePgm, IplImage *imgGris) {
	uchar *buffer = NULL;
	my_image_pgm_saveBuffer(filePgm, imgGris, &buffer);
	MY_FREE(buffer);
}
IplImage *my_image_pgm_loadImage(const char *filePgm) {
	uchar *buffer = NULL;
	IplImage *imgGris = NULL;
	uchar ok = my_image_pgm_loadBuffer(filePgm, &buffer, &imgGris);
	MY_FREE(buffer);
	return ok ? imgGris : NULL;
}

struct MyImageResizer {
	uchar isFixed, isBounded, isScale;
	uchar isSwapWhInVerticalVideos;
	double width, height;
	IplImage *imgBuffer;
};
MyImageResizer* my_imageResizer_newFixedSize(int64_t newWidth,
		int64_t newHeight) {
	struct MyImageResizer *resizer = MY_MALLOC(1, struct MyImageResizer);
	resizer->isFixed = 1;
	resizer->width = newWidth;
	resizer->height = newHeight;
	return resizer;
}

#define SCREEN_MARGIN_W 10
#define SCREEN_MARGIN_H 60

void my_getScreenResolution(int *out_width, int *out_height);

MyImageResizer* my_imageResizer_newScreenSize() {
	int maxW = 0, maxH = 0;
	my_getScreenResolution(&maxW, &maxH);
	my_assert_greaterInt("screen width", maxW, 0);
	my_assert_greaterInt("screen height", maxH, 0);
	struct MyImageResizer *resizer = MY_MALLOC(1, struct MyImageResizer);
	resizer->isBounded = 1;
	resizer->width = maxW - SCREEN_MARGIN_W;
	resizer->height = maxH - SCREEN_MARGIN_H;
	return resizer;
}
char *my_imageResizer_getTextOptions() {
	return "[F:fixed|B:bounded|S:scale][V:swapWhInVerticalVideos][width]x[height]";
}
bool my_imageResizer_testTextOptions(const char *option) {
	if (option == NULL)
		return false;
	if (option[0] == 'F' || option[0] == 'B' || option[0] == 'S')
		return true;
	return false;
}
MyImageResizer* my_imageResizer_newTextOptions(const char *option) {
	if (option == NULL)
		return NULL;
	struct MyImageResizer *resizer = MY_MALLOC(1, struct MyImageResizer);
	if (option[0] == 'F') {
		resizer->isFixed = 1;
	} else if (option[0] == 'B') {
		resizer->isBounded = 1;
	} else if (option[0] == 'S') {
		resizer->isScale = 1;
	} else {
		my_log_error("unknown resize option %s\n", option);
	}
	int64_t pos1 = 1;
	if (option[pos1] == 'V') {
		resizer->isSwapWhInVerticalVideos = 1;
		pos1++;
	}
	int64_t pos2 = my_string_indexOf(option, "x");
	if (pos2 < 0) {
		resizer->width = my_parse_double_isubFromEnd(option, pos1);
	} else {
		resizer->width = my_parse_double_isubFromTo(option, pos1, pos2);
		resizer->height = my_parse_double_isubFromEnd(option, pos2 + 1);
	}
	return resizer;
}
static CvSize resolve_newSize_fixed(int64_t currentW, int64_t currentH,
		double resizer_width, double resizer_height, uchar autoRotate) {
	if (autoRotate && currentH > currentW) {
		double tmp = resizer_width;
		resizer_width = resizer_height;
		resizer_height = tmp;
	}
	if (resizer_width > 0 && resizer_height <= 0) {
		double hwRatio = currentH / (double) currentW;
		return cvSize(my_math_round_int(resizer_width),
				my_math_round_int(resizer_width * hwRatio));
	} else if (resizer_width <= 0 && resizer_height > 0) {
		double whRatio = currentW / (double) currentH;
		return cvSize(my_math_round_int(resizer_height * whRatio),
				my_math_round_int(resizer_height));
	} else {
		return cvSize(my_math_round_int(resizer_width),
				my_math_round_int(resizer_height));
	}
}
static CvSize resolve_newSize_bounded(int64_t currentW, int64_t currentH,
		double resizer_width, double resizer_height, uchar autoRotate) {
	if (autoRotate && currentH > currentW) {
		double tmp = resizer_width;
		resizer_width = resizer_height;
		resizer_height = tmp;
	}
	if (currentW <= resizer_width && currentH <= resizer_height)
		return cvSize(currentW, currentH);
	double scaleW = resizer_width / (double) currentW;
	double scaleH = resizer_height / (double) currentH;
	double scale = MIN(scaleW, scaleH);
	return cvSize(my_math_round_int(currentW * scale),
			my_math_round_int(currentH * scale));
}
static CvSize resolve_newSize_scale(int64_t currentW, int64_t currentH,
		double resizer_width, double resizer_height, uchar autoRotate) {
	if (autoRotate && currentH > currentW) {
		double tmp = resizer_width;
		resizer_width = resizer_height;
		resizer_height = tmp;
	}
	if (resizer_width <= 0)
		resizer_width = resizer_height;
	if (resizer_height <= 0)
		resizer_height = resizer_width;
	return cvSize(my_math_round_int(currentW * resizer_width),
			my_math_round_int(currentH * resizer_height));
}
static CvSize resolve_newSize(MyImageResizer *resizer, int64_t currentW,
		int64_t currentH) {
	if (resizer->isFixed) {
		return resolve_newSize_fixed(currentW, currentH, resizer->width,
				resizer->height, resizer->isSwapWhInVerticalVideos);
	} else if (resizer->isBounded) {
		return resolve_newSize_bounded(currentW, currentH, resizer->width,
				resizer->height, resizer->isSwapWhInVerticalVideos);
	} else if (resizer->isScale) {
		return resolve_newSize_scale(currentW, currentH, resizer->width,
				resizer->height, resizer->isSwapWhInVerticalVideos);
	}
	return cvSize(currentW, currentH);
}
//returned image is owned by MyImageResizer (do not free or resize other image).
IplImage* my_imageResizer_resizeImage(IplImage *imageSrc,
		MyImageResizer *resizer) {
	if (resizer == NULL)
		return imageSrc;
	CvSize newSize = resolve_newSize(resizer, imageSrc->width,
			imageSrc->height);
	if (newSize.width == imageSrc->width && newSize.height == imageSrc->height)
		return imageSrc;
	if (resizer->imgBuffer == NULL || resizer->imgBuffer->width != newSize.width
			|| resizer->imgBuffer->height != newSize.height
			|| resizer->imgBuffer->nChannels != imageSrc->nChannels
			|| resizer->imgBuffer->depth != imageSrc->depth) {
		if (resizer->imgBuffer != NULL)
			cvReleaseImage(&resizer->imgBuffer);
		resizer->imgBuffer = cvCreateImage(newSize, imageSrc->depth,
				imageSrc->nChannels);
	}
	//CV_INTER_AREA is recommended for shrinking
	//CV_INTER_CUBIC is recommended for enlargement
	int64_t cv_interpolation =
			(newSize.width < imageSrc->width
					&& newSize.height < imageSrc->height) ?
					CV_INTER_AREA : CV_INTER_CUBIC;
	cvResize(imageSrc, resizer->imgBuffer, cv_interpolation);
	return resizer->imgBuffer;
}
void my_imageResizer_release(MyImageResizer *resizer) {
	if (resizer == NULL)
		return;
	my_image_release(resizer->imgBuffer);
	MY_FREE(resizer);
}

IplImage* averageImages(IplImage **imagesGray, int64_t numImages) {
	CvSize size = cvGetSize(imagesGray[0]);
	double **proms = MY_MALLOC_MATRIX(size.width, size.height, double);
	int64_t k, cont = 0;
	for (k = 0; k < numImages; ++k) {
		IplImage *frameGris = imagesGray[k];
		int64_t x, y;
		cont++;
		uchar *fila;
		for (y = 0; y < frameGris->height; ++y) {
			fila = (uchar*) (frameGris->imageData + frameGris->widthStep * y);
			for (x = 0; x < frameGris->width; ++x) {
				double delta = fila[x] - proms[x][y];
				proms[x][y] += delta / cont;
			}
		}
	}
	IplImage *img = my_image_newFromArray(proms, size.width, size.height, 255);
	MY_FREE_MATRIX(proms, size.width);
	return img;
}
#endif
