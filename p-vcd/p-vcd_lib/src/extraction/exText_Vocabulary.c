/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

 #include "ex.h"
#include <myutils/myutils_c.h>

#include "exText.h"

#if USE_TEXT

struct Vocabulary {
	int64_t num_words;
	char **words;
	double *idf_weights;
};

struct Vocabulary *vocabulary_load_file(const char *filename) {
	struct Vocabulary *v = MY_MALLOC(1, struct Vocabulary);
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, true));
	const char *line = my_lreader_readLine(reader);
	MyTokenizer *tk1 = my_tokenizer_new(line, '\t');
	v->num_words = my_tokenizer_nextInt(tk1);
	int64_t idf_total_documents = my_tokenizer_nextInt(tk1);
	my_tokenizer_releaseValidateEnd(tk1);
	v->words = MY_MALLOC(v->num_words, char*);
	v->idf_weights = MY_MALLOC(v->num_words, double);
	int64_t cont = 0;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *word = my_tokenizer_nextToken_newString(tk);
		int64_t idf_documents_word = my_tokenizer_nextInt(tk);
		my_tokenizer_releaseValidateEnd(tk);
		my_assert_lessInt("word_id", cont, v->num_words);
		v->words[cont] = word;
		v->idf_weights[cont] = log(
				idf_total_documents / (double) idf_documents_word);
		if (cont > 0
				&& my_compare_string_ignorecase(v->words[cont - 1],
						v->words[cont]) >= 0) {
			my_log_error("words must be sorted (%s >= %s)\n", v->words[cont - 1],
					v->words[cont]);
		}
		cont++;
	}
	my_lreader_close(reader, true);
	my_assert_equalInt("cont", cont, v->num_words);
	return v;
}
int64_t vocabulary_getNumWords(struct Vocabulary *v) {
	return v->num_words;
}
int64_t vocabulary_getWordId(struct Vocabulary *v, const char *word) {
	int64_t pos = -1;
	bool exists = my_binsearch_objArr(word, (void**) v->words, v->num_words,
			my_compare_string_ignorecase, &pos);
	return (exists ? pos : -1);
}
double vocabulary_getWordIdfWeight(struct Vocabulary *v, int64_t word_id) {
	my_assert_indexRangeInt("word_id", word_id, v->num_words);
	return v->idf_weights[word_id];
}
void vocabulary_release(struct Vocabulary *v) {
	MY_FREE_MATRIX(v->words, v->num_words);
	free(v->idf_weights);
	free(v);
}

#endif
