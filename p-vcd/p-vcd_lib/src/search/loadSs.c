/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

struct LoadSSOptions {
	int64_t maxNNload;
	double maxDistLoad;
	bool ignore_same_video;
	int64_t contNnLoaded, contNnNotFound, contNnIgnored;
};

static void det_cargarFrame(struct SearchCollection *colReference,
		const char* line, struct D_Frame *f, struct LoadSSOptions *opt) {
	MyTokenizer *tk = my_tokenizer_new(line, '\t');
	const char *token = my_tokenizer_nextToken(tk);
	f->ssegmentQuery = findSearchSegmentInFile(f->query->sfileQuery, token, 1);
	MyVectorObj *list = my_vectorObj_new();
	int64_t pos = 0;
	while ((token = my_tokenizer_nextToken(tk)) != NULL
			&& (opt->maxNNload <= 0 || pos < opt->maxNNload)) {
		struct SearchSegment *kfRef = findSearchSegmentInCollection(
				colReference, token, 0);
		double distance = my_tokenizer_nextDouble(tk);
		if (opt->ignore_same_video
				&& my_string_equals(f->ssegmentQuery->sfile->fdb->filenameReal,
						kfRef->sfile->fdb->filenameReal)) {
			opt->contNnIgnored++;
			continue;
		}
		pos++;
		if (opt->maxDistLoad > 0 && distance > opt->maxDistLoad)
			break;
		if (kfRef == NULL) {
			opt->contNnNotFound++;
		} else {
			opt->contNnLoaded++;
			struct D_Match *m = MY_MALLOC(1, struct D_Match);
			m->ssegmentRef = kfRef;
			m->distance = distance;
			m->rank = pos;
			my_vectorObj_add(list, m);
		}
	}
	my_tokenizer_release(tk);
	f->numNNs = my_vectorObj_size(list);
	f->nns = (struct D_Match **) my_vectorObj_array(list);
	MY_FREE(list);
}
static struct D_Query *det_cargarFramesVideo(struct SearchProfile *profile,
		MyLineReader *reader, struct LoadSSOptions *opt) {
	const char *line = my_lreader_readLine(reader);
	if (line == NULL)
		return NULL;
	my_assert_prefixString("format file", line, "query=");
	MyTokenizer *tk = my_tokenizer_new(line, '\t');
	char *query_name = my_subStringC_firstEnd(my_tokenizer_nextToken(tk), '=');
	int64_t numSegments = my_parse_int_csubFirstEnd(my_tokenizer_nextToken(tk),
			'=');
	double searchTime = my_parse_double_csubFirstEnd(my_tokenizer_nextToken(tk),
			'=');
	my_tokenizer_releaseValidateEnd(tk);
	struct SearchFile *arcQuery = findSearchFileInCollection(profile->colQuery,
			query_name, false);
	if (arcQuery == NULL) {
		my_log_info("can't find query %s\n", query_name);
		for (int64_t i = 0; i < numSegments; i++)
			line = my_lreader_readLine(reader);
		return det_cargarFramesVideo(profile, reader, opt);
	}
	my_assert_equalInt("numSegments", numSegments, arcQuery->numSegments);
	struct D_Query *query = MY_MALLOC(1, struct D_Query);
	query->sfileQuery = arcQuery;
	query->numFrames = numSegments;
	query->searchTime = searchTime;
	query->frames = MY_MALLOC(query->numFrames, struct D_Frame *);
	for (int64_t i = 0; i < numSegments; i++) {
		line = my_lreader_readLine(reader);
		my_assert_notNull("line", line);
		query->frames[i] = MY_MALLOC(1, struct D_Frame);
		query->frames[i]->query = query;
		det_cargarFrame(profile->colReference, line, query->frames[i], opt);
	}
	return query;
}
struct ArchivoFrames *loadFileSS(const char *filename, int64_t maxNNLoad,
		double maxDistLoad, struct SearchProfile *profile) {
	struct ArchivoFrames *af = MY_MALLOC(1, struct ArchivoFrames);
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filename, true), "PVCD", "SS", 1, 1);
	char *size = my_newString_diskSpace(my_io_getFilesize(filename));
	my_log_info_time("loading file %s (%s)\n", filename, size);
	free(size);
	af->allQueries = my_vectorObj_new();
	struct LoadSSOptions opt = { 0 };
	opt.maxNNload = maxNNLoad;
	opt.maxDistLoad = maxDistLoad;
	opt.ignore_same_video = true;
	for (;;) {
		struct D_Query *queryVideo = det_cargarFramesVideo(profile, reader,
				&opt);
		if (queryVideo == NULL)
			break;
		queryVideo->af = af;
		my_vectorObj_add(af->allQueries, queryVideo);
		for (int64_t i = 0; i < queryVideo->numFrames; ++i) {
			struct D_Frame *frame = queryVideo->frames[i];
			if (frame->numNNs > af->max_nns)
				af->max_nns = frame->numNNs;
			for (int64_t j = 0; j < frame->numNNs; ++j) {
				struct D_Match *nn = frame->nns[j];
				if (nn->distance > af->max_dist)
					af->max_dist = nn->distance;
			}
		}
	}
	my_lreader_close(reader, true);
	if (my_vectorObj_size(af->allQueries) == 0)
		my_log_error("could not load any query at %s\n", filename);
	my_log_info_time(
			"loaded file: %"PRIi64" queries, max %"PRIi64" NNs, total %"PRIi64" NNs\n",
			my_vectorObj_size(af->allQueries), af->max_nns, opt.contNnLoaded);
	if (opt.contNnIgnored > 0)
		my_log_info("%"PRIi64" NNs were ignored\n", opt.contNnIgnored);
	if (opt.contNnNotFound > 0)
		my_log_info("%"PRIi64" NNs could not be loaded\n", opt.contNnNotFound);
	return af;
}
void releaseFileSS(struct ArchivoFrames *af) {
	int j, k;
	for (j = 0; j < my_vectorObj_size(af->allQueries); ++j) {
		struct D_Query *qv = my_vectorObj_get(af->allQueries, j);
		for (k = 0; k < qv->numFrames; ++k) {
			struct D_Frame *qframe = qv->frames[k];
			MY_FREE_MATRIX(qframe->nns, qframe->numNNs);
			MY_FREE(qframe);
		}
		MY_FREE_MULTI(qv->frames, qv);
	}
	my_vectorObj_release(af->allQueries, 0);
	MY_FREE(af);
}
