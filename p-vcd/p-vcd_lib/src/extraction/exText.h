/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef EX_TEXT_H
#define EX_TEXT_H

#include "../pvcd.h"

#define USE_TEXT 1

#if USE_TEXT

/*******************************/
struct Vocabulary;

struct Vocabulary *vocabulary_load_file(const char *filename);

int64_t vocabulary_getNumWords(struct Vocabulary *v);

int64_t vocabulary_getWordId(struct Vocabulary *v, const char *word);

double vocabulary_getWordIdfWeight(struct Vocabulary *v, int64_t word_id);

void vocabulary_release(struct Vocabulary *v);

/*******************************/
struct VocabularyBuilder;

struct VocabularyBuilder *vocabularyBuilder_new_empty();

void vocabularyBuilder_contWordInDocument(struct VocabularyBuilder *vb,
		const char *word);

void vocabularyBuilder_endOfDocument(struct VocabularyBuilder *vb);

void vocabularyBuilder_addVocabulary(struct VocabularyBuilder *this_vb,
		struct VocabularyBuilder *other_vb);

void vocabularyBuilder_saveVocabularyToFile(struct VocabularyBuilder *vb,
		const char *filename);

void vocabularyBuilder_release(struct VocabularyBuilder *vb);

/*******************************/
struct SubtitlesFile;

struct SubtitlesFile *subtitles_load(const char *filename, bool use_word_stemmer);

int64_t subtitles_getNumLines(struct SubtitlesFile *subtitles);

MyVectorString *subtitles_getWordsLine(struct SubtitlesFile *subtitles,
		int64_t numLine);

bool subtitles_isLineRelevant(struct SubtitlesFile *subtitles, int64_t numLine,
		double intervalStart, double intervalEnd);

void subtitles_release(struct SubtitlesFile *subtitles);

#endif

#endif
