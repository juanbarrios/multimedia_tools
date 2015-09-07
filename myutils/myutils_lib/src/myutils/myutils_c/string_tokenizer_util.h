/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_STRING_TOKENIZER_UTIL_H
#define MY_STRING_TOKENIZER_UTIL_H

#include "../myutils_c.h"

typedef struct MyTokenizer MyTokenizer;

MyTokenizer *my_tokenizer_new(const char* line, char token_delimiter);

void my_tokenizer_addDelimiter(MyTokenizer *tk, char other_delimiter);
void my_tokenizer_setJoinMultipleDelimiters(MyTokenizer *tk);
void my_tokenizer_useBrackets(MyTokenizer *tk, char open_bracket,
		char close_bracket);

const char *my_tokenizer_nextToken(MyTokenizer *tk);
char *my_tokenizer_nextToken_newString(MyTokenizer *tk);
bool my_tokenizer_hasNext(MyTokenizer *tk);
bool my_tokenizer_isNext(MyTokenizer *tk, const char *value);
double my_tokenizer_nextDouble(MyTokenizer *tk);
double my_tokenizer_nextDouble0(MyTokenizer *tk);
double my_tokenizer_nextFraction(MyTokenizer *tk);
double my_tokenizer_nextSeconds(MyTokenizer *tk);
int64_t my_tokenizer_nextInt(MyTokenizer *tk);
int64_t my_tokenizer_nextInt0(MyTokenizer *tk);
const char *my_tokenizer_getCurrentTail(MyTokenizer *tk);
void my_tokenizer_release(MyTokenizer *tk);
void my_tokenizer_releaseValidateEnd(MyTokenizer *tk);

MyVectorString *my_tokenizer_split(MyTokenizer *tk);
MyVectorInt *my_tokenizer_splitInt(MyTokenizer *tk);
MyVectorDouble *my_tokenizer_splitDouble(MyTokenizer *tk);

MyVectorString *my_tokenizer_splitLine(const char* line, char token_delimiter);
MyVectorString *my_tokenizer_splitInWords(const char* line);
MyVectorInt *my_tokenizer_splitLineInt(const char* line, char token_delimiter);
MyVectorDouble *my_tokenizer_splitLineDouble(const char* line,
		char token_delimiter);

#endif
