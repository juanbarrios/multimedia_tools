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

struct WordConts {
	int64_t current_cont;
	int64_t cont_documents_with_word;
	char *word;
};
struct VocabularyBuilder {
	MyVectorObj *cont_words;
	int64_t cont_documents;
};

struct VocabularyBuilder *vocabularyBuilder_new_empty() {
	struct VocabularyBuilder *vb = MY_MALLOC(1, struct VocabularyBuilder);
	vb->cont_words = my_vectorObj_new();
	return vb;
}
static int search_conts(const void *search_object, const void *array_object) {
	const char *word = search_object;
	const struct WordConts *c = array_object;
	return my_compare_string_ignorecase(word, c->word);
}
static int qsort_conts(const void *obj1, const void *obj2) {
	const struct WordConts *c1 = *((const struct WordConts **) obj1);
	const struct WordConts *c2 = *((const struct WordConts **) obj2);
	return my_compare_string_ignorecase(c1->word, c2->word);
}
void vocabularyBuilder_contWordInDocument(struct VocabularyBuilder *vb,
		const char *word) {
	int64_t pos = my_vectorObj_indexOf_binsearch(vb->cont_words, word,
			search_conts);
	if (pos >= 0) {
		struct WordConts *c = my_vectorObj_get(vb->cont_words, pos);
		c->current_cont++;
	} else {
		struct WordConts *c = MY_MALLOC(1, struct WordConts);
		c->word = my_newString_string(word);
		c->current_cont = 1;
		my_vectorObj_add(vb->cont_words, c);
		my_vectorObj_qsort(vb->cont_words, qsort_conts);
	}
}
void vocabularyBuilder_endOfDocument(struct VocabularyBuilder *vb) {
	bool exists_docs = false;
	for (int64_t i = 0; i < my_vectorObj_size(vb->cont_words); ++i) {
		struct WordConts *c = my_vectorObj_get(vb->cont_words, i);
		if (c->current_cont > 0) {
			c->current_cont = 0;
			c->cont_documents_with_word++;
			exists_docs = true;
		}
	}
	if (exists_docs)
		vb->cont_documents++;
}
void vocabularyBuilder_addVocabulary(struct VocabularyBuilder *this_vb,
		struct VocabularyBuilder *other_vb) {
	for (int64_t i = 0; i < my_vectorObj_size(other_vb->cont_words); ++i) {
		struct WordConts *other_c = my_vectorObj_get(other_vb->cont_words, i);
		int64_t pos = my_vectorObj_indexOf_binsearch(this_vb->cont_words,
				other_c->word, search_conts);
		if (pos >= 0) {
			struct WordConts *this_c = my_vectorObj_get(this_vb->cont_words,
					pos);
			this_c->cont_documents_with_word +=
					other_c->cont_documents_with_word;
		} else {
			struct WordConts *this_c = MY_MALLOC(1, struct WordConts);
			this_c->word = my_newString_string(other_c->word);
			this_c->cont_documents_with_word =
					other_c->cont_documents_with_word;
			my_vectorObj_add(this_vb->cont_words, this_c);
			my_vectorObj_qsort(this_vb->cont_words, qsort_conts);
		}
	}
	this_vb->cont_documents += other_vb->cont_documents;
}
void vocabularyBuilder_saveVocabularyToFile(struct VocabularyBuilder *vb,
		const char *filename) {
	int64_t cont_remove = 0;
	for (int64_t i = 0; i < my_vectorObj_size(vb->cont_words); ++i) {
		struct WordConts *c = my_vectorObj_get(vb->cont_words, i);
		if (c->cont_documents_with_word > vb->cont_documents)
			my_log_error("word '%s' has invalid cont %"PRIi64"/%"PRIi64"\n",
					c->word, c->cont_documents_with_word, vb->cont_documents);
		if (c->cont_documents_with_word == vb->cont_documents) {
			my_vectorObj_remove(vb->cont_words, i);
			i--;
			cont_remove++;
		}
	}
	my_log_info("stop list: %"PRIi64" words in all documents\n", cont_remove);
	FILE *out = my_io_openFileWrite1(filename);
	fprintf(out, "%"PRIi64"\t%"PRIi64"\n", my_vectorObj_size(vb->cont_words),
			vb->cont_documents);
	for (int64_t i = 0; i < my_vectorObj_size(vb->cont_words); ++i) {
		struct WordConts *c = my_vectorObj_get(vb->cont_words, i);
		fprintf(out, "%s\t%"PRIi64"\n", c->word, c->cont_documents_with_word);
	}
	fclose(out);
}
void vocabularyBuilder_release(struct VocabularyBuilder *vb) {
	for (int64_t i = 0; i < my_vectorObj_size(vb->cont_words); ++i) {
		struct WordConts *c = my_vectorObj_get(vb->cont_words, i);
		free(c->word);
	}
	my_vectorObj_release(vb->cont_words, true);
	free(vb);
}

#endif
