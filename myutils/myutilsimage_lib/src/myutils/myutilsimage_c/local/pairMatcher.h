/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef PAIRMATCHER_H
#define PAIRMATCHER_H

#include "local.h"

struct MatchesAndModel {
	int64_t numMatches;
	int64_t *idVectorQ, *idVectorR;
};

struct PairMatcher;

char *pairMatcher_help();

struct PairMatcher *newPairMatcher(const char *codeAndParameters);
int64_t computePairMatches(struct PairMatcher *pairMatcher,
		MyLocalDescriptors *descriptorQ, MyLocalDescriptors *descriptorR);
void releasePairMatcher(struct PairMatcher *pairMatcher);

struct MatchesAndModel *getLastMatchesAndModel(struct PairMatcher *pairMatcher);

#ifndef NO_OPENCV
CvMat *getLastTransform(struct PairMatcher *pairMatcher);
#endif

#endif
