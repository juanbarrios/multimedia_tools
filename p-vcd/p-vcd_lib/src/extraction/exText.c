/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV

#include "exText.h"

#if USE_TEXT

MY_MUTEX_NEWSTATIC(global_mutex);

static MyVectorObj *vbuilders = NULL;

struct State_Text {
	char *filenameVocabulary, *inputDirTexts;
	bool use_word_stemmer;
	struct VocabularyBuilder *vbuilder;
	struct Vocabulary *vocabulary;
	struct SubtitlesFile *subtitles_current_file;
	int64_t descriptorLength;
	double *double_array;
	MySparseArray *descriptor;
};

static void ext_new_text(const char *extCode, const char *extParameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(extParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct State_Text *es = MY_MALLOC(1, struct State_Text);
	es->filenameVocabulary = my_tokenizer_nextToken_newString(tk);
	es->inputDirTexts = my_tokenizer_nextToken_newString(tk);
	my_tokenizer_releaseValidateEnd(tk);
	es->use_word_stemmer = true;
	if (my_io_existsFile(es->filenameVocabulary)) {
		es->vocabulary = vocabulary_load_file(es->filenameVocabulary);
		es->descriptorLength = vocabulary_getNumWords(es->vocabulary);
	} else {
		es->vbuilder = vocabularyBuilder_new_empty();
		es->descriptorLength = 1;
		MY_MUTEX_LOCK(global_mutex);
		if (vbuilders == NULL)
			vbuilders = my_vectorObj_new();
		my_vectorObj_add(vbuilders, es->vbuilder);
		MY_MUTEX_UNLOCK(global_mutex);
	}
	es->double_array = MY_MALLOC_NOINIT(es->descriptorLength, double);
	es->descriptor = my_sparseArray_new();
	*out_state = es;
	*out_td = descriptorType(DTYPE_SPARSE_ARRAY, es->descriptorLength, 1,
			es->descriptorLength);
	*out_useImgGray = false;
}

static void ext_func_init_video(struct Extractor_InitParams *ip, void *state) {
	struct State_Text *es = state;
	char *fname1 = my_newString_format("%s/%s.txt", es->inputDirTexts,
			ip->fileDB->id);
	char *basename = my_io_getFilenameWithoutExtension(
			ip->fileDB->filenameReal);
	char *fname2 = my_newString_format("%s/%s.txt", es->inputDirTexts,
			basename);
	if (my_io_existsFile(fname1))
		es->subtitles_current_file = subtitles_load(fname1,
				es->use_word_stemmer);
	else if (my_io_existsFile(fname2))
		es->subtitles_current_file = subtitles_load(fname2,
				es->use_word_stemmer);
	else
		my_log_error("can't load text file %s or %s\n", fname1, fname2);
	MY_FREE_MULTI(fname1, fname2, basename);
	if (es->vbuilder != NULL) {
		for (int64_t i = 0;
				i < subtitles_getNumLines(es->subtitles_current_file); ++i) {
			MyVectorString *words = subtitles_getWordsLine(
					es->subtitles_current_file, i);
			for (int64_t j = 0; j < my_vectorString_size(words); ++j) {
				char *word = my_vectorString_get(words, j);
				vocabularyBuilder_contWordInDocument(es->vbuilder, word);
			}
			vocabularyBuilder_endOfDocument(es->vbuilder);
		}
	}
}
static int64_t cont_words(MyVectorString *words, struct Vocabulary *v,
		double *freq) {
	int64_t sum = 0;
	for (int64_t i = 0; i < my_vectorString_size(words); ++i) {
		char *word = my_vectorString_get(words, i);
		int64_t word_id = vocabulary_getWordId(v, word);
		if (word_id < 0)
			continue;
		freq[word_id]++;
		sum++;
	}
	return sum;
}
static void compute_tf_idf_normalizeL2(struct Vocabulary *v, double *freq,
		int64_t sum) {
	int64_t size = vocabulary_getNumWords(v);
	double norm = 0;
	for (int64_t i = 0; i < size; ++i) {
		freq[i] = (freq[i] / sum) * vocabulary_getWordIdfWeight(v, i);
		norm += freq[i] * freq[i];
	}
	if (norm != 0 && norm != 1) {
		norm = sqrt(norm);
		for (int64_t i = 0; i < size; ++i) {
			freq[i] /= norm;
		}
	}
}
static void *ext_segment_text(struct Extractor_InitParams *ip,
		int64_t idSegment, void *state) {
	struct State_Text *es = state;
	if (es->vocabulary == NULL)
		return es->descriptor;
	struct VideoSegment *s = ip->segmentation->segments + idSegment;
	MY_SETZERO(es->double_array, es->descriptorLength, double);
	int64_t sum = 0;
	for (int64_t i = 0; i < subtitles_getNumLines(es->subtitles_current_file);
			++i) {
		if (!subtitles_isLineRelevant(es->subtitles_current_file, i,
				s->start_second, s->end_second))
			continue;
		MyVectorString *words = subtitles_getWordsLine(
				es->subtitles_current_file, i);
		sum += cont_words(words, es->vocabulary, es->double_array);
	}
	if (sum > 0)
		compute_tf_idf_normalizeL2(es->vocabulary, es->double_array, sum);
	my_sparseArray_storeArrayDouble(es->descriptor, es->double_array,
			es->descriptorLength);
	return es->descriptor;
}
static void ext_func_end_video(struct Extractor_InitParams *ip, void* state) {
	struct State_Text *es = state;
	subtitles_release(es->subtitles_current_file);
}
static void ext_release_text(void *state) {
	struct State_Text *es = state;
	MY_MUTEX_LOCK(global_mutex);
	if (vbuilders != NULL) {
		struct VocabularyBuilder *all = vocabularyBuilder_new_empty();
		MyProgress* lt = my_progress_new("merge vocabulary",
				my_vectorObj_size(vbuilders), 1);
		for (int64_t i = 0; i < my_vectorObj_size(vbuilders); ++i) {
			struct VocabularyBuilder *vb = my_vectorObj_get(vbuilders, i);
			vocabularyBuilder_addVocabulary(all, vb);
			vocabularyBuilder_release(vb);
			my_progress_add1(lt);
		}
		my_progress_release(lt);
		vocabularyBuilder_saveVocabularyToFile(all, es->filenameVocabulary);
		vocabularyBuilder_release(all);
		my_vectorObj_release(vbuilders, false);
		vbuilders = NULL;
	}
	MY_MUTEX_UNLOCK(global_mutex);
	if (es->vocabulary != NULL)
		vocabulary_release(es->vocabulary);
	my_sparseArray_release(es->descriptor);
	MY_FREE_MULTI(es->filenameVocabulary, es->inputDirTexts, es->double_array,
			es);
}

void ext_reg_text() {
	addExtractorSegmentDef("TEXT", "[fileVocabulary]_[inputDirTexts]",
			ext_new_text, ext_func_init_video, ext_segment_text,
			ext_func_end_video, ext_release_text);
}
#endif
#endif
