/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"
#include <metricknn/metricknn_c/metricknn_impl.h>

#ifndef NO_OPENCV
#define ANCHO_MIN 85
#define ALTO_MIN  45
#define ANCHO_MAX 160
#define ALTO_MAX  120

#define PIP_VER_PASO1 0
#define PIP_VER_PASO2 1
#define PIP_VER_PASO3 0

struct Pip_Evaluacion {
	int64_t width, height;
	double **limTL, **limTR, **limBL, **limBR, **limH, **limV;
	int64_t xBL, yBL, xBR, yBR, xTR, yTR, xTL, yTL;
	double thTL, thTR, thBL, thBR, thH, thV;
	double mejorThreshold;
	int64_t mejor_xBL, mejor_yBL, mejor_xBR, mejor_yBR, mejor_xTR, mejor_yTR,
			mejor_xTL, mejor_yTL;
};
static void evaluar(struct Pip_Evaluacion *eval) {
	int64_t width = eval->width, height = eval->height, xBL = eval->xBL, yBL =
			eval->yBL, xBR = eval->xBR, yBR = eval->yBR, xTR = eval->xTR, yTR =
			eval->yTR, xTL = eval->xTL, yTL = eval->yTL;
	if (xBL < 30 && yBL > height - 30 && xBR > width - 30 && yBR > height - 30
			&& xTR > width - 30 && yTR < 30 && xTL < 30 && yTL < 30)
		return;
	if (xBL < 30 && xBR > width - 30)
		return;
	if (yBL > height - 30 && yTL < 30)
		return;
	double w = xBR - xTL, h = yBR - yTL;
	if (w / h < 0.95 || w / h > 2.15)
		return;
	double thresh = 0;
	int64_t i, n = 0;
	if (xBL > 0 && yBL < eval->height - 1) {
		thresh += eval->limBL[xBL][yBL];
		n++;
	}
	if (xBR < eval->width - 1 && yBR < eval->height - 1) {
		thresh += eval->limBR[xBR][yBR];
		n++;
	}
	if (xTR < eval->width - 1 && yTR > 0) {
		thresh += eval->limTR[xTR][yTR];
		n++;
	}
	if (xTL > 0 && yTL > 0) {
		thresh += eval->limTL[xTL][yTL];
		n++;
	}
	if (xBR < width - 1) {
		double p = 0;
		for (i = yTR; i <= yBR; ++i) {
			if (eval->limV[xBR][i] <= eval->thV)
				continue;
			p += eval->limV[xBR][i];
		}
		p /= yBR - yTR;
		thresh += p;
		n++;
	}
	if (yBR < height - 1) {
		double p = 0;
		for (i = xBL; i <= xBR; ++i) {
			if (eval->limH[i][yBR] <= eval->thH)
				continue;
			p += eval->limH[i][yBR];
		}
		p /= xBR - xBL;
		thresh += p;
		n++;
	}
	if (xTL > 0) {
		double p = 0;
		for (i = yTL; i <= yBL; ++i) {
			if (eval->limV[xBL][i] <= eval->thV)
				continue;
			p += eval->limV[xBL][i];
		}
		p /= yBL - yTL;
		thresh += p;
		n++;
	}
	if (yTL > 0) {
		double p = 0;
		for (i = xTL; i <= xTR; ++i) {
			if (eval->limH[i][yTL] <= eval->thH)
				continue;
			p += eval->limH[i][yTL];
		}
		p /= xTR - xTL;
		thresh += p;
		n++;
	}
	thresh = thresh / n;
	/*
	 tres = (eval->limBL[xBL][yBL] + eval->limBR[xBR][yBR]
	 + eval->limTR[xTR][yTR] + eval->limTL[xTL][yTL]) / 4.0;
	 thresh = 4.0 / ((1.0 / eval->limBL[xBL][yBL]) + (1.0
	 / eval->limBR[xBR][yBR]) + (1.0 / eval->limTR[xTR][yTR]) + (1.0
	 / eval->limTL[xTL][yTL]));*/
	if (0)
		my_log_info(
				"BL(%"PRIi64",%"PRIi64")%1.2lf-BR(%"PRIi64",%"PRIi64")%1.2lf-TR(%"PRIi64",%"PRIi64")%1.2lf-TL(%"PRIi64",%"PRIi64")%1.2lf   TH=%1.4lf\n",
				xBL, yBL, eval->limBL[xBL][yBL], xBR, yBR,
				eval->limBR[xBR][yBR], xTR, yTR, eval->limTR[xTR][yTR], xTL,
				yTL, eval->limTL[xTL][yTL], thresh);
	if (thresh > 60 && thresh > eval->mejorThreshold) {
		eval->mejorThreshold = thresh;
		eval->mejor_xBL = xBL;
		eval->mejor_yBL = yBL;
		eval->mejor_xBR = xBR;
		eval->mejor_yBR = yBR;
		eval->mejor_xTR = xTR;
		eval->mejor_yTR = yTR;
		eval->mejor_xTL = xTL;
		eval->mejor_yTL = yTL;
		if (0)
			my_log_info(
					"BL(%"PRIi64",%"PRIi64")%1.2lf-BR(%"PRIi64",%"PRIi64")%1.2lf-TR(%"PRIi64",%"PRIi64")%1.2lf-TL(%"PRIi64",%"PRIi64")%1.2lf   TH=%1.4lf\n",
					xBL, yBL, eval->limBL[xBL][yBL], xBR, yBR,
					eval->limBR[xBR][yBR], xTR, yTR, eval->limTR[xTR][yTR], xTL,
					yTL, eval->limTL[xTL][yTL], thresh);
	}
}
static void probarTL(struct Pip_Evaluacion *eval) {
	for (eval->xTL = MAX(0, eval->xBL - 1);
			eval->xTL < MIN(eval->width, eval->xBL + 1); ++eval->xTL) {
		for (eval->yTL = MAX(0, eval->yTR - 1);
				eval->yTL < MIN(eval->height, eval->yTR + 1); ++eval->yTL) {
			if (eval->limTL[eval->xTL][eval->yTL] <= eval->thTL)
				continue;
			evaluar(eval);
		}
	}
}
static void probarTR(struct Pip_Evaluacion *eval) {
	for (eval->xTR = MAX(0, eval->xBR - 1);
			eval->xTR < MIN(eval->xBR + 1, eval->width); ++eval->xTR) {
		for (eval->yTR = eval->yBR - ALTO_MIN;
				eval->yTR >= 0 && eval->yTR > eval->yBR - ALTO_MAX;
				--eval->yTR) {
			if (eval->limTR[eval->xTR][eval->yTR] <= eval->thTR)
				continue;
			probarTL(eval);
		}
	}
}
static void probarBR(struct Pip_Evaluacion *eval) {
	for (eval->xBR = eval->xBL + ANCHO_MIN;
			eval->xBR < eval->width && eval->xBR < eval->xBL + ANCHO_MAX;
			++eval->xBR) {
		for (eval->yBR = MAX(0, eval->yBL - 1);
				eval->yBR < MIN(eval->yBL + 1, eval->height); ++eval->yBR) {
			if (eval->limBR[eval->xBR][eval->yBR] <= eval->thBR)
				continue;
			probarTR(eval);
		}
	}

}
void probarBL(struct Pip_Evaluacion *eval) {
	for (eval->xBL = 0; eval->xBL < eval->width; ++eval->xBL) {
		for (eval->yBL = 0; eval->yBL < eval->height; ++eval->yBL) {
			if (eval->limBL[eval->xBL][eval->yBL] <= eval->thBL)
				continue;
			probarBR(eval);
		}
	}
}
////////
static void probarBL_2(struct Pip_Evaluacion *eval) {
	for (eval->xBL = MAX(0, eval->xTL - 1);
			eval->xBL < MIN(eval->width, eval->xTL + 1); ++eval->xBL) {
		for (eval->yBL = MAX(0, eval->yBR - 1);
				eval->yBL < MIN(eval->height, eval->yBR + 1); ++eval->yBL) {
			if (eval->limBL[eval->xBL][eval->yBL] <= eval->thBL)
				continue;
			evaluar(eval);
		}
	}
}
static void probarTR_2(struct Pip_Evaluacion *eval) {
	for (eval->xTR = MAX(0, eval->xBR - 1);
			eval->xTR < MIN(eval->width, eval->xBR + 1); ++eval->xTR) {
		for (eval->yTR = MAX(0, eval->yTL - 1);
				eval->yTR < MIN(eval->height, eval->yTL + 1); ++eval->yTR) {
			if (eval->limTR[eval->xTR][eval->yTR] <= eval->thTR)
				continue;
			probarBL_2(eval);
		}
	}
}
static void probarBR_2(struct Pip_Evaluacion *eval) {
	for (eval->xBR = eval->xTL + ANCHO_MIN;
			eval->xBR < eval->width && eval->xBR < eval->xTL + ANCHO_MAX;
			++eval->xBR) {
		for (eval->yBR = eval->yTL + ALTO_MIN;
				eval->yBR < eval->height && eval->yBR < eval->yTL + ALTO_MAX;
				++eval->yBR) {
			if (eval->limBR[eval->xBR][eval->yBR] <= eval->thBR)
				continue;
			probarTR_2(eval);
		}
	}
}
static void probarTL_2(struct Pip_Evaluacion *eval) {
	for (eval->xTL = 0; eval->xTL < eval->width; ++eval->xTL) {
		for (eval->yTL = 0; eval->yTL < eval->height; ++eval->yTL) {
			if (eval->limTL[eval->xTL][eval->yTL] <= eval->thTL)
				continue;
			probarBR_2(eval);
		}
	}
}
#define QUANTILE_THRESHOLD(quant) quant.a0_9

static double getQuantileThreshold(double **lim, int64_t width, int64_t height) {
	return 0;
	double *values = MY_MALLOC_NOINIT(width * height, double);
	int64_t x, y, pos = 0;
	for (x = 0; x < width; ++x) {
		for (y = 0; y < height; ++y) {
			values[pos++] = lim[x][y];
		}
	}
	my_assert_equalInt("pos", pos, width * height);
	struct MyQuantiles quant = my_math_computeQuantiles(width * height, values);
	MY_FREE(values);
	return QUANTILE_THRESHOLD(quant);
}
struct Pip_Rectangulo {
	uchar asignado;
	int64_t x, y, w, h;
};
static struct Pip_Rectangulo pip_detectar_rectangulo(double **limTL,
		double **limTR, double **limBL, double **limBR, double **limH,
		double **limV, int64_t width, int64_t height) {
	struct Pip_Evaluacion eval = { 0 };
	eval.limTL = limTL;
	eval.limTR = limTR;
	eval.limBL = limBL;
	eval.limBR = limBR;
	eval.limH = limH;
	eval.limV = limV;
	eval.width = width;
	eval.height = height;
	eval.thTL = getQuantileThreshold(limTL, width, height);
	eval.thTR = getQuantileThreshold(limTR, width, height);
	eval.thBL = getQuantileThreshold(limBL, width, height);
	eval.thBR = getQuantileThreshold(limBR, width, height);
	eval.thH = getQuantileThreshold(limH, width, height);
	eval.thV = getQuantileThreshold(limV, width, height);
	//log_info("umbrales thTL=%lf thTR=%lf thBL=%lf thBR=%lf\n", eval.thTL,
	//eval.thTR, eval.thBL, eval.thBR);
	eval.mejorThreshold = 0;
	//probarBL(&eval);
	probarTL_2(&eval);
	if (eval.mejorThreshold > 0) {
		my_log_info(
				"(%"PRIi64",%"PRIi64")%1.2lf-(%"PRIi64",%"PRIi64")%1.2lf-(%"PRIi64",%"PRIi64")%1.2lf-(%"PRIi64",%"PRIi64")%1.2lf    TH=%1.4lf\n",
				eval.mejor_xBL, eval.mejor_yBL,
				limBL[eval.mejor_xBL][eval.mejor_yBL], eval.mejor_xBR,
				eval.mejor_yBR, limBR[eval.mejor_xBR][eval.mejor_yBR],
				eval.mejor_xTR, eval.mejor_yTR,
				limTR[eval.mejor_xTR][eval.mejor_yTR], eval.mejor_xTL,
				eval.mejor_yTL, limTL[eval.mejor_xTL][eval.mejor_yTL],
				eval.mejorThreshold);
		struct Pip_Rectangulo rect;
		rect.asignado = 1;
		rect.x = eval.mejor_xTL;
		rect.y = eval.mejor_yTL;
		//sin incluir el borde
		rect.w = eval.mejor_xBR - eval.mejor_xTL - 1;
		rect.h = eval.mejor_yBR - eval.mejor_yTL - 1;
		return rect;
	} else {
		struct Pip_Rectangulo rect = { 0 };
		rect.asignado = 0;
		return rect;
	}
}

static struct Pip_Rectangulo detectarRectanguloPIP(double **arrayBordes,
		int64_t arr_width, int64_t arr_height) {
	int64_t width = arr_width + 2;
	int64_t height = arr_height + 2;
	double **bordes = MY_MALLOC_MATRIX(width, height, double);
	int64_t x, y;
	for (y = 0; y < height; ++y) {
		if (y == 0 || y == height - 1) {
			for (x = 0; x < width; ++x)
				bordes[x][y] = 255;
		} else {
			for (x = 0; x < width; ++x) {
				if (x == 0 || x == width - 1)
					bordes[x][y] = 255;
				else
					bordes[x][y] = arrayBordes[x - 1][y - 1];
			}
		}
	}
	double **limTL = MY_MALLOC_MATRIX(width, height, double);
	double **limTR = MY_MALLOC_MATRIX(width, height, double);
	double **limBL = MY_MALLOC_MATRIX(width, height, double);
	double **limBR = MY_MALLOC_MATRIX(width, height, double);
	double **limH = MY_MALLOC_MATRIX(width, height, double);
	double **limV = MY_MALLOC_MATRIX(width, height, double);
	int64_t dist, LARGO = 20, CERO = 1000;
	double sumaT, sumaL, sumaB, sumaR;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			if (1) {
				if (bordes[x][y] > 0)
					sumaT = sumaL = sumaB = sumaR = 1.0 / bordes[x][y];
				else
					sumaT = sumaL = sumaB = sumaR = CERO;
				for (dist = 1; dist <= LARGO; ++dist) {
					if (x + dist < width && bordes[x + dist][y] > 0)
						sumaL += 1.0 / bordes[x + dist][y];
					else
						sumaL += CERO;
					if (x - dist >= 0 && bordes[x - dist][y] > 0)
						sumaR += 1.0 / bordes[x - dist][y];
					else
						sumaR += CERO;
					if (y + dist < height && bordes[x][y + dist] > 0)
						sumaT += 1.0 / bordes[x][y + dist];
					else
						sumaT += CERO;
					if (y - dist >= 0 && bordes[x][y - dist] > 0)
						sumaB += 1.0 / bordes[x][y - dist];
					else
						sumaB += CERO;
				}
				sumaT = (LARGO + 1) / sumaT;
				sumaL = (LARGO + 1) / sumaL;
				sumaB = (LARGO + 1) / sumaB;
				sumaR = (LARGO + 1) / sumaR;
				//limTL[x][y] = 2.0 / (1 / sumaT + 1 / sumaL);
				//limTR[x][y] = 2.0 / (1 / sumaT + 1 / sumaR);
				//limBL[x][y] = 2.0 / (1 / sumaB + 1 / sumaL);
				//limBR[x][y] = 2.0 / (1 / sumaB + 1 / sumaR);
				//limH[x][y] = 2.0 / (1 / sumaL + 1 / sumaR);
				//limV[x][y] = 2.0 / (1 / sumaT + 1 / sumaB);
				double f =
						x == 0 || y == 0 || x == width - 1 || y == height - 1 ?
								0 : 2.0;
				limTL[x][y] = 2.0
						/ (1 / MAX(0.001, sumaT - f * sumaB)
								+ 1 / MAX(0.1, sumaL - f * sumaR));
				limTR[x][y] = 2.0
						/ (1 / MAX(0.001, sumaT - f * sumaB)
								+ 1 / MAX(0.1, sumaR - f * sumaL));
				limBL[x][y] = 2.0
						/ (1 / MAX(0.001, sumaB - f * sumaT)
								+ 1 / MAX(0.1, sumaL - f * sumaR));
				limBR[x][y] = 2.0
						/ (1 / MAX(0.001, sumaB - f * sumaT)
								+ 1 / MAX(0.1, sumaR - f * sumaL));
				limH[x][y] = 1
						/ ((1 / sumaL + 1 / sumaR) * (LARGO + 1)
								/ (2 * LARGO + 2));
				limV[x][y] = 1
						/ ((1 / sumaT + 1 / sumaB) * (LARGO + 1)
								/ (2 * LARGO + 2));
			} else {
				sumaT = sumaL = sumaB = sumaR = bordes[x][y];
				for (dist = 1; dist <= LARGO; ++dist) {
					if (x + dist < width)
						sumaL += bordes[x + dist][y];
					if (x - dist >= 0)
						sumaR += bordes[x - dist][y];
					if (y + dist < height)
						sumaT += bordes[x][y + dist];
					if (y - dist >= 0)
						sumaB += bordes[x][y - dist];
				}
				sumaT = sumaT / (LARGO + 1);
				sumaL = sumaL / (LARGO + 1);
				sumaB = sumaB / (LARGO + 1);
				sumaR = sumaR / (LARGO + 1);
				double f =
						x == 0 || y == 0 || x == width - 1 || y == height - 1 ?
								0 : 1.0;
				limTL[x][y] = MAX(0,
						(sumaT + sumaL - f * sumaB - f * sumaR) / 2.0);
				limTR[x][y] = MAX(0,
						(sumaT + sumaR - f * sumaB - f * sumaL) / 2.0);
				limBL[x][y] = MAX(0,
						(sumaB + sumaL - f * sumaT - f * sumaR) / 2.0);
				limBR[x][y] = MAX(0,
						(sumaB + sumaR - f * sumaT - f * sumaL) / 2.0);
				limH[x][y] = MAX(0,
						(sumaL + sumaR - f * sumaT - f * sumaB) / 2.0);
				limV[x][y] = MAX(0,
						(sumaT + sumaB - f * sumaL - f * sumaR) / 2.0);
			}
		}
	}
	struct Pip_Rectangulo rect = pip_detectar_rectangulo(limTL, limTR, limBL,
			limBR, limH, limV, width, height);
#if PIP_VER_PASO3
	IplImage *pip = cvCreateImage(cvSize(arr_width, arr_height), 8, 3);
	cvSetZero(pip);
	if (rect.asignado) {
		cvRectangle(pip, cvPoint(rect.x, rect.y),
				cvPoint(rect.x + rect.w, rect.y + rect.h), cvScalarAll(255), 1,
				8, 0);
	}
	cvShowImage("bordes", my_image_newFromArray(bordes, arr_width, arr_height, 0));
	cvShowImage("tl", my_image_newFromArray(limTL, width, height, 0));
	cvShowImage("tr", my_image_newFromArray(limTR, width, height, 0));
	cvShowImage("bl", my_image_newFromArray(limBL, width, height, 0));
	cvShowImage("br", my_image_newFromArray(limBR, width, height, 0));
	cvShowImage("h", my_image_newFromArray(limH, width, height, 0));
	cvShowImage("v", my_image_newFromArray(limV, width, height, 0));
	cvShowImage("pip", pip);
	if (cvWaitKey(0) == 27)
	pvcd_system_close_error();
	cvDestroyWindow("bordes");
	cvDestroyWindow("tl");
	cvDestroyWindow("tr");
	cvDestroyWindow("bl");
	cvDestroyWindow("br");
	cvDestroyWindow("h");
	cvDestroyWindow("v");
	cvDestroyWindow("pip");
#endif
	MY_FREE_MATRIX(limTL, width);
	MY_FREE_MATRIX(limTR, width);
	MY_FREE_MATRIX(limBL, width);
	MY_FREE_MATRIX(limBR, width);
	MY_FREE_MATRIX(limH, width);
	MY_FREE_MATRIX(limV, width);
	MY_FREE_MATRIX(bordes, width);
	return rect;
}

struct Proceso_PIP {
	CvRect zona;
};

static void tra_config_pip(const char *trCode, const char *trParameters, void **out_state) {
	struct Proceso_PIP *es = MY_MALLOC(1, struct Proceso_PIP);
	es->zona = cvRect(0, 0, 0, 0);
	*out_state = es;
}

#if PIP_VER_PASO1
int64_t pip_espera = 0;
#endif

static double **pip_calcularBordesPromedios(VideoFrame *video_frame) {
	int64_t x, y, cont = 0;
	CvSize frame_size = getFrameSize(video_frame);
	double **framesMu = MY_MALLOC_MATRIX(frame_size.width, frame_size.height,
			double);
	double **framesVar = MY_MALLOC_MATRIX(frame_size.width, frame_size.height,
			double);
	while (loadNextFrame(video_frame)) {
		IplImage *frameGris = getCurrentFrameGray(video_frame);
		cont++;
		for (y = 0; y < frame_size.height; ++y) {
			uchar *ptr = (uchar*) (frameGris->imageData
					+ frameGris->widthStep * y);
			for (x = 0; x < frame_size.width; ++x) {
				double delta = ptr[x] - framesMu[x][y];
				framesMu[x][y] += delta / cont;
				framesVar[x][y] += delta * (ptr[x] - framesMu[x][y]);
			}
		}
#if PIP_VER_PASO1
		cvShowImage("frameGris", frameGris);
		IplImage *im_mediasFrame = my_image_newFromArray(framesMu,
				frame_size.width, frame_size.height, 0);
		IplImage *im_varsFrame = my_image_newFromArray(framesVar,
				frame_size.width, frame_size.height, 0);
		cvShowImage("mediasFrame", im_mediasFrame);
		cvShowImage("varsFrame", im_varsFrame);
		char c = cvWaitKey(pip_espera);
		if (c == 27)
		pvcd_system_close_error();
		if (c == 32)
		pip_espera = pip_espera > 0 ? 0 : 1;
		my_image_release(im_mediasFrame);
		my_image_release(im_varsFrame);
#endif
	}
#if PIP_VER_PASO1
	cvDestroyWindow("frameGris");
	cvDestroyWindow("mediasFrame");
	cvDestroyWindow("varsFrame");
#endif
	for (y = 0; y < frame_size.height; ++y)
		for (x = 0; x < frame_size.width; ++x)
			framesVar[x][y] /= cont;
	IplImage *img_framesMu = my_image_newFromArray(framesMu, frame_size.width,
			frame_size.height, 0);
	IplImage *img_framesVar = my_image_newFromArray(framesVar, frame_size.width,
			frame_size.height, 0);
	MY_FREE_MATRIX(framesMu, frame_size.width);
	MY_FREE_MATRIX(framesVar, frame_size.width);
	IplImage *diffMu16b = cvCreateImage(frame_size, IPL_DEPTH_16S, 1);
	IplImage *diffVar16b = cvCreateImage(frame_size, IPL_DEPTH_16S, 1);
	cvLaplace(img_framesMu, diffMu16b, 3);
	cvLaplace(img_framesVar, diffVar16b, 3);
	IplImage *diffMu = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	IplImage *diffVar = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	cvConvertScaleAbs(diffMu16b, diffMu, 1, 0);
	cvConvertScaleAbs(diffVar16b, diffVar, 1, 0);
	double **ret = MY_MALLOC_MATRIX(frame_size.width, frame_size.height, double);
	for (y = 0; y < frame_size.height; ++y) {
		uchar *ptrMu = (uchar*) (diffMu->imageData + diffMu->widthStep * y);
		uchar *ptrVar = (uchar*) (diffVar->imageData + diffVar->widthStep * y);
		for (x = 0; x < frame_size.width; ++x) {
			ret[x][y] = ptrMu[x] * 0.5 + ptrVar[x] * 0.5;
		}
	}
#if PIP_VER_PASO2
	IplImage *img_ret = my_image_newFromArray(ret, frame_size.width,
			frame_size.height, 255);
	cvShowImage("img_framesMu", img_framesMu);
	cvShowImage("img_framesVar", img_framesVar);
	cvShowImage("diffMu", diffMu);
	cvShowImage("diffVar", diffVar);
	cvShowImage("img_ret", img_ret);
	char c = (char) cvWaitKey(0);
	if (c == 27)
		pvcd_system_exit_error();
	if (c == 's') {
		cvSaveImage(
				my_newString_format("%s_%s.png", getVideoName(video_frame),
						"img_framesMu"), img_framesMu, NULL);
		cvSaveImage(
				my_newString_format("%s_%s.png", getVideoName(video_frame),
						"img_framesVar"), img_framesVar, NULL);
		cvSaveImage(
				my_newString_format("%s_%s.png", getVideoName(video_frame), "diffMu"),
				diffMu, NULL);
		cvSaveImage(
				my_newString_format("%s_%s.png", getVideoName(video_frame), "diffVar"),
				diffVar, NULL);
		cvSaveImage(
				my_newString_format("%s_%s.png", getVideoName(video_frame), "img_ret"),
				img_ret, NULL);
	}
#endif
	my_image_release(img_framesMu);
	my_image_release(img_framesVar);
	my_image_release(diffMu16b);
	my_image_release(diffVar16b);
	my_image_release(diffMu);
	my_image_release(diffVar);
	return ret;
}

static uchar tra_preprocesar_compute_pip(VideoFrame *video_frame,
		FileDB *fileDB, void* estado, char **out_stateToSave) {
	CvSize frame_size = getFrameSize(video_frame);
	my_log_info("pip %s: promediando frames...\n", getVideoName(video_frame));
	double **arrayBordes = pip_calcularBordesPromedios(video_frame);
	my_log_info("pip %s: detectando rectangulo...\n", getVideoName(video_frame));
	struct Pip_Rectangulo rect = detectarRectanguloPIP(arrayBordes,
			frame_size.width, frame_size.height);
	if (rect.asignado) {
		my_log_info("pip %s: (%"PRIi64",%"PRIi64") w=%"PRIi64" h=%"PRIi64"\n",
				getVideoName(video_frame), rect.x, rect.y, rect.w, rect.h);
		*out_stateToSave = my_newString_format("%"PRIi64"-%"PRIi64"-%"PRIi64"-%"PRIi64"",
				rect.x, rect.y, rect.w, rect.h);
		return 1;
	} else {
		my_log_info("pip %s: sin pip\n", getVideoName(video_frame));
		return 0;
	}
}
static void tra_preprocesar_load_pip(void* estado, const char *savedState) {
	struct Proceso_PIP *es = estado;
	MyTokenizer *tk = my_tokenizer_new(savedState, '-');
	es->zona.x = my_tokenizer_nextInt(tk);
	es->zona.y = my_tokenizer_nextInt(tk);
	es->zona.width = my_tokenizer_nextInt(tk);
	es->zona.height = my_tokenizer_nextInt(tk);
	my_tokenizer_release(tk);
}
static IplImage *tra_transformar_pip(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_PIP *es = estado;
	cvSetImageROI(imagen, es->zona);
	return imagen;
}
static void tra_release_pip(void *estado) {
	struct Proceso_PIP *es = estado;
	MY_FREE(es);
}
void tra_reg_pip() {
	Transform_Def *def = newTransformDef("PIP", NULL);
	def->func_new = tra_config_pip;
	def->func_preprocess_compute = tra_preprocesar_compute_pip;
	def->func_preprocess_load = tra_preprocesar_load_pip;
	def->func_transform_frame = tra_transformar_pip;
	def->func_release = tra_release_pip;
}
#endif
