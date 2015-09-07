/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV

#define VER_PASO1 0
#define VER_PASO2 0

struct Estado_ACC {
	uchar forzarDeteccion;
	double esqTLx, esqTLy, esqTRx, esqTRy, esqBLx, esqBLy, esqBRx, esqBRy,
			centroPrevX, centroPrevY;
	IplImage *imgGris, *imgBinaria, *conversionPre, *conversionPost,
			*transformadoPrev, *transformadoPrevGris, *transformadoPost;
};

static void tra_config_acc(const char *trCode, const char *trParameters, void **out_state) {
	struct Estado_ACC *es = MY_MALLOC(1, struct Estado_ACC);
	if (trParameters != NULL && my_string_equals(trParameters, "FORCED"))
		es->forzarDeteccion = 1;
	*out_state = es;
}

struct Transf_Hough {
	int64_t pasosTheta, pasosRho;
	double **votos, *thetas;
	//double minTheta, maxTheta;
	double minRho, maxRho, largoRho;
};
struct Recta {
	uchar esValida;
	double votos, theta, rho;
};
struct Poligono {
	struct Recta arriba, abajo, izq, der;
};

static struct Transf_Hough* nueva_transf_hough(int64_t pasosTheta, int64_t pasosRho) {
	struct Transf_Hough* hough = MY_MALLOC(1, struct Transf_Hough);
	hough->pasosTheta = pasosTheta;
	hough->pasosRho = pasosRho;
	hough->votos = MY_MALLOC_MATRIX(pasosTheta, pasosRho, double);
	hough->thetas = MY_MALLOC_NOINIT(pasosTheta, double);
	return hough;
}
static void release_transf_hough(struct Transf_Hough* hough) {
	MY_FREE_MATRIX(hough->votos, hough->pasosTheta);
	MY_FREE(hough->thetas);
	MY_FREE(hough);
}
static void hough_init(struct Transf_Hough* hough, double minTheta,
		double maxTheta, double minRho, double maxRho) {
	int64_t i, j;
	for (i = 0; i < hough->pasosTheta; ++i)
		for (j = 0; j < hough->pasosRho; ++j)
			hough->votos[i][j] = 0;
	//hough->minTheta = minTheta;
	//hough->maxTheta = maxTheta;
	hough->minRho = minRho;
	hough->maxRho = maxRho;
	hough->largoRho = maxRho - minRho;
	//no incluyo los limites de theta
	double largoBin = (maxTheta - minTheta) / (hough->pasosTheta + 1);
	for (i = 0; i < hough->pasosTheta; ++i)
		hough->thetas[i] = minTheta + (i + 1) * largoBin;
}

static void hough_votar(struct Transf_Hough* hough, double x, double y,
		double valor_voto) {
	int64_t i, celda_abajo;
	double rho, celda, ponderacion_arriba, ponderacion_abajo;
	for (i = 0; i < hough->pasosTheta; i++) {
		//segun el theta calculo rho
		rho = x * cos(hough->thetas[i]) + y * sin(hough->thetas[i]);
		if (rho < hough->minRho || rho > hough->maxRho)
			continue;
		// veo cual es la celda que le corresponde
		celda = (hough->pasosRho - 1) * (rho - hough->minRho) / hough->largoRho;
		//ponderador de votos
		celda_abajo = (int64_t) celda;
		ponderacion_arriba = celda - celda_abajo;
		ponderacion_abajo = 1 - ponderacion_arriba;
		if (MY_BETWEEN(celda_abajo, 0, hough->pasosRho - 1))
			hough->votos[i][celda_abajo] += valor_voto * ponderacion_abajo;
		celda_abajo++;
		if (MY_BETWEEN(celda_abajo, 0, hough->pasosRho - 1))
			hough->votos[i][celda_abajo] += valor_voto * ponderacion_arriba;
	}
}

#if VER_PASO1 || VER_PASO2
static void dibujar_punto(IplImage *imagen, double x, double y, CvScalar color) {
	//log_info("punto %f,%f\n", punto.x, punto.y);
	int64_t a = round_int(x), b = round_int(y);
	cvCircle(imagen, cvPoint(a, b), 3, color, CV_FILLED, 8, 0);
}
#endif
#if VER_PASO1
static void dibujar_recta(IplImage *imagen, struct Recta recta, double centroX,
		double centroY, CvScalar color) {
	double rx = centroX + recta.rho * cos(recta.theta);
	double ry = centroY + recta.rho * sin(recta.theta);
	CvPoint a, b;
	double m = tan(M_PI_2 + recta.theta);
	if (isnan(m) || m > imagen->height || -m > imagen->height) {
		a.x = b.x = rx;
		a.y = 0;
		b.y = imagen->height;
	} else {
		double n = ry - rx * m;
		a.x = 0;
		b.x = imagen->width;
		a.y = round_int(a.x * m + n);
		b.y = round_int(b.x * m + n);
	}
	dibujar_punto(imagen, centroX, centroY, color);
	dibujar_punto(imagen, rx, ry, color);
	my_log_info(
			"%s votos=%3.0lf t=%4.0lf r=%3.0lf (rx=%3.0lf,ry=%3.0lf,m=%4.1lf) (%i,%i)-(%i,%i)\n",
			recta.esValida ? "RECTA" : "inval", recta.votos,
			MY_RAD2GRAD(recta.theta), recta.rho, rx, ry, m, a.x, a.y, b.x, b.y);
	cvLine(imagen, a, b, color, 2, 8, 0);
}
IplImage *img_votos = NULL;
IplImage *img_votantes = NULL;
int64_t acc_milis_espera = 0;
#endif

static struct Recta hough_buscar_recta(struct Transf_Hough* hough,
		IplImage *imgBorde, int64_t desdex, int64_t desdey, int64_t hastax,
		int64_t hastay, double centroX, double centroY) {
#if VER_PASO1
	my_image_release(img_votantes);
	img_votantes = cvCreateImage(cvSize(imgBorde->width, imgBorde->height),
			imgBorde->depth, imgBorde->nChannels);
	cvSetZero(img_votantes);
	double totales = 0;
#endif
	int64_t x, y;
	uchar *ptr, val;
	for (y = desdey; y < hastay; ++y) {
		ptr = (uchar*) (imgBorde->imageData + imgBorde->widthStep * y);
		for (x = desdex; x < hastax; ++x) {
			val = ptr[x];
			if (val > 50) {
				//el origen esta al centro de la imagen
				hough_votar(hough, x - centroX, y - centroY, 1);
#if VER_PASO1
				PIXC_8U(img_votantes,x,y,0) = 150;
				totales += hough->pasosTheta;
#endif
			}
#if VER_PASO1
			else if (val > 0)
			PIXC_8U(img_votantes,x,y,0) = 80;
#endif
		}
	}
	double max_val = 0;
	int64_t i, j, max_i = -1, max_j = -1;
	for (i = 0; i < hough->pasosTheta; ++i)
		for (j = 0; j < hough->pasosRho; ++j)
			if (hough->votos[i][j] > max_val) {
				max_val = hough->votos[i][j];
				max_i = i;
				max_j = j;
			}
	struct Recta recta;
	recta.esValida = (max_val > 10) ? 1 : 0; //30
	recta.votos = max_val;
	if (recta.esValida || max_i >= 0) {
		recta.theta = hough->thetas[max_i];
		recta.rho = max_j * hough->largoRho / (hough->pasosRho - 1)
				+ hough->minRho;
	} else {
		recta.theta = recta.rho = 0;
	}
#if VER_PASO1
	my_image_release(img_votos);
	img_votos = my_image_newFromArray(hough->votos, hough->pasosTheta,
			hough->pasosRho, 0);
	dibujar_punto(img_votos, max_i, max_j, cvScalarAll(80));
	cvShowImage("img_votos", img_votos);
	CvScalar col = cvScalarAll(recta.esValida ? 255 : 80);
	dibujar_recta(img_votantes, recta, centroX, centroY, col);
	cvShowImage("img_votantes", img_votantes);
	char c = cvWaitKey(acc_milis_espera);
	if (c == 27)
	pvcd_system_exit_error();
	if (c == 32)
	acc_milis_espera = acc_milis_espera > 0 ? 0 : 1;
#endif
	return recta;
}

static struct Poligono hough_buscar_poligono(struct Transf_Hough* hough,
		IplImage *imgBorde, CvPoint desde, CvPoint hasta, double centroX,
		double centroY) {
	struct Poligono poli;
	int64_t medx = (int64_t) centroX, medy = (int64_t) centroY;
	//arriba
	hough_init(hough, -M_PI_2, -M_PI_4, medy * 0.7, medy * M_SQRT2);
	poli.arriba = hough_buscar_recta(hough, imgBorde, desde.x, desde.y, hasta.x,
			medy, centroX, centroY);
	//abajo
	hough_init(hough, M_PI_4, M_PI_2, medy * 0.5, medy * M_SQRT2);
	poli.abajo = hough_buscar_recta(hough, imgBorde, desde.x, medy, hasta.x,
			hasta.y, centroX, centroY);
	//izquierda
	hough_init(hough, -M_PI, -3 * M_PI_4, medx * 0.9, medx * M_SQRT2);
	poli.izq = hough_buscar_recta(hough, imgBorde, desde.x, desde.y, medx,
			hasta.y, centroX, centroY);
	//derecha
	hough_init(hough, -M_PI_4, 0, medx * 0.5, medx * M_SQRT2);
	poli.der = hough_buscar_recta(hough, imgBorde, medx, desde.y, hasta.x,
			hasta.y, centroX, centroY);
	//log_info("votos: %1.0lf %1.0lf %1.0lf %1.0lf\n", poli.arriba.votos,	poli.abajo.votos, poli.izq.votos, poli.der.votos);
	return poli;
}

static void determinar_zona_no_negra(IplImage *imgGris, CvPoint *desde,
		CvPoint *hasta, uchar minNegro) {
	desde->x = imgGris->width;
	desde->y = imgGris->height;
	hasta->x = hasta->y = 0;
	int64_t x, y;
	uchar *ptr;
	for (y = 0; y < imgGris->height; ++y) {
		ptr = (uchar*) (imgGris->imageData + imgGris->widthStep * y);
		for (x = 0; x < imgGris->width; ++x) {
			if (ptr[x] >= minNegro) {
				if (x < desde->x)
					desde->x = x;
				if (y < desde->y)
					desde->y = y;
				if (x > hasta->x)
					hasta->x = x;
				if (y > hasta->y)
					hasta->y = y;
			}
		}
	}
	if (0) {
		//CONVEX HULL
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2,
				sizeof(CvContour), sizeof(CvPoint), storage);
		CvPoint pt0;
		for (y = 3; y < imgGris->height - 3; ++y) {
			uchar *ptr1 = (uchar*) (imgGris->imageData + imgGris->widthStep * y);
			for (x = 3; x < imgGris->width - 3; ++x) {
				if (ptr1[x] >= 2) {
					pt0.x = x;
					pt0.y = y;
					cvSeqPush(ptseq, &pt0);
				}
			}
		}
		cvSetZero(imgGris);
		CvSeq* hull = cvConvexHull2(ptseq, 0, CV_CLOCKWISE, 0);
		int64_t hullcount = hull->total;
		pt0 = **CV_GET_SEQ_ELEM(CvPoint*, hull, hullcount - 1);
		int64_t i;
		for (i = 0; i < hullcount; i++) {
			CvPoint pt = **CV_GET_SEQ_ELEM(CvPoint*, hull, i);
			cvLine(imgGris, pt0, pt, cvScalarAll(255), 2, 8, 0);
			pt0 = pt;
		}
		cvClearMemStorage(storage);
	}
}
static void binarizarImagen(IplImage *imgGris, IplImage *imgBinaria) {
	cvThreshold(imgGris, imgBinaria, 3, 255, CV_THRESH_BINARY);
}
#define PIXC_8U(img, x, y, nc) (((uchar*) ((img)->imageData + (img)->widthStep * (y)))[img->nChannels*(x)+(nc)])

static void determinarBordesEnImagenBinaria(IplImage *imgBinaria,
		IplImage *imgBorde) {
	cvSetZero(imgBorde);
	int64_t MAX_CONT = 2;
	int64_t x, y;
	for (y = 0; y < imgBorde->height; ++y) {
		uchar *ptr =
				(uchar*) (imgBinaria->imageData + imgBinaria->widthStep * y);
		uchar *ptrB = (uchar*) (imgBorde->imageData + imgBorde->widthStep * y);
		int64_t cont = 0;
		for (x = 0; x < imgBorde->width / 2; ++x) {
			if (ptr[x] == 0)
				continue;
			if (cont++ >= MAX_CONT)
				break;
			if (x > MAX_CONT)
				ptrB[x] = 255;
		}
		cont = 0;
		for (x = imgBorde->width - 1; x > imgBorde->width / 2; --x) {
			if (ptr[x] == 0)
				continue;
			if (cont++ >= MAX_CONT)
				break;
			if (x < imgBorde->width - MAX_CONT)
				ptrB[x] = 255;
		}
	}
	for (x = 0; x < imgBorde->width; ++x) {
		int64_t cont = 0;
		for (y = 0; y < imgBorde->height / 2; ++y) {
			if (PIXC_8U(imgBinaria, x, y, 0) == 0)
				continue;
			if (cont++ >= MAX_CONT)
				break;
			if (y > MAX_CONT)
				PIXC_8U(imgBorde, x, y, 0) = 255;
		}
		cont = 0;
		for (y = imgBorde->height - 1; y > imgBorde->height / 2; --y) {
			if (PIXC_8U(imgBinaria, x, y, 0) == 0)
				continue;
			if (cont++ >= MAX_CONT)
				break;
			if (y < imgBorde->height - MAX_CONT)
				PIXC_8U(imgBorde, x, y, 0) = 255;
		}
	}
}
struct Promedio {
	double mean, M2; //varianza=M2/n;
	int64_t n;
};
static void promediar(struct Promedio *p, double value) {
	p->n++;
	double delta = value - p->mean;
	p->mean += delta / p->n;
	p->M2 += delta * (value - p->mean);
}
struct RectaPromedio {
	struct Promedio prom_rho, prom_theta;
};
struct PuntoPromedio {
	struct Promedio prom_x, prom_y;
};
struct PoligonoPromedio {
	struct RectaPromedio r_arr, r_aba, r_izq, r_der;
	struct PuntoPromedio esq_arr_izq, esq_arr_der, esq_aba_izq, esq_aba_der;
};
static void tra_prom_recta(struct Recta recta, struct RectaPromedio *rprom) {
	if (recta.esValida) {
		promediar(&rprom->prom_rho, recta.rho);
		promediar(&rprom->prom_theta, recta.theta);
	}
}
static void intersectar_rectas(struct Recta r1, struct Recta r2, double centroX,
		double centroY, double *out_x, double *out_y) {
	double x = (r2.rho * sin(r1.theta) - r1.rho * sin(r2.theta))
			/ (cos(r2.theta) * sin(r1.theta) - cos(r1.theta) * sin(r2.theta));
	double y = (r1.rho - x * cos(r1.theta)) / sin(r1.theta);
	x += centroX;
	y += centroY;
	*out_x = x;
	*out_y = y;
}
static void tra_prom_intersec(struct Recta recta1, struct Recta recta2,
		double centroX, double centroY, struct PuntoPromedio *pprom) {
	if (recta1.esValida && recta2.esValida) {
		double x = 0, y = 0;
		intersectar_rectas(recta1, recta2, centroX, centroY, &x, &y);
		double relativeX = x - centroX;
		double relativeY = y - centroY;
		promediar(&pprom->prom_x, relativeX);
		promediar(&pprom->prom_y, relativeY);
	}
}
static void tra_acc_promediar(struct Poligono poli, double centroX,
		double centroY, struct PoligonoPromedio *proms) {
	int64_t i, cont = 1;
	/*if (poli.arriba.esValida && poli.abajo.esValida && poli.izq.esValida
	 && poli.der.esValida)
	 cont = 3;*/
	for (i = 0; i < cont; ++i) {
		tra_prom_recta(poli.arriba, &proms->r_arr);
		tra_prom_recta(poli.abajo, &proms->r_aba);
		tra_prom_recta(poli.izq, &proms->r_izq);
		tra_prom_recta(poli.der, &proms->r_der);
		tra_prom_intersec(poli.arriba, poli.izq, centroX, centroY,
				&proms->esq_arr_izq);
		tra_prom_intersec(poli.arriba, poli.der, centroX, centroY,
				&proms->esq_arr_der);
		tra_prom_intersec(poli.abajo, poli.izq, centroX, centroY,
				&proms->esq_aba_izq);
		tra_prom_intersec(poli.abajo, poli.der, centroX, centroY,
				&proms->esq_aba_der);
	}
}
static void centroMasa(IplImage *imgBinaria, uchar minNegro,
		double *out_centroX, double *out_centroY) {
	double centrox = 0, centroy = 0;
	int64_t x, y, w = imgBinaria->width, h = imgBinaria->height;
	int64_t contPix = 0, minX = w, minY = h, maxX = 0, maxY = 0;
	uchar *ptr;
	for (y = 0; y < h; ++y) {
		ptr = (uchar*) (imgBinaria->imageData + imgBinaria->widthStep * y);
		for (x = 0; x < w; ++x) {
			if (ptr[x] >= minNegro) {
				contPix++;
				centrox += (x - centrox) / contPix;
				centroy += (y - centroy) / contPix;
				if (x < minX)
					minX = x;
				if (x > maxX)
					maxX = x;
				if (y < minY)
					minY = y;
				if (y > maxY)
					maxY = y;
			}
		}
	}
	double medx = (maxX + minX) / 2.0;
	double medy = (maxY + minY) / 2.0;
	*out_centroX = centrox * 0.5 + medx * 0.5;
	*out_centroY = centroy * 0.5 + medy * 0.5;
}
static uchar tra_preprocesar_compute_acc(VideoFrame *video_frame,
		FileDB *fileDB, void* estado, char **out_stateToSave) {
	struct Estado_ACC *es = estado;
	CvPoint desde, hasta;
	IplImage *imgBinaria = NULL, *imgBorde = NULL;
	struct PoligonoPromedio *poli_prom = MY_MALLOC(1,
			struct PoligonoPromedio);
	int64_t numframes = 0;
	struct Transf_Hough* hough = nueva_transf_hough(150, 200); //45, 60
	while (loadNextFrame(video_frame)) {
		IplImage *frameGris = getCurrentFrameGray(video_frame);
		if (imgBinaria == NULL) {
			imgBinaria = cvCreateImage(cvGetSize(frameGris), IPL_DEPTH_8U, 1);
			imgBorde = cvCreateImage(cvGetSize(frameGris), IPL_DEPTH_8U, 1);
		}
		binarizarImagen(frameGris, imgBinaria);
		determinarBordesEnImagenBinaria(imgBinaria, imgBorde);
#if VER_PASO1
		cvShowImage("frameGris", frameGris);
		cvShowImage("imgBinaria", imgBinaria);
		cvShowImage("imgBorde", imgBorde);
#endif
		determinar_zona_no_negra(imgBinaria, &desde, &hasta, 1);
		double centroX = 0, centroY = 0;
		centroMasa(imgBinaria, 1, &centroX, &centroY);
		struct Poligono poli = hough_buscar_poligono(hough, imgBorde, desde,
				hasta, centroX, centroY);
		tra_acc_promediar(poli, centroX, centroY, poli_prom);
		numframes++;
	}
	release_transf_hough(hough);
	my_image_release(imgBinaria);
	my_image_release(imgBorde);
	if (!es->forzarDeteccion) {
		MY_FREE(poli_prom);
		my_log_info("acc, no se detecta camcording en %s\n",
				getVideoName(video_frame));
		return 0;
	}
	//arriba,abajo,izq,der
	char *est = my_newString_format(
			"%1.2lf,%1.2lf,%1.2lf,%1.2lf,%1.2lf,%1.2lf,%1.2lf,%1.2lf",
			poli_prom->esq_arr_izq.prom_x.mean,
			poli_prom->esq_arr_izq.prom_y.mean,
			poli_prom->esq_arr_der.prom_x.mean,
			poli_prom->esq_arr_der.prom_y.mean,
			poli_prom->esq_aba_izq.prom_x.mean,
			poli_prom->esq_aba_izq.prom_y.mean,
			poli_prom->esq_aba_der.prom_x.mean,
			poli_prom->esq_aba_der.prom_y.mean);
	MY_FREE(poli_prom);
	my_log_info("preprocessing %s: %s\n", getVideoName(video_frame), est);
	my_log_info(
			"lines in %s: up=%lf(%5.1lf) down=%lf(%5.1lf) left=%lf(%5.1lf) right=%lf(%5.1lf)\n",
			getVideoName(video_frame), poli_prom->r_arr.prom_rho.mean,
			MY_RAD2GRAD(poli_prom->r_arr.prom_theta.mean),
			poli_prom->r_aba.prom_rho.mean,
			MY_RAD2GRAD(poli_prom->r_aba.prom_theta.mean),
			poli_prom->r_izq.prom_rho.mean,
			MY_RAD2GRAD(poli_prom->r_izq.prom_theta.mean),
			poli_prom->r_der.prom_rho.mean,
			MY_RAD2GRAD(poli_prom->r_der.prom_theta.mean));
	*out_stateToSave = est;
	return 1;
}
static void tra_preprocesar_load_acc(void* estado, const char *savedState) {
#if VER_PASO2
	my_log_info("preproceso: %s\n", savedState);
#endif
	struct Estado_ACC *es = estado;
	MyTokenizer *tk = my_tokenizer_new(savedState, ',');
	es->esqTLx = my_tokenizer_nextDouble(tk);
	es->esqTLy = my_tokenizer_nextDouble(tk);
	es->esqTRx = my_tokenizer_nextDouble(tk);
	es->esqTRy = my_tokenizer_nextDouble(tk);
	es->esqBLx = my_tokenizer_nextDouble(tk);
	es->esqBLy = my_tokenizer_nextDouble(tk);
	es->esqBRx = my_tokenizer_nextDouble(tk);
	es->esqBRy = my_tokenizer_nextDouble(tk);
	my_tokenizer_releaseValidateEnd(tk);
}
#define ACC_MARGEN_W 100
#define ACC_MARGEN_H 100

static IplImage *tra_transformar_acc(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Estado_ACC *es = estado;
	if (es->imgGris == NULL) {
		CvSize frame_size = cvGetSize(imagen);
		es->imgGris = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
		es->imgBinaria = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
		es->conversionPre = cvCreateImage(frame_size, IPL_DEPTH_32F, 3);
		CvSize size_con_margen = cvSize(frame_size.width + ACC_MARGEN_W * 2,
				frame_size.height + ACC_MARGEN_H * 2);
		es->conversionPost = cvCreateImage(size_con_margen, IPL_DEPTH_32F, 3);
		es->transformadoPrev = cvCreateImage(size_con_margen, IPL_DEPTH_8U, 3);
		es->transformadoPrevGris = cvCreateImage(size_con_margen, IPL_DEPTH_8U,
				1);
		es->transformadoPost = cvCreateImage(frame_size, IPL_DEPTH_8U, 3);
	}
	IplImage *entrada = imagen;
	if (imagen->nChannels > 1) {
		cvCvtColor(imagen, es->imgGris, CV_BGR2GRAY);
		entrada = es->imgGris;
	}
	binarizarImagen(entrada, es->imgBinaria);
	double centroX = 0, centroY = 0;
	centroMasa(es->imgBinaria, 1, &centroX, &centroY);
	if (es->centroPrevX > 0) {
		centroX = centroX * 0.9 + es->centroPrevX * 0.1;
		centroY = centroY * 0.9 + es->centroPrevY * 0.1;
	}
	es->centroPrevX = centroX;
	es->centroPrevY = centroY;
	CvPoint2D32f tl, tr, bl, br;
	tl = cvPoint2D32f(es->esqTLx + centroX, es->esqTLy + centroY);
	tr = cvPoint2D32f(es->esqTRx + centroX, es->esqTRy + centroY);
	bl = cvPoint2D32f(es->esqBLx + centroX, es->esqBLy + centroY);
	br = cvPoint2D32f(es->esqBRx + centroX, es->esqBRy + centroY);
	//mapeo las intersecciones con los bordes de la imagen (dejo un margen)
	CvPoint2D32f tl2 = cvPoint2D32f(ACC_MARGEN_W, ACC_MARGEN_H);
	CvPoint2D32f tr2 = cvPoint2D32f(es->conversionPost->width - ACC_MARGEN_W,
	ACC_MARGEN_H);
	CvPoint2D32f br2 = cvPoint2D32f(es->conversionPost->width - ACC_MARGEN_W,
			es->conversionPost->height - ACC_MARGEN_H);
	CvPoint2D32f bl2 = cvPoint2D32f(ACC_MARGEN_W,
			es->conversionPost->height - ACC_MARGEN_H);
	//creo la transformacion que mapea punto a punto
	CvMat* mmat = cvCreateMat(3, 3, CV_32FC1);
	CvPoint2D32f puntos_src[4] = { tl, tr, br, bl };
	CvPoint2D32f puntos_dst[4] = { tl2, tr2, br2, bl2 };
	mmat = cvGetPerspectiveTransform(puntos_src, puntos_dst, mmat);
	//paso la imagen a 32bit float para poder transformarla
	cvConvertScale(imagen, es->conversionPre, 1 / 255.0, 0);
	cvWarpPerspective(es->conversionPre, es->conversionPost, mmat,
			CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
	//vuelvo la imagen a 8-bit color
	cvConvertScale(es->conversionPost, es->transformadoPrev, 255.0, 0);
	//busco si se puede recortar
	cvCvtColor(es->transformadoPrev, es->transformadoPrevGris, CV_BGR2GRAY);
#if VER_PASO2
	CvScalar colorRect = cvScalar(100, 0, 200, 0);
	CvScalar colorPunto = cvScalar(50, 200, 50, 0);
	cvLine(imagen, cvPoint(tl.x, tl.y), cvPoint(tr.x, tr.y), colorRect, 3, 8,
			0);
	cvLine(imagen, cvPoint(tl.x, tl.y), cvPoint(bl.x, bl.y), colorRect, 3, 8,
			0);
	cvLine(imagen, cvPoint(tr.x, tr.y), cvPoint(br.x, br.y), colorRect, 3, 8,
			0);
	cvLine(imagen, cvPoint(bl.x, bl.y), cvPoint(br.x, br.y), colorRect, 3, 8,
			0);
	dibujar_punto(imagen, centroX, centroY, colorPunto);
	dibujar_punto(imagen, tl.x, tl.y, colorPunto);
	dibujar_punto(imagen, tl.x, tl.y, colorPunto);
	dibujar_punto(imagen, tr.x, tr.y, colorPunto);
	dibujar_punto(imagen, br.x, br.y, colorPunto);
	dibujar_punto(imagen, bl.x, bl.y, colorPunto);
	dibujar_punto(es->conversionPost, tl2.x, tl2.y, colorPunto);
	dibujar_punto(es->conversionPost, tr2.x, tr2.y, colorPunto);
	dibujar_punto(es->conversionPost, br2.x, br2.y, colorPunto);
	dibujar_punto(es->conversionPost, bl2.x, bl2.y, colorPunto);
	cvShowImage("imagenCC", imagen);
	cvShowImage("conversionPost", es->conversionPost);
	my_log_info(
			"centro=%3.0f,%3.0f tl=%3.0f,%3.0f tr=%3.0f,%3.0f bl=%3.0f,%3.0f br=%3.0f,%3.0f\n",
			centroX, centroY, tl.x, tl.y, tr.x, tr.y, bl.x, bl.y, br.x, br.y);
#endif
	CvPoint desde, hasta;
	determinar_zona_no_negra(es->transformadoPrevGris, &desde, &hasta, 15);
	desde.x = MIN(desde.x, tl2.x + ACC_MARGEN_W/2);
	desde.y = MIN(desde.y, tl2.y + ACC_MARGEN_H/2);
	hasta.x = MAX(hasta.x, br2.x - ACC_MARGEN_W/2);
	hasta.y = MAX(hasta.y, br2.y - ACC_MARGEN_H/2);
	CvRect clip;
	clip = cvRect(desde.x, desde.y, hasta.x - desde.x, hasta.y - desde.y);
	cvSetImageROI(es->transformadoPrev, clip);
	cvResize(es->transformadoPrev, es->transformadoPost, CV_INTER_LINEAR);
	cvResetImageROI(es->transformadoPrev);
	cvReleaseMat(&mmat);
	return es->transformadoPost;
}
static void tra_release_acc(void *estado) {
	struct Estado_ACC *es = estado;
	my_image_release(es->imgGris);
	my_image_release(es->imgBinaria);
	my_image_release(es->conversionPre);
	my_image_release(es->conversionPost);
	my_image_release(es->transformadoPrev);
	my_image_release(es->transformadoPrevGris);
	my_image_release(es->transformadoPost);
	MY_FREE(es);
}

void tra_reg_acc() {
	Transform_Def *def = newTransformDef("ACC", "(FORCED)");
	def->func_new = tra_config_acc;
	def->func_preprocess_compute = tra_preprocesar_compute_acc;
	def->func_preprocess_load = tra_preprocesar_load_acc;
	def->func_transform_frame = tra_transformar_acc;
	def->func_release = tra_release_acc;
}
#endif
