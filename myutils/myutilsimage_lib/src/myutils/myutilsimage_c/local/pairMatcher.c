/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "pairMatcher.h"

struct PairMatcher_Basic;

char *pairMatcherBasic_help();

struct PairMatcher_Basic *pairMatcherBasic_new(const char *parameters);
void pairMatcherBasic_setDescriptorR(struct PairMatcher_Basic *es,
		MyLocalDescriptors *descriptorR);
int64_t pairMatcherBasic_computeMatches(struct PairMatcher_Basic *es,
		MyLocalDescriptors *descriptorQ);
void pairMatcherBasic_unsetDescriptorR(struct PairMatcher_Basic *es);
void pairMatcherBasic_release(struct PairMatcher_Basic *es);

struct MatchesAndModel *pairMatcherBasic_getLastMatches(
		struct PairMatcher_Basic *es);

struct PairMatcher_Ransac;

char *pairMatcherRansac_help();

struct PairMatcher_Ransac* pairMatcherRansac_new(const char *parameters);
int64_t pairMatcherRansac_computeMatches(struct PairMatcher_Ransac *es,
		MyLocalDescriptors *descriptorQ, MyLocalDescriptors *descriptorR,
		struct MatchesAndModel *inputMatches);
void pairMatcherRansac_release(struct PairMatcher_Ransac* es);

struct MatchesAndModel *pairMatcherRansac_getLastModel(
		struct PairMatcher_Ransac* es);
#ifndef NO_OPENCV
CvMat *pairMatcherRansac_getLastTransformation(struct PairMatcher_Ransac* es);
#endif

struct PairMatcher {
	struct PairMatcher_Basic *simpleMatch;
	struct PairMatcher_Ransac *ransacMatch;
};

char *pairMatcher_help() {
#ifndef NO_OPENCV
	return my_newString_format(
			"MATCH,[matchHelp] or RANSAC,[matchHelp],[ransacHelp]\n   matchHelp=%s\nransacHelp=%s",
			pairMatcherBasic_help(), pairMatcherRansac_help());
#else
	return my_newString_format("MATCH,%s\n", pairMatcherBasic_help());
#endif
}

struct PairMatcher *newPairMatcher(const char *codeAndParameters) {
	struct PairMatcher *es = MY_MALLOC(1, struct PairMatcher);
	if (my_string_startsWith(codeAndParameters, "MATCH,")) {
		es->simpleMatch = pairMatcherBasic_new(codeAndParameters + 6);
#ifndef NO_OPENCV
	} else if (my_string_startsWith(codeAndParameters, "RANSAC,")) {
		codeAndParameters += 7;
		char *p1 = my_subStringC_startFirst(codeAndParameters, ',');
		char *p2 = my_subStringC_firstEnd(codeAndParameters, ',');
		es->simpleMatch = pairMatcherBasic_new(p1);
		es->ransacMatch = pairMatcherRansac_new(p2);
		MY_FREE_MULTI(p1, p2);
#endif
	} else {
		my_log_error("error %s\n", codeAndParameters);
	}
	return es;
}
int64_t computePairMatches(struct PairMatcher *pairMatcher,
		MyLocalDescriptors *descriptorQ, MyLocalDescriptors *descriptorR) {
	pairMatcherBasic_setDescriptorR(pairMatcher->simpleMatch, descriptorR);
	int64_t n = pairMatcherBasic_computeMatches(pairMatcher->simpleMatch,
			descriptorQ);
	pairMatcherBasic_unsetDescriptorR(pairMatcher->simpleMatch);
	if (n == 0)
		return 0;
#ifndef NO_OPENCV
	if (pairMatcher->ransacMatch != NULL) {
		struct MatchesAndModel *matches = pairMatcherBasic_getLastMatches(
				pairMatcher->simpleMatch);
		n = pairMatcherRansac_computeMatches(pairMatcher->ransacMatch,
				descriptorQ, descriptorR, matches);
	}
#endif
	return n;
}

void releasePairMatcher(struct PairMatcher *pairMatcher) {
	pairMatcherBasic_release(pairMatcher->simpleMatch);
	if (pairMatcher->ransacMatch != NULL) {
#ifndef NO_OPENCV
		pairMatcherRansac_release(pairMatcher->ransacMatch);
#endif
	}
}

struct MatchesAndModel *getLastMatchesAndModel(struct PairMatcher *pairMatcher) {
#ifndef NO_OPENCV
	if (pairMatcher->ransacMatch != NULL) {
		return pairMatcherRansac_getLastModel(pairMatcher->ransacMatch);
	}
#endif
	return pairMatcherBasic_getLastMatches(pairMatcher->simpleMatch);
}

#ifndef NO_OPENCV
CvMat *getLastTransform(struct PairMatcher *pairMatcher) {
	if (pairMatcher->ransacMatch != NULL) {
		return pairMatcherRansac_getLastTransformation(pairMatcher->ransacMatch);
	}
	return NULL;
}
#endif
