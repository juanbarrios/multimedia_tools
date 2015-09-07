/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "pairMatcher.h"

#ifndef NO_OPENCV
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc_c.h>

struct C_TransfModel {
	double scalex, scaley, tx, ty;
	double a, b, c, d, e, f, g, h, i;
	bool *isInlier;
	int64_t contInliers;
};
struct PairMatcher_Ransac {
	int64_t numCycles, modelType, minMatches, minSamplesModel;
	double minWidth, maxWidth, minHeight, maxHeight, minRatioWH, maxRatioWH;
	double distMaxInliers;
	bool useLScorrection;
	//eval
	struct MatchesAndModel *inputMatches;
	struct C_TransfModel *lastModel;
	struct MatchesAndModel *lastOutputMatches;
	CvMat *lastTransform;
	MyLocalDescriptors *descriptorQ, *descriptorR;
};

#define MODEL_TRANSLATION 1
#define MODEL_SCALE_SIFT 2
#define MODEL_SCALE_FREE 3
#define MODEL_AFFINE 4
#define MODEL_PERSPECTIVE 5

static struct C_TransfModel *loc_newModel(struct PairMatcher_Ransac *es) {
	struct C_TransfModel *m = MY_MALLOC(1, struct C_TransfModel);
	m->isInlier = MY_MALLOC(es->inputMatches->numMatches, bool);
	return m;
}
struct LMatch {
	struct MyLocalKeypoint src, des;
};
static struct LMatch loc_getMatchingPair(struct PairMatcher_Ransac *es,
		int64_t numPair) {
	int64_t numQ = es->inputMatches->idVectorQ[numPair];
	int64_t numR = es->inputMatches->idVectorR[numPair];
	struct LMatch lm;
	lm.src = my_localDescriptors_getKeypoint(es->descriptorQ, numQ);
	lm.des = my_localDescriptors_getKeypoint(es->descriptorR, numR);
	return lm;
}
static struct C_TransfModel *loc_fixModel1_trans(struct PairMatcher_Ransac *es,
		int64_t numInlier) {
	struct LMatch lm = loc_getMatchingPair(es, numInlier);
	struct C_TransfModel *m = loc_newModel(es);
	m->tx = lm.des.x - lm.src.x;
	m->ty = lm.des.y - lm.src.y;
	return m;
}
static struct C_TransfModel *loc_fixModel2_siftScale(
		struct PairMatcher_Ransac *es, int64_t numInlier) {
	struct LMatch lm = loc_getMatchingPair(es, numInlier);
	struct C_TransfModel *m = loc_newModel(es);
	if (lm.des.radius == 0 || lm.src.radius == 0)
		my_log_error("keypoints do not have scale\n");
	m->scalex = m->scaley = lm.des.radius / lm.src.radius;
	m->tx = lm.des.x - m->scalex * lm.src.x;
	m->ty = lm.des.y - m->scaley * lm.src.y;
	return m;
}
static struct C_TransfModel *loc_fixModel2_twoScales(
		struct PairMatcher_Ransac *es, int64_t numInlier1, int64_t numInlier2) {
	if (numInlier1 == numInlier2)
		return NULL;
	struct LMatch lm1 = loc_getMatchingPair(es, numInlier1);
	struct LMatch lm2 = loc_getMatchingPair(es, numInlier2);
	struct C_TransfModel *m = loc_newModel(es);
	m->scalex = (lm1.des.x - lm2.des.x) / (lm1.src.x - lm2.src.x);
	m->scaley = (lm1.des.y - lm2.des.y) / (lm1.src.y - lm2.src.y);
	m->tx = lm1.des.x - m->scalex * lm1.src.x;
	m->ty = lm1.des.y - m->scaley * lm1.src.y;
	return m;
}
static struct C_TransfModel *loc_fixModel3_affine(struct PairMatcher_Ransac *es,
		int64_t numInlier1, int64_t numInlier2, int64_t numInlier3) {
	if (numInlier1 == numInlier2 || numInlier1 == numInlier3
			|| numInlier2 == numInlier3)
		return NULL;
	struct LMatch lm1 = loc_getMatchingPair(es, numInlier1);
	struct LMatch lm2 = loc_getMatchingPair(es, numInlier2);
	struct LMatch lm3 = loc_getMatchingPair(es, numInlier3);
	CvPoint2D32f pts_src[3] = { cvPoint2D32f(lm1.src.x, lm1.src.y),
			cvPoint2D32f(lm2.src.x, lm2.src.y), cvPoint2D32f(lm3.src.x,
					lm3.src.y) };
	CvPoint2D32f pts_dst[3] = { cvPoint2D32f(lm1.des.x, lm1.des.y),
			cvPoint2D32f(lm2.des.x, lm2.des.y), cvPoint2D32f(lm3.des.x,
					lm3.des.y) };
	CvMat* mapMatrix = cvCreateMat(2, 3, CV_32FC1);
	cvGetAffineTransform(pts_src, pts_dst, mapMatrix);
	struct C_TransfModel *m = loc_newModel(es);
	m->a = cvGetReal2D(mapMatrix, 0, 0);
	m->b = cvGetReal2D(mapMatrix, 0, 1);
	m->c = cvGetReal2D(mapMatrix, 0, 2);
	m->d = cvGetReal2D(mapMatrix, 1, 0);
	m->e = cvGetReal2D(mapMatrix, 1, 1);
	m->f = cvGetReal2D(mapMatrix, 1, 2);
	cvReleaseMat(&mapMatrix);
	return m;
}
static struct C_TransfModel *loc_fixModel4_hom(struct PairMatcher_Ransac *es,
		int64_t numInlier1, int64_t numInlier2, int64_t numInlier3,
		int64_t numInlier4) {
	if (numInlier1 == numInlier2 || numInlier1 == numInlier3
			|| numInlier1 == numInlier4 || numInlier2 == numInlier3
			|| numInlier2 == numInlier4 || numInlier3 == numInlier4)
		return NULL;
	struct LMatch lm1 = loc_getMatchingPair(es, numInlier1);
	struct LMatch lm2 = loc_getMatchingPair(es, numInlier2);
	struct LMatch lm3 = loc_getMatchingPair(es, numInlier3);
	struct LMatch lm4 = loc_getMatchingPair(es, numInlier4);
	CvPoint2D32f pts_src[4] = { cvPoint2D32f(lm1.src.x, lm1.src.y),
			cvPoint2D32f(lm2.src.x, lm2.src.y), cvPoint2D32f(lm3.src.x,
					lm3.src.y), cvPoint2D32f(lm4.src.x, lm4.src.y) };
	CvPoint2D32f pts_dst[4] = { cvPoint2D32f(lm1.des.x, lm1.des.y),
			cvPoint2D32f(lm2.des.x, lm2.des.y), cvPoint2D32f(lm3.des.x,
					lm3.des.y), cvPoint2D32f(lm4.des.x, lm4.des.y) };
	CvMat* mapMatrix = cvCreateMat(3, 3, CV_32FC1);
	cvGetPerspectiveTransform(pts_src, pts_dst, mapMatrix);
	struct C_TransfModel *m = loc_newModel(es);
	m->a = cvGetReal2D(mapMatrix, 0, 0);
	m->b = cvGetReal2D(mapMatrix, 0, 1);
	m->c = cvGetReal2D(mapMatrix, 0, 2);
	m->d = cvGetReal2D(mapMatrix, 1, 0);
	m->e = cvGetReal2D(mapMatrix, 1, 1);
	m->f = cvGetReal2D(mapMatrix, 1, 2);
	m->g = cvGetReal2D(mapMatrix, 2, 0);
	m->h = cvGetReal2D(mapMatrix, 2, 1);
	m->i = cvGetReal2D(mapMatrix, 2, 2);
	cvReleaseMat(&mapMatrix);
	return m;
}
static int64_t minSamplesPerModel(int64_t modelType) {
	switch (modelType) {
	case MODEL_TRANSLATION:
	case MODEL_SCALE_SIFT:
		return 1;
	case MODEL_SCALE_FREE:
		return 2;
	case MODEL_AFFINE:
		return 3;
	case MODEL_PERSPECTIVE:
		return 4;
	}
	my_log_error("unknown transformation model %"PRIi64"\n", modelType);
	return 0;
}
static struct C_TransfModel *loc_fixModel(struct PairMatcher_Ransac *es,
		int64_t *randomSample) {
	switch (es->modelType) {
	case MODEL_TRANSLATION:
		return loc_fixModel1_trans(es, randomSample[0]);
	case MODEL_SCALE_SIFT:
		return loc_fixModel2_siftScale(es, randomSample[0]);
	case MODEL_SCALE_FREE:
		return loc_fixModel2_twoScales(es, randomSample[0], randomSample[1]);
	case MODEL_AFFINE:
		return loc_fixModel3_affine(es, randomSample[0], randomSample[1],
				randomSample[2]);
	case MODEL_PERSPECTIVE:
		return loc_fixModel4_hom(es, randomSample[0], randomSample[1],
				randomSample[2], randomSample[3]);
	}
	my_log_error("unknown transformation model %"PRIi64"\n", es->modelType);
	return NULL;
}
static double euclideanDist2D(double x1, double y1, double x2, double y2) {
	double dx = x2 - x1;
	double dy = y2 - y1;
	return sqrt(dx * dx + dy * dy);
}
static bool evaluateLocalMatch(struct C_TransfModel *m,
		struct PairMatcher_Ransac *es, int64_t numPair) {
	struct LMatch pair = loc_getMatchingPair(es, numPair);
	float srcx = pair.src.x, srcy = pair.src.y;
	double newX = 0, newY = 0;
	switch (es->modelType) {
	case MODEL_TRANSLATION:
		newX = srcx + m->tx;
		newY = srcy + m->ty;
		break;
	case MODEL_SCALE_SIFT:
	case MODEL_SCALE_FREE:
		newX = m->scalex * srcx + m->tx;
		newY = m->scaley * srcy + m->ty;
		break;
	case MODEL_AFFINE:
		newX = m->a * srcx + m->b * srcy + m->c;
		newY = m->d * srcx + m->e * srcy + m->f;
		break;
	case MODEL_PERSPECTIVE: {
		newX = m->a * srcx + m->b * srcy + m->c;
		newY = m->d * srcx + m->e * srcy + m->f;
		double w = m->g * srcx + m->h * srcy + m->i;
		newX /= w;
		newY /= w;
		break;
	}
	}
	double dist = euclideanDist2D(newX, newY, pair.des.x, pair.des.y);
	return (dist <= es->distMaxInliers) ? true : false;
}

#define STRICT_SCALE 1
#define actMinMax(val,varMin,varMax) varMin=MIN(varMin,val);varMax=MAX(varMax,val)

static void evaluateModel(struct C_TransfModel *m,
		struct PairMatcher_Ransac *es) {
	if (m == NULL)
		return;
	m->contInliers = 0;
	if (es->modelType == MODEL_SCALE_FREE) {
		if (!MY_BETWEEN(m->scalex, 0.2, 5) || !MY_BETWEEN(m->scaley, 0.2 ,5)
		|| !MY_BETWEEN( m->scalex / m->scaley, 0.333, 3))
			return;
		if (STRICT_SCALE) {
			if (!MY_BETWEEN(m->scalex, 0.95,
					1.05) || !MY_BETWEEN(m->scaley, 0.95,1.05)
					|| !MY_BETWEEN( m->scalex / m->scaley, 0.97, 1.03))
				return;
		}
	}
	for (int64_t pos = 0; pos < es->inputMatches->numMatches; ++pos) {
		m->isInlier[pos] = evaluateLocalMatch(m, es, pos);
		if (m->isInlier[pos]) {
			m->contInliers++;
		}
	}
	if (m->contInliers > 0
			&& (es->minWidth > 0 || es->maxWidth > 0 || es->minHeight > 0
					|| es->maxHeight > 0 || es->minRatioWH > 0
					|| es->maxRatioWH > 0)) {
		double minX = DBL_MAX, minY = DBL_MAX, maxX = 0, maxY = 0;
		for (int64_t pos = 0; pos < es->inputMatches->numMatches; ++pos) {
			if (m->isInlier[pos]) {
				struct LMatch pair = loc_getMatchingPair(es, pos);
				//actMinMax(pair.src.x, minX, maxX);
				//actMinMax(pair.src.y, minY, maxY);
				actMinMax(pair.des.x, minX, maxX);
				actMinMax(pair.des.y, minY, maxY);
			}
		}
		double w = maxX - minX;
		double h = maxY - minY;
		double r = w / h;
		if ((es->minWidth > 0 && w < es->minWidth)
				|| (es->maxWidth > 0 && w > es->maxWidth)
				|| (es->minHeight > 0 && h < es->minHeight)
				|| (es->maxHeight > 0 && h > es->maxHeight)
				|| (es->minRatioWH > 0 && r < es->minRatioWH)
				|| (es->maxRatioWH > 0 && r > es->maxRatioWH))
			m->contInliers = 0;
		if (es->minRatioWH > 0
				&& (es->modelType == MODEL_AFFINE
						|| es->modelType == MODEL_PERSPECTIVE)
				&& (m->a < 0 || m->b < 0 || m->d < 0 || m->e < 0 || m->g < 0
						|| m->h < 0))
			m->contInliers = 0;
	}
}
static void loc_promPos(struct PairMatcher_Ransac *es, bool *inlierPairs,
		double *p1x_prom, double *p1y_prom, double *p2x_prom, double *p2y_prom) {
	*p1x_prom = *p1y_prom = *p2x_prom = *p2y_prom = 0;
	int64_t pos, cont = 0;
	for (pos = 0; pos < es->inputMatches->numMatches; ++pos) {
		if (!inlierPairs[pos])
			continue;
		struct LMatch pair = loc_getMatchingPair(es, pos);
		cont++;
		*p1x_prom += (pair.src.x - *p1x_prom) / ((double) cont);
		*p1y_prom += (pair.src.y - *p1y_prom) / ((double) cont);
		*p2x_prom += (pair.des.x - *p2x_prom) / ((double) cont);
		*p2y_prom += (pair.des.y - *p2y_prom) / ((double) cont);
	}
}
static struct C_TransfModel* loc_correctModel2_siftScale(
		struct PairMatcher_Ransac *es, bool *inlierPairs) {
	double p1x_prom = 0, p1y_prom = 0, p2x_prom = 0, p2y_prom = 0;
	loc_promPos(es, inlierPairs, &p1x_prom, &p1y_prom, &p2x_prom, &p2y_prom);
//formula for single scale: sigma_x == sigma_y
	double sigNum = 0, sigDen = 0;
	int64_t pos;
	for (pos = 0; pos < es->inputMatches->numMatches; ++pos) {
		if (!inlierPairs[pos])
			continue;
		struct LMatch pair = loc_getMatchingPair(es, pos);
		sigNum += pair.src.x * (pair.des.x - p2x_prom)
				+ pair.src.y * (pair.des.y - p2y_prom);
		sigDen += pair.src.x * (pair.src.x - p1x_prom)
				+ pair.src.y * (pair.src.y - p1y_prom);
	}
	double sigma = sigNum / sigDen;
	double tx = p2x_prom - sigma * p1x_prom;
	double ty = p2y_prom - sigma * p1y_prom;
	if (MY_IS_REAL(sigma) && MY_IS_REAL(tx) && MY_IS_REAL(ty)) {
		struct C_TransfModel *m = loc_newModel(es);
		m->scalex = m->scaley = sigma;
		m->tx = tx;
		m->ty = ty;
		return m;
	}
	return NULL;
}
static struct C_TransfModel* loc_correctModel2_twoScales(
		struct PairMatcher_Ransac *es, bool *inlierPairs) {
	double p1x_prom = 0, p1y_prom = 0, p2x_prom = 0, p2y_prom = 0;
	loc_promPos(es, inlierPairs, &p1x_prom, &p1y_prom, &p2x_prom, &p2y_prom);
//formula for independent scales: sigma_x and sigma_y
	double sigNumX = 0, sigDenX = 0, sigNumY = 0, sigDenY = 0;
	int64_t pos;
	for (pos = 0; pos < es->inputMatches->numMatches; ++pos) {
		if (!inlierPairs[pos])
			continue;
		struct LMatch pair = loc_getMatchingPair(es, pos);
		sigNumX += pair.src.x * (pair.des.x - p2x_prom);
		sigNumY += pair.src.y * (pair.des.y - p2y_prom);
		sigDenX += pair.src.x * (pair.src.x - p1x_prom);
		sigDenY += pair.src.y * (pair.src.y - p1y_prom);
	}
	double sigmaX = sigNumX / sigDenX;
	double sigmaY = sigNumY / sigDenY;
	double tx = p2x_prom - sigmaX * p1x_prom;
	double ty = p2y_prom - sigmaY * p1y_prom;
	if (MY_IS_REAL(
			sigmaX) && MY_IS_REAL(sigmaY) && MY_IS_REAL(tx) && MY_IS_REAL(ty)) {
		struct C_TransfModel *m = loc_newModel(es);
		m->scalex = sigmaX;
		m->scaley = sigmaY;
		m->tx = tx;
		m->ty = ty;
		return m;
	}
	return NULL;
}
static struct C_TransfModel* loc_correctModel(struct PairMatcher_Ransac *es,
bool *inlierPairs) {
	switch (es->modelType) {
	case MODEL_SCALE_SIFT:
		return loc_correctModel2_siftScale(es, inlierPairs);
	case MODEL_SCALE_FREE:
		return loc_correctModel2_twoScales(es, inlierPairs);
	}
	my_log_error("this model does not support correction\n");
	return NULL;
}
static char* loc_printModel(int64_t modelType, struct C_TransfModel *m) {
	switch (modelType) {
	case MODEL_TRANSLATION:
		return my_newString_format("T=(%1.1lf %1.1lf)", m->tx, m->ty);
	case MODEL_SCALE_SIFT:
		return my_newString_format("S=%1.1lf T=(%1.1lf %1.1lf)", m->scalex,
				m->tx, m->ty);
	case MODEL_SCALE_FREE:
		return my_newString_format("S=(%1.1lf %1.1lf) T=(%1.1lf %1.1lf)",
				m->scalex, m->scaley, m->tx, m->ty);
	case MODEL_AFFINE:
		return my_newString_format(
				"A=[%5.2lf %5.2lf; %5.2lf %5.2lf] T=(%4.1lf %4.1lf)", m->a,
				m->b, m->d, m->e, m->c, m->f);
	case MODEL_PERSPECTIVE:
		return my_newString_format(
				"A=[%5.2lf %5.2lf %5.2lf] [%5.2lf %5.2lf %5.2lf] [%5.2lf %5.2lf %5.2lf]",
				m->a, m->b, m->c, m->d, m->e, m->f, m->g, m->h, m->i);
	}
	return NULL;
}
static void loc_releaseModel(struct C_TransfModel *m) {
	MY_FREE(m->isInlier);
	MY_FREE(m);
}

int64_t pairMatcherRansac_computeMatches(struct PairMatcher_Ransac *es,
		MyLocalDescriptors *descriptorQ, MyLocalDescriptors *descriptorR,
		struct MatchesAndModel *inputMatches) {
	if (es->lastModel != NULL) {
		loc_releaseModel(es->lastModel);
		es->lastModel = NULL;
	}
	int64_t numVectorsQ = my_localDescriptors_getNumDescriptors(descriptorQ);
	int64_t numVectorsR = my_localDescriptors_getNumDescriptors(descriptorR);
	es->descriptorQ = descriptorQ;
	es->descriptorR = descriptorR;
	if (numVectorsQ < es->minMatches || numVectorsR < es->minMatches)
		return 0;
	es->inputMatches = inputMatches;
	my_log_info("matches pre-ransac: %"PRIi64"\n",
			es->inputMatches->numMatches);
	if (es->inputMatches->numMatches < es->minMatches)
		return 0;
	struct C_TransfModel *bestModel = NULL;
	int64_t i = 0;
	for (i = 0; i < es->numCycles; ++i) {
		int64_t *randomSample = MY_MALLOC(es->minSamplesModel, int64_t);
		my_random_intList_noRepetitions(0, es->inputMatches->numMatches,
				randomSample, es->minSamplesModel);
		struct C_TransfModel *m = loc_fixModel(es, randomSample);
		evaluateModel(m, es);
		free(randomSample);
		if (es->useLScorrection && m != NULL && m->contInliers > 0) {
			for (;;) {
				struct C_TransfModel *m2 = loc_correctModel(es, m->isInlier);
				if (m2 == NULL)
					break;
				evaluateModel(m2, es);
				if (m2->contInliers > m->contInliers) {
					loc_releaseModel(m);
					m = m2;
				} else {
					loc_releaseModel(m2);
					break;
				}
			}
		}
		if (m != NULL && m->contInliers > 0
				&& (bestModel == NULL || m->contInliers > bestModel->contInliers)) {
			if (bestModel != NULL)
				loc_releaseModel(bestModel);
			bestModel = m;
		} else if (m != NULL) {
			loc_releaseModel(m);
		}
	}
	if (bestModel == NULL) {
		return 0;
	}
	char *st = loc_printModel(es->modelType, bestModel);
	my_log_info("MODEL type=%"PRIi64" %s\n", es->modelType, st);
	MY_FREE(st);
	my_log_info("matches post-ransac: %"PRIi64"\n", bestModel->contInliers);
	es->lastModel = bestModel;
	//double d = (numVectorsQ - bestModel->contInliers) / ((double) numVectorsQ);
	return bestModel->contInliers;
}
char *pairMatcherRansac_help() {
	return "(modelType:1..5)_numCycles_distMaxInliers_[opt:MINMATCHES_num]_[opt:CORRECTION]_[opt:MINWIDTH_w]_[opt:MINHEIGHT_h]_[opt:MAXWIDTH_w]_[opt:MAXHEIGHT_h]_[opt:MINRATIOWH_p]_[opt:MAXRATIOWH_p]  Ej: RANSAC,0.8_L2,2_300_2.5";
}
struct PairMatcher_Ransac* pairMatcherRansac_new(const char *parameters) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	if (!my_tokenizer_hasNext(tk))
		my_log_error("\n%s\n", pairMatcherRansac_help());
	struct PairMatcher_Ransac *es = MY_MALLOC(1, struct PairMatcher_Ransac);
	es->modelType = my_tokenizer_nextInt(tk);
	es->numCycles = my_tokenizer_nextInt(tk);
	es->distMaxInliers = my_tokenizer_nextDouble(tk);
	while (my_tokenizer_hasNext(tk)) {
		if (my_tokenizer_isNext(tk, "MINMATCHES"))
			es->minMatches = my_tokenizer_nextInt(tk);
		else if (my_tokenizer_isNext(tk, "CORRECTION"))
			es->useLScorrection = 1;
		else if (my_tokenizer_isNext(tk, "MINWIDTH"))
			es->minWidth = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "MAXWIDTH"))
			es->maxWidth = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "MINHEIGHT"))
			es->minHeight = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "MAXHEIGHT"))
			es->maxHeight = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "MINRATIOWH"))
			es->minRatioWH = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "MAXRATIOWH"))
			es->maxRatioWH = my_tokenizer_nextFraction(tk);
		else
			break;
	}
	my_tokenizer_releaseValidateEnd(tk);
	es->minSamplesModel = minSamplesPerModel(es->modelType);
	if (es->minMatches < es->minSamplesModel)
		es->minMatches = es->minSamplesModel;
	if (true) {
		MyStringBuffer *sb = my_stringbuf_new();
		my_stringbuf_appendString(sb, "numCycles=");
		my_stringbuf_appendInt(sb, es->numCycles);
		my_stringbuf_appendString(sb, " modelType=");
		my_stringbuf_appendInt(sb, es->modelType);
		my_stringbuf_appendString(sb, " maxDistInliers=");
		my_stringbuf_appendDouble(sb, es->distMaxInliers);
		if (es->minMatches > 0) {
			my_stringbuf_appendString(sb, " minMatches=");
			my_stringbuf_appendInt(sb, es->minMatches);
		}
		if (es->useLScorrection)
			my_stringbuf_appendString(sb, " withCorrection");
		if (es->minWidth > 0) {
			my_stringbuf_appendString(sb, " minWidth=");
			my_stringbuf_appendDouble(sb, es->minWidth);
		}
		if (es->maxWidth > 0) {
			my_stringbuf_appendString(sb, " maxWidth=");
			my_stringbuf_appendDouble(sb, es->maxWidth);
		}
		if (es->minHeight > 0) {
			my_stringbuf_appendString(sb, " minHeight=");
			my_stringbuf_appendDouble(sb, es->minHeight);
		}
		if (es->maxHeight > 0) {
			my_stringbuf_appendString(sb, " maxHeight=");
			my_stringbuf_appendDouble(sb, es->maxHeight);
		}
		if (es->minRatioWH > 0) {
			my_stringbuf_appendString(sb, " minRatioWH=");
			my_stringbuf_appendDouble(sb, es->minRatioWH);
		}
		if (es->maxRatioWH > 0) {
			my_stringbuf_appendString(sb, " maxRatioWH=");
			my_stringbuf_appendDouble(sb, es->maxRatioWH);
		}
		my_log_info("RANSAC with %s\n", my_stringbuf_getCurrentBuffer(sb));
		my_stringbuf_release(sb);
	}
	return es;
}
void pairMatcherRansac_release(struct PairMatcher_Ransac* es) {
	if (es->lastModel != NULL)
		loc_releaseModel(es->lastModel);
	if (es->lastOutputMatches != NULL) {
		MY_FREE_MULTI(es->lastOutputMatches->idVectorQ,
				es->lastOutputMatches->idVectorR, es->lastOutputMatches);
	}
	if (es->lastTransform != NULL) {
		cvReleaseMat(&es->lastTransform);
	}
	free(es);
}
/**********************/
#if 0
struct State_FindHom {
	DescriptorType desType;
	int64_t numCiclos, minMatches, minSamplesModel;
	double minWidth, maxWidth, minHeight, maxHeight, minRatioWH, maxRatioWH;
	double distMaxInliers;
	bool useLScorrection;
	Distance *distMatcher;
	//eval
	struct MatchesAndModel *inputMatches;
	struct C_TransfModel *lastModel;
	struct MatchesAndModel *lastOutputMatches;
	void *descriptorQ, *descriptorR;
};

char *local_help_findhom() {
	return "FINDHOMOGRAPHY,numCycles_distMaxInliers_[opt:MINMATCHES_num]_[opt:CORRECTION]_[opt:MINWIDTH_w]_[opt:MINHEIGHT_h]_[opt:MAXWIDTH_w]_[opt:MAXHEIGHT_h]_[opt:MINRATIOWH_p]_[opt:MAXRATIOWH_p]_[DistanceNoSpatial]  Ej: RANSAC,2_300_2.5_LOWE,0.9_L2";
}
static double dist_findhom(void *descriptor1, void *descriptor2,
		DescriptorType desType, void *state_dist, double currentRange) {
	struct State_FindHom *es = state_dist;
	if (es->lastModel != NULL ) {
		loc_releaseModel(es->lastModel);
		es->lastModel = NULL;
	}
	int64_t numVectorsQ = local_getNumVectores(descriptor1);
	int64_t numVectorsR = local_getNumVectores(descriptor2);
	es->descriptorQ = descriptor1;
	es->descriptorR = descriptor2;
	if (numVectorsQ < es->minMatches || numVectorsR < es->minMatches)
	return 1;
	DIST(es->distMatcher, descriptor1, descriptor2);
	es->inputMatches = retrieveLastMatchesAndModel(es->distMatcher);
	if (CON_LOG_LOCALES)
	my_log_info("matches pre-ransac: %"PRIi64"\n",
			es->inputMatches->numMatches);
	if (es->inputMatches->numMatches < es->minMatches) {
		if (CON_LOG_LOCALES)
		my_log_info("too few matches\n");
		return 1;
	}
	cvFindHomography();
	struct LMatch lm1 = loc_getMatchingPair(es, numInlier1);
	struct LMatch lm2 = loc_getMatchingPair(es, numInlier2);
	struct LMatch lm3 = loc_getMatchingPair(es, numInlier3);
	struct LMatch lm4 = loc_getMatchingPair(es, numInlier4);
	CvPoint2D32f pts_src[4] = {cvPoint2D32f(lm1.src.x, lm1.src.y),
		cvPoint2D32f(lm2.src.x, lm2.src.y), cvPoint2D32f(lm3.src.x,
				lm3.src.y), cvPoint2D32f(lm4.src.x, lm4.src.y)};
	CvPoint2D32f pts_dst[4] = {cvPoint2D32f(lm1.des.x, lm1.des.y),
		cvPoint2D32f(lm2.des.x, lm2.des.y), cvPoint2D32f(lm3.des.x,
				lm3.des.y), cvPoint2D32f(lm4.des.x, lm4.des.y)};
	CvMat* mapMatrix = cvCreateMat(3, 3, CV_32FC1);
	cvGetPerspectiveTransform(pts_src, pts_dst, mapMatrix);
	int64_t numCiclos = es->numCiclos;
}
static void* dist_findhom_config(char *code, char *parameters,
		struct SearchCollection *colQuery, struct SearchCollection *colReference,
		char *path_profile, DescriptorType desType, int64_t idDescriptor) {
	MyTokenizer *tk = my_tokenizer_new3(parameters, '_', 1);
	if (!my_tokenizer_hasNext(tk))
	my_log_error("\n%s\n", local_help_findhom());
	struct State_FindHom *es = MY_MALLOC(1, struct State_FindHom);
	es->numCiclos = my_tokenizer_nextInt(tk);
	es->distMaxInliers = my_tokenizer_nextDouble(tk);
	es->distMatcher = findDistanceDescCol(desType, my_tokenizer_getCurrentTail(tk),
			colQuery, colReference, path_profile, idDescriptor);
	my_tokenizer_release(tk);
	es->desType = desType;
	if (es->minMatches < es->minSamplesModel)
	es->minMatches = es->minSamplesModel;
	return es;
}
static void dist_findhom_release(void *state_dist) {
	struct State_Ransac* es = state_dist;
	if (es->lastModel != NULL )
	loc_releaseModel(es->lastModel);
	if (es->lastOutputMatches != NULL ) {
		cvReleaseMat(&es->lastOutputMatches->transform);
		MY_FREE_MULTI(es->lastOutputMatches->idVectorQ,
				es->lastOutputMatches->idVectorR, es->lastOutputMatches);
	}
	releaseDistance(es->distMatcher);
	MY_FREE(es);
}
reg_distance_desc(dtype, "FINDHOMOGRAPHY", dist_findhom_config, NULL, dist_findhom,
		dist_findhom_release);
#endif
/*********************************/
struct MatchesAndModel *pairMatcherRansac_getLastModel(
		struct PairMatcher_Ransac* es) {
	struct C_TransfModel *m = es->lastModel;
	if (es->lastOutputMatches == NULL)
		es->lastOutputMatches = MY_MALLOC(1, struct MatchesAndModel);
	struct MatchesAndModel *outM = es->lastOutputMatches;
	if (m == NULL || m->contInliers == 0) {
		outM->numMatches = 0;
		return outM;
	}
	outM->numMatches = m->contInliers;
	MY_REALLOC(outM->idVectorQ, outM->numMatches, int64_t);
	MY_REALLOC(outM->idVectorR, outM->numMatches, int64_t);
	int64_t pos, cont = 0;
	for (pos = 0; pos < es->inputMatches->numMatches; ++pos) {
		if (m->isInlier[pos]) {
			outM->idVectorQ[cont] = es->inputMatches->idVectorQ[pos];
			outM->idVectorR[cont] = es->inputMatches->idVectorR[pos];
			cont++;
		}
	}
	my_assert_equalInt("numMatches", cont, outM->numMatches);
	if (es->lastTransform != NULL) {
		cvReleaseMat(&es->lastTransform);
		es->lastTransform = NULL;
	}
	if (es->modelType == MODEL_AFFINE) {
		CvMat* mat = cvCreateMat(2, 3, CV_32FC1);
		cvSetReal2D(mat, 0, 0, m->a);
		cvSetReal2D(mat, 0, 1, m->b);
		cvSetReal2D(mat, 0, 2, m->c);
		cvSetReal2D(mat, 1, 0, m->d);
		cvSetReal2D(mat, 1, 1, m->e);
		cvSetReal2D(mat, 1, 2, m->f);
		es->lastTransform = mat;
	} else if (es->modelType == MODEL_PERSPECTIVE) {
		CvMat* mat = cvCreateMat(3, 3, CV_32FC1);
		cvSetReal2D(mat, 0, 0, m->a);
		cvSetReal2D(mat, 0, 1, m->b);
		cvSetReal2D(mat, 0, 2, m->c);
		cvSetReal2D(mat, 1, 0, m->d);
		cvSetReal2D(mat, 1, 1, m->e);
		cvSetReal2D(mat, 1, 2, m->f);
		cvSetReal2D(mat, 2, 0, m->g);
		cvSetReal2D(mat, 2, 1, m->h);
		cvSetReal2D(mat, 2, 2, m->i);
		es->lastTransform = mat;
	}
	return outM;
}
CvMat *pairMatcherRansac_getLastTransformation(struct PairMatcher_Ransac* es) {
	return es->lastTransform;
}

#endif
