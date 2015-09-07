/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

void mknn_dataset_printObjectsRaw(MknnDataset *dataset, FILE *out) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	int64_t length_in_bytes = mknn_domain_vector_getVectorLengthInBytes(domain);
	for (int64_t i = 0; i < mknn_dataset_getNumObjects(dataset); ++i) {
		void *object = mknn_dataset_getObject(dataset, i);
		int64_t wrote = fwrite(object, sizeof(char), length_in_bytes, out);
		my_assert_equalInt("fwrite", wrote, length_in_bytes);
	}
}
void mknn_dataset_printObjectsText(MknnDataset *dataset, FILE *out) {
	MknnPrinter *printer = mknn_printer_new(mknn_dataset_getDomain(dataset));
	for (int64_t i = 0; i < mknn_dataset_getNumObjects(dataset); ++i) {
		void *object = mknn_dataset_getObject(dataset, i);
		char *line = mknn_printer_objectToNewString(printer, object);
		fprintf(out, "%s\n", line);
		free(line);
	}
	mknn_printer_release(printer);
}
void mknn_dataset_printObjectsRawFile(MknnDataset *dataset,
		const char *filename_write) {
	char *st_domain = mknn_domain_toString(mknn_dataset_getDomain(dataset));
	my_log_info("printing %"PRIi64" objects, domain=%s, format=raw\n",
			mknn_dataset_getNumObjects(dataset), st_domain);
	FILE *out = my_io_openFileWrite1(filename_write);
	mknn_dataset_printObjectsRaw(dataset, out);
	fclose(out);
	free(st_domain);
}
void mknn_dataset_printObjectsTextFile(MknnDataset *dataset,
		const char *filename_write) {
	char *st_domain = mknn_domain_toString(mknn_dataset_getDomain(dataset));
	my_log_info("printing %"PRIi64" objects, domain=%s, format=text\n",
			mknn_dataset_getNumObjects(dataset), st_domain);
	FILE *out = my_io_openFileWrite1(filename_write);
	mknn_dataset_printObjectsText(dataset, out);
	fclose(out);
	free(st_domain);
}

void mknn_dataset_save(MknnDataset *dataset, const char *filename_write) {
	char *st_domain = mknn_domain_toString(mknn_dataset_getDomain(dataset));
	my_log_info("printing %"PRIi64" objects, domain=%s, format=mknn\n",
			mknn_dataset_getNumObjects(dataset), st_domain);
	FILE *out = my_io_openFileWrite1Config(filename_write, "MetricKnn",
			"MknnDataset", 1, 2);
	fprintf(out, "dataset.domain=%s\n", st_domain);
	fprintf(out, "dataset.num_objects=%"PRIi64"\n",
			mknn_dataset_getNumObjects(dataset));
	fprintf(out, "--\n\n");
	if (mknn_domain_isGeneralDomainString(mknn_dataset_getDomain(dataset))) {
		for (int64_t i = 0; i < mknn_dataset_getNumObjects(dataset); ++i) {
			char *line = mknn_dataset_getObject(dataset, i);
			fprintf(out, "%s\n", line);
		}
	} else if (mknn_domain_isGeneralDomainVector(
			mknn_dataset_getDomain(dataset))) {
		mknn_dataset_printObjectsRaw(dataset, out);
	}
	free(st_domain);
	fclose(out);
}
MknnDataset *mknn_dataset_restore(const char *filename_read) {
	FILE *input = my_io_openFileRead1(filename_read, true);
	MyLineReader *reader = my_lreader_config_open_params(input, 1, "MetricKnn",
			"MknnDataset", 1, 2);
	MknnDomain *domain = NULL;
	int64_t num_objects = -1;
	for (;;) {
		const char *line = my_lreader_readLine(reader);
		if (line == NULL || my_string_equals(line, "--"))
			break;
		int64_t pos = my_string_indexOf(line, "=");
		if (my_string_startsWith_ignorecase(line, "dataset.domain=")) {
			domain = mknn_domain_newParseString(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"dataset.num_objects=")) {
			num_objects = my_parse_int(line + pos + 1);
		} else {
			my_log_error("invalid line %s\n", line);
		}
	}
	my_lreader_close(reader, false);
	if (domain == NULL || num_objects < 0)
		my_log_error("invalid format %s\n", filename_read);
	MknnDataset *dataset = NULL;
	if (mknn_domain_isGeneralDomainString(domain)) {
		char **strings = MY_MALLOC(num_objects, char*);
		for (int64_t i = 0; i < num_objects; ++i)
			strings[i] = my_lreader_readLine_newString(reader);
		dataset = mknn_datasetLoader_PointerArray((void**) strings, num_objects,
				domain, true, true, true);
		mknn_domain_release(domain);
	} else if (mknn_domain_isGeneralDomainVector(domain)) {
		int64_t expected_size = num_objects
				* mknn_domain_vector_getVectorLengthInBytes(domain);
		void *data_bytes = MY_MALLOC_NOINIT(expected_size, char);
		my_io_readBytesFile(input, data_bytes, expected_size, true);
		MknnDataset *dataset = mknn_datasetLoader_PointerCompactVectors_alt(
				data_bytes, true, num_objects, domain, true);
		return dataset;
	} else {
		my_log_error("can't load domain %s in %s\n",
				mknn_generalDomain_toString(
						mknn_domain_getGeneralDomain(domain)), filename_read);
	}
	my_log_info_time("%s: loaded %"PRIi64" objects\n", filename_read,
			num_objects);
	return dataset;
}
