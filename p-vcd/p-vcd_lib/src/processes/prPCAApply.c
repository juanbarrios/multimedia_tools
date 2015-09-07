/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

#ifndef NO_OPENCV

struct State_ApplyPca {
	char *method_name;
	int64_t outDimensions;
	bool withTxt;
	MknnDatatype outDatatype;
	MyDatatype outMyDatatype;
	MknnPcaAlgorithm *pca;
	void **descriptors_input;
	void **descriptors_output;
};

static void applyPca_new(const char *segCode, const char *segParameters,
		void **out_state) {
	struct State_ApplyPca *es = MY_MALLOC(1, struct State_ApplyPca);
	es->method_name = my_newString_format("%s_%s", segCode, segParameters);
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	char *filenameState = my_tokenizer_nextToken_newString(tk);
	es->outDimensions = my_tokenizer_nextInt(tk);
	if (!mknn_datatype_parseString(my_tokenizer_nextToken(tk),
			&es->outDatatype))
		my_log_error("unknown datatype\n");
	es->withTxt = my_tokenizer_isNext(tk, "withTxt");
	my_tokenizer_releaseValidateEnd(tk);
	my_assert_notNull("filenameState", filenameState);
	es->outMyDatatype = mknn_datatype_convertMknn2My(es->outDatatype);
	es->pca = mknn_pca_new();
	mknn_pca_restore(es->pca, filenameState);
	*out_state = es;
}
static void applyPca_file(int64_t current_process, void *state_object,
		int64_t current_thread) {
	struct State_ApplyPca *es = state_object;
	MyLocalDescriptors *ldes_input = es->descriptors_input[current_process];
	MyLocalDescriptors *ldes_output = my_localDescriptors_new(0,
			es->outMyDatatype, es->outDimensions);
	applyPcaToLocalDescriptors(es->pca, ldes_input, ldes_output);
	es->descriptors_output[current_process] = ldes_output;
}

static void applyPca_process(LoadDescriptors *desloader, void *state) {
	struct State_ApplyPca *es = state;
	DescriptorType td_in = loadDescriptors_getDescriptorType(desloader);
	DescriptorType td_out;
	if (td_in.dtype == DTYPE_LOCAL_VECTORS) {
		td_out = descriptorType(DTYPE_LOCAL_VECTORS, 0, 0, 0);
	} else if (td_in.dtype == DTYPE_ARRAY_DOUBLE
			|| td_in.dtype == DTYPE_ARRAY_FLOAT
			|| td_in.dtype == DTYPE_ARRAY_UCHAR) {
		if (mknn_datatype_isDouble(es->outDatatype))
			td_out = descriptorType(DTYPE_ARRAY_DOUBLE, es->outDimensions, 0,
					0);
		else if (mknn_datatype_isFloat(es->outDatatype))
			td_out = descriptorType(DTYPE_ARRAY_FLOAT, es->outDimensions, 0, 0);
		else if (mknn_datatype_isUInt8(es->outDatatype))
			td_out = descriptorType(DTYPE_ARRAY_UCHAR, es->outDimensions, 0, 0);
		else
			my_log_error("unsupported datatype %s\n",
					mknn_datatype_toString(es->outDatatype));
	} else {
		my_log_error("unsupported input type\n");
	}
	char *descriptor = my_newString_format("%s-%s", es->method_name,
			loadDescriptors_getDescriptor(desloader));
	char *descAlias = my_newString_string(descriptor);
	my_io_removeInvalidChars(descAlias);
	DB *db = loadDescriptors_getDb(desloader);
	char *output_dir = my_newString_format("%s/%s", db->pathDescriptors,
			descAlias);
	SaveDescriptors *dessaver = newSaveDescriptors(output_dir, td_out, true,
			es->withTxt, loadDescriptors_getIsSingleFile(desloader), descriptor,
			loadDescriptors_getSegmentation(desloader));
	MY_FREE_MULTI(descriptor, descAlias, output_dir);
	MyProgress *lt = my_progress_new(es->method_name, db->numFilesDb, 1);
	for (int64_t i = 0; i < db->numFilesDb; ++i) {
		FileDB *fdb = db->filesDb[i];
		struct Segmentation *seg = NULL;
		struct DescriptorsFile* df = loadDescriptorsFileDB(desloader, fdb);
		es->descriptors_input = df->descriptors;
		es->descriptors_output = MY_MALLOC(df->numDescriptors, void*);
		if (td_in.dtype == DTYPE_LOCAL_VECTORS) {
			my_parallel_incremental(df->numDescriptors, es, applyPca_file, NULL,
					0);
		}
		saveDescriptors(dessaver, fdb->id, df->numDescriptors,
				es->descriptors_output, seg);
		if (td_in.dtype == DTYPE_LOCAL_VECTORS) {
			for (int64_t i = 0; i < df->numDescriptors; ++i) {
				MyLocalDescriptors *ldes = es->descriptors_output[i];
				my_localDescriptors_release(ldes);
			}
		}
		MY_FREE(es->descriptors_output);
		releaseDescriptorsFile(df);
		my_progress_add1(lt);
	}
	releaseSaveDescriptors(dessaver);
	my_progress_release(lt);
}

static void applyPca_release(void *state) {
	struct State_ApplyPca *es = state;
	mknn_pca_release(es->pca);
	MY_FREE_MULTI(es->method_name, es);
}
#endif

void proc_reg_applyPca() {
#ifndef NO_OPENCV
	addProcessorDefDescriptor("PCA-APPLY",
			"filenameStatePca_outDimensions_outDatatype_(withTxt)",
			applyPca_new, applyPca_process, applyPca_release);
#endif
}
