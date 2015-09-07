/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "pairMatcher.h"

struct PairMatcher_Basic {
	double ratio_2NN;
	double range_NN;
	MknnDistance *distance;
	//mknn parameters
	int64_t num_threads;
	MknnDataset *dataset_R;
	MknnIndex *index;
	MknnResolver *resolver;
	//output
	int64_t numVectorsQ, numMatches, current_maxNumVectorsQ;
	int64_t *vector_nn;
	struct MatchesAndModel *lastOutputMatches;
};

char *pairMatcherBasic_help() {
	return "(d1/d2)_fdist_[opt:_rangeDist]";
}
struct PairMatcher_Basic *pairMatcherBasic_new(const char *parameters) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	if (!my_tokenizer_hasNext(tk))
		my_log_error("\n%s must be format %s\n", parameters,
				pairMatcherBasic_help());
	struct PairMatcher_Basic *es = MY_MALLOC(1, struct PairMatcher_Basic);
	es->ratio_2NN = my_tokenizer_nextFraction(tk);
	char *string_distance = my_newString_string(my_tokenizer_nextToken(tk));
	es->range_NN = my_tokenizer_hasNext(tk) ? my_tokenizer_nextFraction(tk) : 0;
	my_tokenizer_releaseValidateEnd(tk);
	if (es->range_NN != 0) {
		char *st = my_newString_double(es->range_NN);
		my_log_info("using rangeNN=%s for distance %s\n", st, string_distance);
		free(st);
	}
	if (es->ratio_2NN < 1) {
		char *st = my_newString_double(es->ratio_2NN);
		my_log_info("using ratio 1NN/2NN=%s for distance %s\n", st,
				string_distance);
		free(st);
	}
	es->distance = mknn_distance_newPredefined(
			mknn_distanceParams_newParseString(string_distance), true);
	free(string_distance);
	es->num_threads = my_parallel_getNumberOfCores();
	return es;
}

MknnDataset *my_localDescriptors_newDatasetVectors(MyLocalDescriptors *ldes);

void pairMatcherBasic_unsetDescriptorR(struct PairMatcher_Basic *es) {
	if (es->dataset_R == NULL)
		return;
	mknn_resolver_release(es->resolver);
	mknn_index_release(es->index);
	es->dataset_R = NULL;
}
void pairMatcherBasic_setDescriptorR(struct PairMatcher_Basic *es,
		MyLocalDescriptors *descriptorR) {
	if (es->dataset_R != NULL)
		pairMatcherBasic_unsetDescriptorR(es);
	MknnDataset *dataset = my_localDescriptors_newDatasetVectors(descriptorR);
	if (mknn_dataset_getNumObjects(dataset) == 0) {
		mknn_dataset_release(dataset);
		return;
	}
	es->dataset_R = dataset;
	es->index = mknn_index_newPredefined(
			mknn_predefIndex_LinearScan_indexParams(), true, es->dataset_R,
			true, es->distance,
			false);
	es->resolver = mknn_index_newResolver(es->index,
			mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(2,
					es->range_NN, es->num_threads),
			true);
}

int64_t pairMatcherBasic_computeMatches(struct PairMatcher_Basic *es,
		MyLocalDescriptors *descriptorQ) {
	if (es->dataset_R == NULL)
		return 0;
	MknnDataset *dataset_Q = my_localDescriptors_newDatasetVectors(descriptorQ);
	if (mknn_dataset_getNumObjects(dataset_Q) == 0) {
		mknn_dataset_release(dataset_Q);
		return 0;
	}
	int64_t numVectorsQ = mknn_dataset_getNumObjects(dataset_Q);
	es->numVectorsQ = numVectorsQ;
	if (numVectorsQ > es->current_maxNumVectorsQ) {
		MY_REALLOC(es->vector_nn, numVectorsQ, int64_t);
		es->current_maxNumVectorsQ = numVectorsQ;
	}
	MknnResult *result = mknn_resolver_search(es->resolver, false, dataset_Q,
	true);
	es->numMatches = 0;
	for (int64_t i = 0; i < mknn_result_getNumQueries(result); ++i) {
		MknnResultQuery *r = mknn_result_getResultQuery(result, i);
		es->vector_nn[i] = -1;
		if (es->ratio_2NN >= 0 && es->ratio_2NN < 1 && r->num_nns > 1) {
			double dist1NN = r->nn_distance[0];
			double dist2NN = r->nn_distance[1];
			if (dist1NN != DBL_MAX && dist2NN != DBL_MAX
					&& dist1NN / dist2NN <= es->ratio_2NN) {
				es->vector_nn[i] = r->nn_position[0];
			}
		} else if (r->num_nns > 0) {
			es->vector_nn[i] = r->nn_position[0];
		}
		if (es->vector_nn[i] >= 0)
			es->numMatches++;
	}
	my_log_info("matches: %"PRIi64"\n", es->numMatches);
	mknn_result_release(result);
	//return (numVectorsQ - es->numMatches) / ((double) numVectorsQ);
	return es->numMatches;
}
void pairMatcherBasic_release(struct PairMatcher_Basic *es) {
	if (es->lastOutputMatches != NULL)
		MY_FREE_MULTI(es->lastOutputMatches->idVectorQ,
				es->lastOutputMatches->idVectorR, es->lastOutputMatches);
	mknn_distance_release(es->distance);
	MY_FREE_MULTI(es->vector_nn, es);
}

struct MatchesAndModel *pairMatcherBasic_getLastMatches(
		struct PairMatcher_Basic *es) {
	if (es->lastOutputMatches == NULL)
		es->lastOutputMatches = MY_MALLOC(1, struct MatchesAndModel);
	struct MatchesAndModel *outM = es->lastOutputMatches;
	outM->numMatches = es->numMatches;
	if (outM->numMatches == 0)
		return outM;
	MY_REALLOC(outM->idVectorQ, outM->numMatches, int64_t);
	MY_REALLOC(outM->idVectorR, outM->numMatches, int64_t);
	int64_t cont = 0;
	for (int64_t i = 0; i < es->numVectorsQ; ++i) {
		if (es->vector_nn[i] >= 0) {
			outM->idVectorQ[cont] = i;
			outM->idVectorR[cont] = es->vector_nn[i];
			cont++;
		}
	}
	my_assert_equalInt("numMatches", cont, es->numMatches);
	return outM;
}
