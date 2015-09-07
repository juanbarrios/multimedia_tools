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

struct stemmer;

extern struct stemmer * create_stemmer(void);
extern void free_stemmer(struct stemmer * z);

extern int stem(struct stemmer * z, char * b, int k);

struct TextLine {
	double time_start, time_end;
	MyVectorString *words;
};
struct SubtitlesFile {
	int64_t num_lines;
	struct TextLine *text_lines;
};

struct SubtitlesFile *subtitles_load(const char *filename, bool use_word_stemmer) {
	struct stemmer *stemmer = create_stemmer();
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, true));
	struct SubtitlesFile *subtitles = MY_MALLOC(1, struct SubtitlesFile);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		double time_start = my_tokenizer_nextSeconds(tk);
		double time_end = my_tokenizer_nextSeconds(tk);
		MyVectorString *words = my_tokenizer_splitInWords(
				my_tokenizer_getCurrentTail(tk));
		my_tokenizer_release(tk);
		if (my_vectorString_size(words) == 0) {
			my_vectorString_release(words, true);
			continue;
		}
		if (use_word_stemmer) {
			for (int64_t i = 0; i < my_vectorString_size(words); ++i) {
				char *word = my_vectorString_get(words, i);
				int k2 = stem(stemmer, word, strlen(word) - 1);
				word[k2 + 1] = '\0';
			}
		}
		MY_REALLOC(subtitles->text_lines, subtitles->num_lines + 1,
				struct TextLine);
		struct TextLine *tl = subtitles->text_lines + subtitles->num_lines;
		tl->time_start = time_start;
		tl->time_end = time_end;
		tl->words = words;
		subtitles->num_lines++;
	}
	my_lreader_close(reader, true);
	free_stemmer(stemmer);
	return subtitles;
}
int64_t subtitles_getNumLines(struct SubtitlesFile *subtitles) {
	return subtitles->num_lines;
}
MyVectorString *subtitles_getWordsLine(struct SubtitlesFile *subtitles,
		int64_t numLine) {
	my_assert_indexRangeInt("numLine", numLine, subtitles->num_lines);
	return subtitles->text_lines[numLine].words;
}

bool subtitles_isLineRelevant(struct SubtitlesFile *subtitles, int64_t numLine,
		double intervalStart, double intervalEnd) {
	return MY_INTERSECT(intervalStart, intervalEnd,
			subtitles->text_lines[numLine].time_start,
			subtitles->text_lines[numLine].time_end);
}
void subtitles_release(struct SubtitlesFile *subtitles) {
	for (int64_t i = 0; i < subtitles->num_lines; ++i) {
		my_vectorString_release(subtitles->text_lines[i].words, true);
	}
	MY_FREE(subtitles->text_lines);
	free(subtitles);
}

#endif
