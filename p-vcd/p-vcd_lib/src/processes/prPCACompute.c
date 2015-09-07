/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

#ifndef NO_OPENCV
struct State_ComputePca {
	char *filenameState;
	double sampleFractionSegments, sampleFractionPerFrame;
};
static void computePca_new(const char *segCode, const char *segParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	struct State_ComputePca *es = MY_MALLOC(1, struct State_ComputePca);
	es->filenameState = my_tokenizer_nextToken_newString(tk);
	es->sampleFractionSegments = my_tokenizer_nextDouble0(tk);
	es->sampleFractionPerFrame = my_tokenizer_nextDouble0(tk);
	my_tokenizer_releaseValidateEnd(tk);
	my_assert_notNull("filenameState", es->filenameState);
	*out_state = es;
}

static void computePca_process(LoadDescriptors *desloader, void *state) {
	struct State_ComputePca *es = state;
	MknnPcaAlgorithm *pca = mknn_pca_new();
	DB *db = loadDescriptors_getDb(desloader);
	for (int64_t i = 0; i < db->numFilesDb; ++i) {
		struct DescriptorsFile *df = loadDescriptorsFileDB(desloader,
				db->filesDb[i]);
		MknnDataset *dataset = loadDescriptorsFile_getMknnDataset(df,
				es->sampleFractionSegments, es->sampleFractionPerFrame);
		mknn_pca_addDatasetToVectorStats(pca, dataset);
		mknn_dataset_release(dataset);
		releaseLoadDescriptors_allFileBytes(desloader);
	}
	mknn_pca_computeTransformationMatrix(pca);
	mknn_pca_save(pca, es->filenameState, true);
	mknn_pca_release(pca);
}

static void computePca_release(void *state) {
	struct State_ComputePca *es = state;
	MY_FREE_MULTI(es->filenameState, es);
}
#endif

void proc_reg_computePca() {
#ifndef NO_OPENCV
	addProcessorDefDescriptor("PCA-COMPUTE",
			"filenameStatePca_(sampleFractionSegments)_(sampleFractionPerFrame)",
			computePca_new, computePca_process, computePca_release);
#endif
}
