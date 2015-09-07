/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

static void loadQueryVectorsTxt(MyLineReader *reader, int64_t numVectors,
		struct QueryTxt *query, int64_t maxNNLoad, double maxDistLoad,
		int64_t *out_maxNNLoaded, double *out_maxDistLoaded) {
	if (numVectors == 0)
		return;
	query->name_query = MY_MALLOC(numVectors, char*);
	query->nns = MY_MALLOC(numVectors, MyVectorObj*);
	for (int64_t i = 0; i < numVectors; i++) {
		const char *line = my_lreader_readLine(reader);
		my_assert_notNull("line", line);
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *nameQ = my_newString_string(my_tokenizer_nextToken(tk));
		MyVectorObj *nnList = my_vectorObj_new();
		double distPrev = 0;
		while (my_tokenizer_hasNext(tk)) {
			struct NNTxt *nn = MY_MALLOC(1, struct NNTxt);
			nn->name_nn = my_newString_string(my_tokenizer_nextToken(tk));
			nn->distance = my_tokenizer_nextDouble(tk);
			if (distPrev > nn->distance) {
				my_log_info("not sorted distances (%lf > %lf) in %s at %s\n",
						distPrev, nn->distance, nameQ, query->name_file);
				MY_FREE_MULTI(nn->name_nn, nn);
				break;
			}
			distPrev = nn->distance;
			if (nn->distance > maxDistLoad) {
				MY_FREE_MULTI(nn->name_nn, nn);
				break;
			}
			my_vectorObj_add(nnList, nn);
			if (nn->distance > *out_maxDistLoaded)
				*out_maxDistLoaded = nn->distance;
			if (my_vectorObj_size(nnList) >= maxNNLoad)
				break;
		}
		my_tokenizer_release(tk);
		query->name_query[i] = nameQ;
		query->nns[i] = nnList;
		if (my_vectorObj_size(nnList) > *out_maxNNLoaded)
			*out_maxNNLoaded = my_vectorObj_size(nnList);
	}
}
static struct QueryTxt *parseQueryTxt(MyLineReader *reader, int64_t maxNNLoad,
		double maxDistLoad, int64_t *out_maxNNLoaded, double *out_maxDistLoaded) {
	const char *line = my_lreader_readLine(reader);
	if (line == NULL)
		return NULL;
	my_assert_prefixString("format file", line, "query=");
	MyTokenizer *tk = my_tokenizer_new(line, '\t');
	struct QueryTxt *query = MY_MALLOC(1, struct QueryTxt);
	query->name_file = my_subStringC_firstEnd(my_tokenizer_nextToken(tk), '=');
	query->num_queries = my_parse_int_csubFirstEnd(my_tokenizer_nextToken(tk), '=');
	query->search_time = my_parse_double_csubFirstEnd(my_tokenizer_nextToken(tk), '=');
	my_tokenizer_releaseValidateEnd(tk);
	loadQueryVectorsTxt(reader, query->num_queries, query, maxNNLoad,
			maxDistLoad, out_maxNNLoaded, out_maxDistLoaded);
	return query;
}
static void releaseQueryTxt(struct QueryTxt *query) {
	int64_t j, k;
	for (j = 0; j < query->num_queries; ++j) {
		MY_FREE(query->name_query[j]);
		MyVectorObj *nns = query->nns[j];
		for (k = 0; k < my_vectorObj_size(nns); ++k) {
			struct NNTxt *nn = my_vectorObj_get(nns, k);
			MY_FREE_MULTI(nn->name_nn, nn);
		}
		my_vectorObj_release(nns, 0);
	}
	MY_FREE_MULTI(query->name_query, query->nns, query->name_file, query);
}
//Array_obj of struct QueryTxt *
MyVectorObj *loadSsFileTxt(const char *filename, int64_t maxNNLoad, double maxDistLoad) {
	char *format = my_io_detectFileConfig(filename, "PVCD");
	if (format == NULL
			|| (!my_string_equals("SS", format) && !my_string_equals("SSVector", format)))
		my_log_error("invalid format '%s'\n", format);
	MyLineReader *reader = my_lreader_config_open(my_io_openFileRead1(filename, 1), "PVCD", format,
			1, 1);
	MY_FREE(format);
	int64_t maxNNLoaded = 0;
	double maxDistLoaded = 0;
	MyProgress *lt = my_progress_new(filename, 0, 1);
	MyVectorObj *allQueries = my_vectorObj_new();
	for (;;) {
		struct QueryTxt *query = parseQueryTxt(reader, maxNNLoad, maxDistLoad,
				&maxNNLoaded, &maxDistLoaded);
		if (query == NULL)
			break;
		my_vectorObj_add(allQueries, query);
		my_progress_add1(lt);
	}
	my_lreader_close(reader, true);
	my_progress_release(lt);
	char *st = my_newString_double(maxDistLoaded);
	my_log_info_time(
			"%s: %"PRIi64" queries, maxNNLoaded=%"PRIi64", maxDistLoaded=%s\n",
			filename, my_vectorObj_size(allQueries), maxNNLoaded, st);
	MY_FREE(st);
	return allQueries;
}
void loadSsFileTxt_inlineProcessByQueryFile(const char *filename, int64_t maxNNLoad,
		double maxDistLoad,
		void (*func_processQuery)(struct QueryTxt *query, void *state_func),
		void *state_func) {
	char *format = my_io_detectFileConfig(filename,"PVCD");
	if (format == NULL
			|| (!my_string_equals("SS", format) && !my_string_equals("SSVector", format)))
		my_log_error("invalid format '%s'\n", format);
	MyLineReader *reader = my_lreader_config_open(my_io_openFileRead1(filename, 1), "PVCD", format,
			1, 1);
	MY_FREE(format);
	int64_t maxNNLoaded = 0;
	double maxDistLoaded = 0;
	MyProgress *lt = my_progress_new(filename, 0, 1);
	int64_t cont = 0;
	for (;;) {
		struct QueryTxt *query = parseQueryTxt(reader, maxNNLoad, maxDistLoad,
				&maxNNLoaded, &maxDistLoaded);
		if (query == NULL)
			break;
		func_processQuery(query, state_func);
		releaseQueryTxt(query);
		my_progress_add1(lt);
		cont++;
	}
	my_lreader_close(reader, true);
	my_progress_release(lt);
	char *st = my_newString_double(maxDistLoaded);
	my_log_info_time(
			"%s: %"PRIi64" queries, maxNNLoaded=%"PRIi64", maxDistLoaded=%s\n",
			filename, cont, maxNNLoaded, st);
	MY_FREE(st);
}
MyVectorObj *loadVectorsFileTxt(const char *filename, int64_t maxNNLoad) {
	char *format = my_io_detectFileConfig(filename, "PVCD");
	if (format == NULL || !my_string_equals("SSVector", format))
		my_log_error("invalid format '%s'\n", format);
	MY_FREE(format);
	return loadSsFileTxt(filename, maxNNLoad, DBL_MAX);
}
void releaseSsFile(MyVectorObj *ssFile) {
	int64_t i;
	for (i = 0; i < my_vectorObj_size(ssFile); ++i) {
		struct QueryTxt *query = my_vectorObj_get(ssFile, i);
		releaseQueryTxt(query);
	}
	my_vectorObj_release(ssFile, 0);
}
