/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

#if 0
struct State_Kmeans {
	char *numCentroids;
	char *distanceCode;
	char *filenameState;
	char *nameExtractor;
	int64_t maxIteration;
	double maxSecondsProcess;
	int64_t minMovedVectors;
	int64_t minMovedCentroids;
	double sampleFractionGlobal, sampleFractionFiles, sampleFractionDescriptors,
			sampleFractionPerFrame;
};
static void kmeans_new(const char *segCode, const char *segParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	struct State_Kmeans *es = MY_MALLOC(1, struct State_Kmeans);
	es->numCentroids = my_tokenizer_nextToken_newString(tk);
	es->distanceCode = my_tokenizer_nextToken_newString(tk);
	es->filenameState = my_tokenizer_nextToken_newString(tk);
	if (my_string_equals("KMEANS_EXTRACT", segCode))
		es->nameExtractor = my_tokenizer_nextToken_newString(tk);
	while (my_tokenizer_hasNext(tk)) {
		if (my_tokenizer_isNext(tk, "maxIteration"))
			es->maxIteration = my_tokenizer_nextInt(tk);
		else if (my_tokenizer_isNext(tk, "maxSeconds"))
			es->maxSecondsProcess = my_tokenizer_nextDouble(tk);
		else if (my_tokenizer_isNext(tk, "minVectorsMoved"))
			es->minMovedVectors = my_tokenizer_nextInt(tk);
		else if (my_tokenizer_isNext(tk, "minMovedCentroids"))
			es->minMovedCentroids = my_tokenizer_nextInt(tk);
		else if (my_tokenizer_isNext(tk, "sampleGlobal"))
			es->sampleFractionGlobal = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "sampleFile"))
			es->sampleFractionFiles = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "sampleDescriptors"))
			es->sampleFractionDescriptors = my_tokenizer_nextFraction(tk);
		else if (my_tokenizer_isNext(tk, "samplePerFrame"))
			es->sampleFractionPerFrame = my_tokenizer_nextFraction(tk);
	}
	my_tokenizer_releaseValidateEnd(tk);
	my_assert_notNull("numCentroids", es->numCentroids);
	my_assert_notNull("distanceCode", es->distanceCode);
	my_assert_notNull("filenameState", es->filenameState);
	*out_state = es;
}
static void perform_kmeans(MknnDataset* dataset, struct State_Kmeans *es) {
	MknnKmeansAlgorithm *kmeans = mknn_kmeans_new();
	mknn_kmeans_setDataset(kmeans, dataset);
	mknn_kmeans_setTermitationCriteria(kmeans, es->maxIteration,
			es->maxSecondsProcess, es->minMovedVectors, es->minMovedCentroids);
	mknn_kmeans_setAutoSaveState(kmeans, es->filenameState, 180);
	if (my_io_existsFile(es->filenameState)) {
		mknn_kmeans_loadState(kmeans, es->filenameState);
	} else {
		my_log_info("Distance=%s\n", es->distanceCode);
		int64_t nc = my_math_getFractionSize(my_parse_double(es->numCentroids),
				mknn_dataset_getNumObjects(dataset));
		my_log_info("NumCentroids=%"PRIi64"\n", nc);
		MknnDistance *distance = mknn_distance_newPredefined(
				mknn_distanceParams_newParseString(es->distanceCode),
				true);
		mknn_kmeans_setDistance(kmeans, distance);
		mknn_kmeans_setNumCentroids(kmeans, nc);
		mknn_kmeans_addDefaultSubsetRuns(kmeans);
	}
	mknn_kmeans_perform(kmeans);
	mknn_kmeans_release(kmeans);
}

static void kmeans_process(LoadDescriptors *desloader,
		void *state) {
	struct State_Kmeans *es = state;
	MknnDataset *dataset = getAlreadyComputedDescriptorsSample(
			desloader, es->sampleFractionGlobal, es->sampleFractionFiles,
			es->sampleFractionDescriptors, es->sampleFractionPerFrame);
	perform_kmeans(dataset, es);
	mknn_dataset_release(dataset);
	return true;
}
static void kmeansExtract_process(
		LoadSegmentation *segloader, void *state) {
	struct State_Kmeans *es = state;
	MknnDataset *dataset = computeDescriptorsSample_nameExtractor(
			segloader, es->nameExtractor, NUM_CORES, es->sampleFractionGlobal,
			es->sampleFractionFiles, es->sampleFractionDescriptors,
			es->sampleFractionPerFrame);
	perform_kmeans(dataset, es);
	mknn_dataset_release(dataset);
	return true;
}

static void kmeans_release(void *state) {
	struct State_Kmeans *es = state;
	MY_FREE_MULTI(es->numCentroids, es->distanceCode, es->filenameState,
			es->nameExtractor, es);
}

void proc_reg_kmeans() {
	addProcessorDefDescriptor("KMEANS",
			"numCentroids_distance_filenameState_(maxIteration=[n]_maxSeconds=[n]minVectorsMoved=[n]minMovedCentroids=[n]sampleGlobal=[n]sampleFile=[n]sampleDescriptors=[n]samplePerFrame=[n])",
			kmeans_new, kmeans_process, kmeans_release);
	addProcessorDefSegmentation("KMEANS_EXTRACT",
			"numCentroids_distance_filenameState_frameDescriptor_(maxIteration=[n]_maxSeconds=[n]minVectorsMoved=[n]minMovedCentroids=[n]sampleGlobal=[n]sampleFile=[n]sampleDescriptors=[n]samplePerFrame=[n])",
			kmeans_new, kmeansExtract_process, kmeans_release);
}
#endif
