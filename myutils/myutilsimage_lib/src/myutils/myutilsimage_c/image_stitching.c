/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "image_stitching.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV
#include <opencv2/imgproc/imgproc_c.h>

#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static void drawBorders(IplImage *img, int64_t border_size) {
	int64_t i, j, nc;
	for (i = 0; i < img->width; ++i) {
		for (j = 0; j < border_size; ++j) {
			for (nc = 0; nc < img->nChannels && nc < 3; ++nc) {
				PIXC_8U(img, i, j, nc) = 0;
				PIXC_8U(img, i, img->height - 1 - j, nc) = 0;
			}
		}
	}
	for (i = 0; i < img->height; ++i) {
		for (j = 0; j < border_size; ++j) {
			for (nc = 0; img->nChannels && nc < 3; ++nc) {
				PIXC_8U(img, j, i, nc) = 0;
				PIXC_8U(img, img->width - 1 - j, i, nc) = 0;
			}
		}
	}
}
static CvPoint projectPoint(int64_t x, int64_t y, CvMat *tmatrix) {
	if (tmatrix->rows == 2) {
		double newX = cvGetReal2D(tmatrix, 0, 0) * x
				+ cvGetReal2D(tmatrix, 0, 1) * y + cvGetReal2D(tmatrix, 0, 2);
		double newY = cvGetReal2D(tmatrix, 1, 0) * x
				+ cvGetReal2D(tmatrix, 1, 1) * y + cvGetReal2D(tmatrix, 1, 2);
		CvPoint p = cvPoint(my_math_round_int(newX), my_math_round_int(newY));
		//log_info("%"PRIi64",%"PRIi64"=>%"PRIi64",%"PRIi64"\n", x, y,p.x,p.y);
		return p;
	} else if (tmatrix->rows == 3) {
		double newX = cvGetReal2D(tmatrix, 0, 0) * x
				+ cvGetReal2D(tmatrix, 0, 1) * y + cvGetReal2D(tmatrix, 0, 2);
		double newY = cvGetReal2D(tmatrix, 1, 0) * x
				+ cvGetReal2D(tmatrix, 1, 1) * y + cvGetReal2D(tmatrix, 1, 2);
		double w = cvGetReal2D(tmatrix, 2, 0) * x
				+ cvGetReal2D(tmatrix, 2, 1) * y + cvGetReal2D(tmatrix, 2, 2);
		newX /= w;
		newY /= w;
		CvPoint p = cvPoint(my_math_round_int(newX), my_math_round_int(newY));
		//log_info("%"PRIi64",%"PRIi64"=>%"PRIi64",%"PRIi64"\n", x, y,p.x,p.y);
		return p;
	}
	return cvPoint(0, 0);
}
static void projectImg(IplImage *src, int64_t TRANS_X, int64_t TRANS_Y,
		IplImage *dst, CvMat *tmatrix) {
	if (tmatrix->rows == 2) {
		//translate
		CvMat* result = cvCreateMat(2, 3, CV_32FC1);
		cvSetReal2D(result, 0, 0, cvGetReal2D(tmatrix, 0, 0));
		cvSetReal2D(result, 0, 1, cvGetReal2D(tmatrix, 0, 1));
		cvSetReal2D(result, 1, 0, cvGetReal2D(tmatrix, 1, 0));
		cvSetReal2D(result, 1, 1, cvGetReal2D(tmatrix, 1, 1));
		cvSetReal2D(result, 0, 2, cvGetReal2D(tmatrix, 0, 2) + TRANS_X);
		cvSetReal2D(result, 1, 2, cvGetReal2D(tmatrix, 1, 2) + TRANS_Y);
		cvWarpAffine(src, dst, result, CV_INTER_LINEAR, cvScalarAll(0));
		cvReleaseMat(&result);
	} else if (tmatrix->rows == 3) {
		//translate matrix
		CvMat* offset = cvCreateMat(3, 3, CV_32FC1);
		cvSetReal2D(offset, 0, 0, 1);
		cvSetReal2D(offset, 0, 1, 0);
		cvSetReal2D(offset, 0, 2, TRANS_X);
		cvSetReal2D(offset, 1, 0, 0);
		cvSetReal2D(offset, 1, 1, 1);
		cvSetReal2D(offset, 1, 2, TRANS_Y);
		cvSetReal2D(offset, 2, 0, 0);
		cvSetReal2D(offset, 2, 1, 0);
		cvSetReal2D(offset, 2, 2, 1);
		//translate
		CvMat* result = cvCreateMat(3, 3, CV_32FC1);
		cvMatMul(offset, tmatrix, result);
		cvWarpPerspective(src, dst, result, CV_INTER_LINEAR, cvScalarAll(0));
		cvReleaseMat(&offset);
		cvReleaseMat(&result);
	}
}
#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static IplImage *remove_borders(IplImage *img) {
	int64_t min_x = img->width, min_y = img->height, max_x = 0, max_y = 0;
	int64_t x, y, nc;
	for (y = 0; y < img->height; ++y) {
		for (x = 0; x < img->width; ++x) {
			for (nc = 0; nc < img->nChannels; ++nc) {
				if (PIXC_8U(img,x,y,3) > 0) {
					if (x < min_x)
						min_x = x;
					if (y < min_y)
						min_y = y;
					if (x > max_x)
						max_x = x;
					if (y > max_y)
						max_y = y;
				}
			}
		}
	}
	int w = max_x - min_x + 1;
	int h = max_y - min_y + 1;
	if (w < 5 || h < 5)
		return NULL;
	IplImage *newImg = cvCreateImage(cvSize(w, h), img->depth, img->nChannels);
	cvSetImageROI(img, cvRect(min_x, min_y, w, h));
	cvCopy(img, newImg, NULL);
	cvResetImageROI(img);
	return newImg;
}
static CvRect getProjectionMBR(IplImage *imgSrc, CvMat *tmatrix) {
	CvPoint p1 = projectPoint(0, 0, tmatrix);
	CvPoint p2 = projectPoint(imgSrc->width - 1, 0, tmatrix);
	CvPoint p3 = projectPoint(0, imgSrc->height - 1, tmatrix);
	CvPoint p4 = projectPoint(imgSrc->width - 1, imgSrc->height - 1, tmatrix);
	int minx = MIN(MIN(p1.x,p2.x), MIN(p3.x, p4.x));
	int miny = MIN(MIN(p1.y,p2.y), MIN(p3.y, p4.y));
	int maxx = MAX(MAX(p1.x,p2.x), MAX(p3.x, p4.x));
	int maxy = MAX(MAX(p1.y,p2.y), MAX(p3.y, p4.y));
	return cvRect(minx - 2, miny - 2, maxx - minx + 5, maxy - miny + 5);
}
static IplImage *convertTo4Channels(IplImage *img) {
	IplImage *newImg = cvCreateImage(cvSize(img->width, img->height),
			img->depth, 4);
	cvRectangle(newImg, cvPoint(0, 0), cvPoint(newImg->width, newImg->height),
			cvScalarAll(255), CV_FILLED, 8, 0);
	if (img->nChannels == 1)
		cvCvtColor(img, newImg, CV_GRAY2BGRA);
	else if (img->nChannels == 3)
		cvCvtColor(img, newImg, CV_BGR2BGRA);
	else if (img->nChannels == 4)
		cvCopy(img, newImg, NULL);
	return newImg;
}
IplImage *generateStitching(IplImage *img_src1, IplImage *img_src2,
		CvMat *tmatrix, uchar with_borders, double blend_weight) {
	if (tmatrix == NULL)
		return NULL;
	if (blend_weight <= 0)
		blend_weight = 1;
	uchar mustFree1 = 0, mustFree2 = 0;
	IplImage *img1 = NULL, *img2 = NULL;
	if (img_src1->nChannels < 4) {
		img1 = convertTo4Channels(img_src1);
		mustFree1 = 1;
	} else {
		img1 = img_src1;
	}
	if (img_src2->nChannels < 4) {
		img2 = convertTo4Channels(img_src2);
		mustFree2 = 1;
	} else {
		img2 = img_src2;
	}
	if (with_borders) {
		drawBorders(img1, 2);
		drawBorders(img2, 2);
	}
	CvRect mbr = getProjectionMBR(img1, tmatrix);
	int64_t width = MAX(img2->width,mbr.x + mbr.width) - MIN(0, mbr.x);
	int64_t height = MAX(img2->height,mbr.y + mbr.height) - MIN(0, mbr.y);
	int64_t orig_x = MAX(0, -mbr.x);
	int64_t orig_y = MAX(0, -mbr.y);
	IplImage *prjTmp = cvCreateImage(cvSize(width, height), img1->depth,
			img1->nChannels);
	projectImg(img1, orig_x, orig_y, prjTmp, tmatrix);
	IplImage *final = cvCreateImage(cvSize(width, height), img1->depth,
			img1->nChannels);
	//cvSetZero(final);
	cvRectangle(final, cvPoint(0, 0), cvPoint(width, height), cvScalarAll(255),
			-1, 8, 0);
	my_image_copyPixels(img2, final, orig_x, orig_y);
	my_image_copyWeightedPixels(prjTmp, final, 0, 0, blend_weight);
	cvReleaseImage(&prjTmp);
	if (mustFree1)
		cvReleaseImage(&img1);
	if (mustFree2)
		cvReleaseImage(&img2);
	IplImage *cropped = remove_borders(final);
	if (cropped == NULL) {
		return final;
	} else {
		cvReleaseImage(&final);
		return cropped;
	}
}
#endif
