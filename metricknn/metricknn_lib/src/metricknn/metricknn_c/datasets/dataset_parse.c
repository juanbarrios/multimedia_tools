/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

MknnDataset *mknn_datasetLoader_ParseVectorFile(const char *filename,
		MknnDatatype datatype) {
	//TODO: test if it is a binary file and derive it to mknn_dataset_restore
	MyDatatype dtype = mknn_datatype_convertMknn2My(datatype);
	size_t numBytesByDim = my_datatype_sizeof(dtype);
	my_function_copy_vector func_copy = my_datatype_getFunctionCopyVector(
			MY_DATATYPE_FLOAT64, dtype);
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, 1));
	my_log_info_time("reading %s\n", filename);
	int64_t num_vectors = 0, num_dimensions = 0, num_lines = 0;
	char *data_bytes = NULL;
	const char *line;
	while ((line = my_lreader_readLineOrComment(reader)) != NULL) {
		num_lines++;
		if (line[0] == '\0' || line[0] == '#')
			continue;
		MyVectorDouble *vector = my_vectorDouble_new();
		MyTokenizer *tk = my_tokenizer_new(line, ' ');
		my_tokenizer_addDelimiter(tk, '\t');
		my_tokenizer_setJoinMultipleDelimiters(tk);
		while (my_tokenizer_hasNext(tk)) {
			const char *token = my_tokenizer_nextToken(tk);
			if (strlen(token) > 0) {
				my_vectorDouble_add(vector, my_parse_double(token));
			}
		}
		my_tokenizer_release(tk);
		if (my_vectorDouble_size(vector) == 0) {
			my_vectorDouble_release(vector);
			continue;
		}
		if (num_dimensions == 0) {
			num_dimensions = my_vectorDouble_size(vector);
		} else if (num_dimensions != my_vectorDouble_size(vector)) {
			my_log_error(
					"Error at line %"PRIi64": different number of columns (%"PRIi64" != %"PRIi64")\n",
					num_lines, my_vectorDouble_size(vector), num_dimensions);
		}
		MY_REALLOC(data_bytes,
				(num_vectors + 1) * num_dimensions * numBytesByDim, char);
		char *current = data_bytes
				+ num_vectors * num_dimensions * numBytesByDim;
		func_copy(my_vectorDouble_array(vector), current, num_dimensions);
		my_vectorDouble_release(vector);
		num_vectors++;
	}
	my_lreader_close(reader, true);
	MknnDataset *dataset = mknn_datasetLoader_PointerCompactVectors(
			(void*) data_bytes, true, num_vectors, num_dimensions, datatype);
	my_log_info_time("%s: %"PRIi64" vectors %"PRIi64"-d %s\n", filename,
			num_vectors, num_dimensions, mknn_datatype_toString(datatype));
	return dataset;
}
MknnDataset *mknn_datasetLoader_ParseStringsFile(const char *filename) {
	//TODO: test if it is a binary file and derive it to mknn_dataset_restore
	MyVectorString *fileLines = my_io_loadLinesFile(filename);
	int64_t size = my_vectorString_size(fileLines);
	char **strings = my_vectorString_releaseReturnBuffer(fileLines);
	MknnDataset *dataset = mknn_datasetLoader_PointerArray((void**) strings,
			size, mknn_domain_newString(), true, true, true);
	return dataset;
}
