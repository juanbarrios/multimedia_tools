/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

#if 0
struct State_Idf {
	char *outFilename;
};

static void idf_new(const char *procCode, const char *segParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(segParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct State_Idf *es = MY_MALLOC(1, struct State_Idf);
	es->outFilename = my_tokenizer_nextToken_newString(tk);
	my_tokenizer_release(tk);
	*out_state = es;
}
static void sum1ByDimension(void *vector, int64_t num_dims, MknnDatatype type,
		int64_t *cont_by_dimension) {
	if (type == MKNN_DATATYPE_FLOATING_POINT_32bits) {
		float *array = vector;
		for (int64_t i = 0; i < num_dims; ++i) {
			if (array[i] != 0)
				cont_by_dimension[i]++;
		}
	} else if (type == MKNN_DATATYPE_FLOATING_POINT_64bits) {
		double *array = vector;
		for (int64_t i = 0; i < num_dims; ++i) {
			if (array[i] != 0)
				cont_by_dimension[i]++;
		}
	} else if (type == MKNN_DATATYPE_UNSIGNED_INTEGER_8bits) {
		unsigned char *array = vector;
		for (int64_t i = 0; i < num_dims; ++i) {
			if (array[i] != 0)
				cont_by_dimension[i]++;
		}
	} else {
		my_log_error("unsupported type\n");
	}
}
static bool idf_process(int64_t num_desloaders, LoadDescriptors **desloaders,
		void *state) {
	struct State_Idf *es = state;
	int64_t num_dims = 0;
	int64_t *cont_by_dimension = NULL;
	int64_t cont_total = 0;
	for (int64_t i = 0; i < num_desloaders; ++i) {
		LoadDescriptors *loader = desloaders[i];
		DB *db = loadDescriptors_getDb(desloaders[i]);
		for (int64_t j = 0; j < db->numFilesDb; ++j) {
			MknnDataset *dataset =
					getAlreadyComputedDescriptorsSample_singleFile(loader,
							db->filesDb[j], 0, 0);
			int64_t nd = mknn_domain_vector_getNumDimensions(
					mknn_dataset_getDomain(dataset));
			if (num_dims == 0) {
				num_dims = nd;
				cont_by_dimension = MY_MALLOC(num_dims, int64_t);
			} else {
				my_assert_equalInt("num_dimensions", nd, num_dims);
			}
			MknnDatatype type = mknn_domain_vector_getDimensionDataType(
					mknn_dataset_getDomain(dataset));
			for (int64_t i = 0; i < mknn_dataset_getNumObjects(dataset); ++i) {
				void *vector = mknn_dataset_getObject(dataset, i);
				sum1ByDimension(vector, num_dims, type, cont_by_dimension);
			}
			cont_total += mknn_dataset_getNumObjects(dataset);
			mknn_dataset_release(dataset);
		}
	}
	if (cont_total > 0 && num_dims > 0) {
		char *fname =
				my_string_endsWith_ignorecase(es->outFilename, ".idf") ?
						my_newString_string(es->outFilename) :
						my_newString_format("%s.idf", es->outFilename);
		FILE *out = my_io_openFileWrite1Config(fname, "PVCD", "IDF", 1, 0);
		free(fname);
		fprintf(out, "%"PRIi64"\t%"PRIi64"\n", num_dims, cont_total);
		for (int64_t i = 0; i < num_dims; ++i) {
			fprintf(out, "%"PRIi64"\n", cont_by_dimension[i]);
		}
		fclose(out);
	}
	return true;
}
static void idf_release(void *state) {
	struct State_Idf *es = state;
	MY_FREE(es->outFilename);
	MY_FREE(es);
}

void proc_reg_idf() {
	addProcessorDefDescriptor("IDF", "outFilename", idf_new, idf_process,
			idf_release);
}
#endif
