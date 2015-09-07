/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

struct State_RootSift {
	char *method_name;
	bool withTxt;
	void **descriptors_input;
	void **descriptors_output;
};

static void rootSift_new(const char *segCode, const char *segParameters,
		void **out_state) {
	struct State_RootSift *es = MY_MALLOC(1, struct State_RootSift);
	if (segParameters == NULL) {
		es->method_name = my_newString_string(segCode);
	} else {
		es->method_name = my_newString_format("%s_%s", segCode, segParameters);
		MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
		es->withTxt = my_tokenizer_isNext(tk, "withTxt");
		my_tokenizer_releaseValidateEnd(tk);
	}
	*out_state = es;
}
static void applyRootSift_file(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread) {
	struct State_RootSift *es = state_object;
	MyDatatype dIn = MY_DATATYPE_UINT8;
	MyDatatype dOut = MY_DATATYPE_FLOAT32;
	my_function_copyOperate_vector funcCopy =
			my_datatype_getFunctionCopyOperateVector(dIn, dOut);
	for (int64_t i = start_process; i < end_process_notIncluded; ++i) {
		MyLocalDescriptors *ldes_input = es->descriptors_input[i];
		int64_t nd = my_localDescriptors_getVectorDimensions(ldes_input);
		int64_t numv = my_localDescriptors_getNumDescriptors(ldes_input);
		MyLocalDescriptors *ldes_output = my_localDescriptors_new(numv, dOut,
				nd);
		es->descriptors_output[i] = ldes_output;
		if (numv == 0)
			continue;
		if (!my_datatype_areEqual(dIn,
				my_localDescriptors_getVectorDatatype(ldes_input)))
			my_log_error("invalid type %i\n",
					my_localDescriptors_getVectorDatatype(ldes_input).my_datatype_code);
		for (int64_t i = 0; i < numv; ++i) {
			struct MyLocalKeypoint kp = my_localDescriptors_getKeypoint(
					ldes_input, i);
			my_localDescriptors_setKeypointSt(ldes_output, i, kp);
		}
		for (int64_t i = 0; i < numv; ++i) {
			void *vector1 = my_localDescriptors_getVector(ldes_input, i);
			void *vector2 = my_localDescriptors_getVector(ldes_output, i);
			funcCopy(vector1, vector2, nd, sqrt);
		}
	}
}

static void rootSift_process(LoadDescriptors *desloader, void *state) {
	struct State_RootSift *es = state;
	DescriptorType td_in = loadDescriptors_getDescriptorType(desloader);
	DescriptorType td_out;
	if (td_in.dtype == DTYPE_LOCAL_VECTORS) {
		td_out = descriptorType(DTYPE_LOCAL_VECTORS, 0, 0, 0);
	} else {
		my_log_error("unsupported input type\n");
	}
	char *descriptor = my_newString_format("%s-%s", es->method_name,
			loadDescriptors_getDescriptor(desloader));
	char *descAlias = my_newString_format("%s-%s", es->method_name,
			loadDescriptors_getDescriptorAlias(desloader));
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
		my_parallel_buffered(df->numDescriptors, es, applyRootSift_file,
		NULL, 0, 0);
		saveDescriptors(dessaver, fdb->id, df->numDescriptors,
				es->descriptors_output, seg);
		for (int64_t i = 0; i < df->numDescriptors; ++i) {
			MyLocalDescriptors *ldes = es->descriptors_output[i];
			my_localDescriptors_release(ldes);
		}
		MY_FREE(es->descriptors_output);
		releaseDescriptorsFile(df);
		my_progress_add1(lt);
	}
	releaseSaveDescriptors(dessaver);
	my_progress_release(lt);
}

static void rootSift_release(void *state) {
	struct State_RootSift *es = state;
	MY_FREE_MULTI(es->method_name, es);
}

void proc_reg_rootSift() {
	addProcessorDefDescriptor("ROOTSIFT", "(withTxt)", rootSift_new,
			rootSift_process, rootSift_release);
}
