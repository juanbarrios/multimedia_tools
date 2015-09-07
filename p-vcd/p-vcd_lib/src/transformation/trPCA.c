/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct TR_PCA {
	IplImage *imgPreRot, *imgPreRotGris, *imgPosRot, *imgCrop;
	IplImage *imgTmpGray, *imgDevX, *imgDevY, *imgGradient;
	double threshold;
	uchar debug;
};

static void tra_config_pca(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct TR_PCA *es = MY_MALLOC(1, struct TR_PCA);
	es->threshold = my_tokenizer_nextDouble(tk);
	if (my_tokenizer_isNext(tk, "DEBUG"))
		es->debug = 1;
	*out_state = es;
}
static IplImage *calculate_borders(IplImage *imgIn, struct TR_PCA *es) {
	IplImage *imgGray;
	if (imgIn->nChannels > 1) {
		if (es->imgTmpGray == NULL )
			es->imgTmpGray = cvCreateImage(cvGetSize(imgIn), IPL_DEPTH_8U, 1);
		cvCvtColor(imgIn, es->imgTmpGray, CV_BGR2GRAY);
		cvSmooth(es->imgTmpGray, es->imgTmpGray, CV_GAUSSIAN, 5, 5, 0, 0);
		cvSmooth(es->imgTmpGray, es->imgTmpGray, CV_GAUSSIAN, 5, 5, 0, 0);
		cvSmooth(es->imgTmpGray, es->imgTmpGray, CV_GAUSSIAN, 5, 5, 0, 0);
		cvSmooth(es->imgTmpGray, es->imgTmpGray, CV_GAUSSIAN, 5, 5, 0, 0);
		imgGray = es->imgTmpGray;
	} else {
		imgGray = imgIn;
	}
	if (es->imgGradient == NULL ) {
		es->imgDevX = cvCreateImage(cvGetSize(imgIn), IPL_DEPTH_16S, 1);
		es->imgDevY = cvCreateImage(cvGetSize(imgIn), IPL_DEPTH_16S, 1);
		es->imgGradient = cvCreateImage(cvGetSize(imgIn), IPL_DEPTH_8U, 1);
	}
	cvSobel(imgGray, es->imgDevX, 1, 0, 3);
	cvSobel(imgGray, es->imgDevY, 0, 1, 3);
	int64_t x, y;
	for (y = 0; y < imgGray->height; ++y) {
		short *ptrX = (short*) (es->imgDevX->imageData
				+ es->imgDevX->widthStep * y);
		short *ptrY = (short*) (es->imgDevY->imageData
				+ es->imgDevY->widthStep * y);
		uchar *ptr = (uchar*) (es->imgGradient->imageData
				+ es->imgGradient->widthStep * y);
		for (x = 0; x < imgGray->width; ++x) {
			//double val = ABS(ptrX[x]) + ABS(ptrY[x]);
			double val = sqrt(
					(ptrX[x] * (double) ptrX[x])
							+ (ptrY[x] * (double) ptrY[x]));
			ptr[x] = (val >= es->threshold) ? 255 : 0;
		}
	}
	IplConvKernel *kernel = cvCreateStructuringElementEx(3, 3, 1, 1,
			CV_SHAPE_ELLIPSE, NULL );
	cvErode(es->imgGradient, es->imgGradient, kernel, 1);
	cvDilate(es->imgGradient, es->imgGradient, kernel, 1);
	cvReleaseStructuringElement(&kernel);
	return es->imgGradient;
}
struct MBR {
	CvPoint2D32f ul, ur, bl, br;
};
static struct MBR calculate_MBR(IplImage *imgBin, int64_t fromx, int64_t fromy,
		int64_t tox, int64_t toy) {
	int64_t min_x = imgBin->width, min_y = imgBin->height, max_x = 0, max_y = 0;
	int64_t x, y;
	for (y = fromy; y < toy; ++y) {
		uchar *ptr = (uchar*) (imgBin->imageData + imgBin->widthStep * y);
		for (x = fromx; x < tox; ++x) {
			if (ptr[x] >= 255) {
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
	struct MBR mbr;
	mbr.ul = cvPoint2D32f(min_x, min_y);
	mbr.ur = cvPoint2D32f(max_x, min_y);
	mbr.bl = cvPoint2D32f(min_x, max_y);
	mbr.br = cvPoint2D32f(max_x, max_y);
	return mbr;
}
static CvPoint2D32f rotate_point(CvPoint2D32f p, CvMat* mapMatrix) {
	return cvPoint2D32f(
			p.x * cvGetReal2D(mapMatrix, 0, 0)
					+ p.y * cvGetReal2D(mapMatrix, 0, 1)
					+ cvGetReal2D(mapMatrix, 0, 2),
			p.x * cvGetReal2D(mapMatrix, 1, 0)
					+ p.y * cvGetReal2D(mapMatrix, 1, 1)
					+ cvGetReal2D(mapMatrix, 1, 2));
}
static struct MBR rotate_MBR(struct MBR mbr, CvMat* mapMatrix) {
	struct MBR new_mbr;
	new_mbr.ul = rotate_point(mbr.ul, mapMatrix);
	new_mbr.ur = rotate_point(mbr.ur, mapMatrix);
	new_mbr.bl = rotate_point(mbr.bl, mapMatrix);
	new_mbr.br = rotate_point(mbr.br, mapMatrix);
	return new_mbr;
}
static IplImage *tra_transformar_pca(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct TR_PCA *es = estado;
	IplImage *imgBorders = calculate_borders(imagen, es);
	struct MBR borderMBR = calculate_MBR(imgBorders, 0, 0,
			imgBorders->width - 1, imgBorders->height - 1);
	int64_t x, y, cont = 0;
	for (y = borderMBR.ul.y; y <= borderMBR.br.y; ++y) {
		uchar *ptr =
				(uchar*) (imgBorders->imageData + imgBorders->widthStep * y);
		for (x = borderMBR.ul.x; x <= borderMBR.br.x; ++x) {
			if (ptr[x] >= 255)
				cont++;
		}
	}
	int64_t tot = imagen->width * imagen->height;
	if (cont <= 100 || cont >= (tot / 2))
		return imagen;
	const CvArr** points = MY_MALLOC_NOINIT(cont, const CvArr*);
	int64_t pos = 0;
	for (y = borderMBR.ul.y; y <= borderMBR.br.y; ++y) {
		uchar *ptr =
				(uchar*) (imgBorders->imageData + imgBorders->widthStep * y);
		for (x = borderMBR.ul.x; x <= borderMBR.br.x; ++x) {
			if (ptr[x] >= 255) {
				CvMat *vec = cvCreateMat(2, 1, CV_16UC1);
				cvSetReal2D(vec, 0, 0, x);
				cvSetReal2D(vec, 1, 0, y);
				points[pos] = vec;
				pos++;
			}
			if (es->debug)
				ptr[x] = (ptr[x] >= es->threshold) ? 255 : 0;
		}
	}
	my_assert_equalInt("cont", cont, pos);
//covariances
	CvMat *covMat = cvCreateMat(2, 2, CV_32FC1);
	CvMat *avg = cvCreateMat(2, 1, CV_32FC1);
	cvCalcCovarMatrix(points, cont, covMat, avg, CV_COVAR_NORMAL);
	double avg_x = cvGetReal2D(avg, 0, 0);
	double avg_y = cvGetReal2D(avg, 1, 0);
	double cov_xx = cvGetReal2D(covMat, 0, 0);
	double cov_xy = cvGetReal2D(covMat, 0, 1);
	double cov_yx = cvGetReal2D(covMat, 1, 0);
	double cov_yy = cvGetReal2D(covMat, 1, 1);
//eigen vectors
	CvMat* evects = cvCreateMat(2, 2, CV_32FC1);
	CvMat* evals = cvCreateMat(2, 1, CV_32FC1);
	cvEigenVV(covMat, evects, evals, DBL_EPSILON, -1, -1);
	double eval1 = cvGetReal2D(evals, 0, 0);
	double eval2 = cvGetReal2D(evals, 1, 0);
	double evec1_x = cvGetReal2D(evects, 0, 0);
	double evec1_y = cvGetReal2D(evects, 0, 1);
	double evec2_x = cvGetReal2D(evects, 1, 0);
	double evec2_y = cvGetReal2D(evects, 1, 1);
	double angle = MY_RAD2GRAD(atan(evec1_y /evec1_x));
	cvReleaseMat(&covMat);
	cvReleaseMat(&avg);
	cvReleaseMat(&evects);
	cvReleaseMat(&evals);
	for (pos = 0; pos < cont; ++pos) {
		CvMat *vec = (CvMat *) points[pos];
		cvReleaseMat(&vec);
	}
	MY_FREE(points);
	if (es->debug) {
		int64_t LARGO1 = 200;
		int64_t LARGO2 = LARGO1*eval2/eval1;//250;
		int64_t LARGO3 = 5;
		int64_t WIDTH1 = 5;
		int64_t WIDTH2 = 2;
		CvScalar color = cvScalar(40, 40, 255, 0);
		IplImage *imgCol = cvCreateImage(cvGetSize(imgBorders), IPL_DEPTH_8U,
				3);
		cvCvtColor(imgBorders, imgCol, CV_GRAY2BGR);
		my_log_info("hay %"PRIi64" int\n", cont);
		my_log_info("cov %1.1lf %1.1lf %1.1lf %1.1lf\n", cov_xx, cov_xy, cov_yx,
				cov_yy);
		my_log_info("avg %1.1lf %1.1lf\n", avg_x, avg_y);
		my_log_info("evals %1.1lf %1.1lf\n", eval1 / 100000, eval2 / 100000);
		my_log_info("evect %1.1lf %1.1lf\n   %1.1lf %1.1lf\n", evec1_x, evec1_y,
				evec2_x, evec2_y);
		cvCircle(imgCol, cvPoint(avg_x, avg_y), LARGO3, color, WIDTH2, 8, 0);
		cvLine(imgCol, cvPoint(avg_x, avg_y),
				cvPoint(avg_x + LARGO2 * evec2_x, avg_y + LARGO2 * evec2_y),
				color, WIDTH1, 8, 0);
		cvLine(imgCol, cvPoint(avg_x, avg_y),
				cvPoint(avg_x + LARGO1 * evec1_x, avg_y + LARGO1 * evec1_y),
				color, WIDTH1, 8, 0);
		my_log_info("angle=%1.1lf\n", angle);
		//cvSaveImage("argo.png", imgCol, NULL );
		return imgCol;
	}
	//rotate
	if (es->imgPosRot == NULL )
		es->imgPosRot = cvCreateImage(cvGetSize(imagen), IPL_DEPTH_8U,
				imagen->nChannels);
	CvMat* mapMatrix = cvCreateMat(2, 3, CV_32FC1);
	cv2DRotationMatrix(cvPoint2D32f(avg_x, avg_y), angle, 1, mapMatrix);
	cvWarpAffine(imagen, es->imgPosRot, mapMatrix,
			CV_INTER_CUBIC + CV_WARP_FILL_OUTLIERS, cvScalarAll(255));
	//rotate MBR
	IplImage *imgRotBorders = calculate_borders(es->imgPosRot, es);
	struct MBR rotMBR = rotate_MBR(borderMBR, mapMatrix);
	int64_t minx = MIN(MIN(rotMBR.ul.x,rotMBR.ur.x),MIN(rotMBR.bl.x,rotMBR.br.x));
	int64_t miny = MIN(MIN(rotMBR.ul.y,rotMBR.ur.y),MIN(rotMBR.bl.y,rotMBR.br.y));
	int64_t maxx = MAX(MAX(rotMBR.ul.x,rotMBR.ur.x),MAX(rotMBR.bl.x,rotMBR.br.x));
	int64_t maxy = MAX(MAX(rotMBR.ul.y,rotMBR.ur.y),MAX(rotMBR.bl.y,rotMBR.br.y));
	minx = MAX(MIN(minx,imgRotBorders->width-1),0);
	maxx = MAX(MIN(maxx,imgRotBorders->width-1),0);
	miny = MAX(MIN(miny,imgRotBorders->height-1),0);
	maxy = MAX(MIN(maxy,imgRotBorders->height-1),0);
	struct MBR newMBR = calculate_MBR(imgRotBorders, minx, miny, maxx, maxy);
	int64_t MARGIN = 10;
	newMBR.ul.x = MAX(0,newMBR.ul.x - MARGIN);
	newMBR.ul.y = MAX(0,newMBR.ul.y - MARGIN);
	newMBR.br.x = MIN(imgRotBorders->width-1,newMBR.br.x + MARGIN);
	newMBR.br.y = MIN(imgRotBorders->height-1,newMBR.br.y + MARGIN);
	CvRect mbrP = cvRect(newMBR.ul.x, newMBR.ul.y,
			newMBR.br.x - newMBR.ul.x + 1, newMBR.br.y - newMBR.ul.y + 1);
	if (mbrP.width < 2 * MARGIN + 5 || mbrP.height < 2 * MARGIN + 5)
		return imagen;
	if (es->imgCrop == NULL || es->imgCrop->width != mbrP.width
			|| es->imgCrop->height != mbrP.height) {
		my_image_release(es->imgCrop);
		es->imgCrop = cvCreateImage(cvSize(mbrP.width, mbrP.height),
				IPL_DEPTH_8U, es->imgPosRot->nChannels);
	}
	cvSetImageROI(es->imgPosRot, mbrP);
	cvCopy(es->imgPosRot, es->imgCrop, NULL );
	cvResetImageROI(es->imgPosRot);
	return es->imgCrop;
}
static void tra_release_pca(void *estado) {
	struct TR_PCA *es = estado;
	my_image_release(es->imgPreRot);
	my_image_release(es->imgPreRotGris);
	my_image_release(es->imgCrop);
	MY_FREE(es);
}
void tra_reg_pca() {
	Transform_Def *def = newTransformDef("PCA", "binThreshold[_DEBUG]");
	def->func_new = tra_config_pca;
	def->func_transform_frame = tra_transformar_pca;
	def->func_release = tra_release_pca;
}
#endif
