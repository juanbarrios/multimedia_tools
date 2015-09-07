/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "string_tokenizer_util.h"

struct MyTokenizer {
	const char *line, *original_line;
	char *buffer;
	char delimiter1, delimiter2, delimiter3;
	int64_t buffer_size;
	bool useBrackets, joinMultipleDelimiters;
	char open_bracket, close_bracket;
};

MyTokenizer *my_tokenizer_new(const char* line, char token_delimiter) {
	MyTokenizer *tk = MY_MALLOC(1, MyTokenizer);
	tk->original_line = tk->line = line;
	tk->delimiter1 = token_delimiter;
	tk->delimiter2 = tk->delimiter3 = '\0';
	return tk;
}

void my_tokenizer_addDelimiter(MyTokenizer *tk, char other_delimiter) {
	if (tk->delimiter1 == '\0' || tk->delimiter1 == other_delimiter)
		tk->delimiter1 = other_delimiter;
	else if (tk->delimiter2 == '\0' || tk->delimiter2 == other_delimiter)
		tk->delimiter2 = other_delimiter;
	else if (tk->delimiter3 == '\0' || tk->delimiter3 == other_delimiter)
		tk->delimiter3 = other_delimiter;
	else
		my_log_error("too many delimiters in MyTokenizer\n");
}
void my_tokenizer_setJoinMultipleDelimiters(MyTokenizer *tk) {
	tk->joinMultipleDelimiters = true;
}
void my_tokenizer_useBrackets(MyTokenizer *tk, char open_bracket,
		char close_bracket) {
	if (open_bracket == close_bracket)
		my_log_error(
				"MyTokenizer requires different brackets in order to support nesting\n");
	tk->useBrackets = true;
	tk->open_bracket = open_bracket;
	tk->close_bracket = close_bracket;
}
static bool my_is_matched_bracket(const char *string, int64_t posStart,
		int64_t posEnd, char open_bracket, char close_bracket) {
	int64_t contOpenBrackets = 1;
	for (int64_t i = posStart + 1; i < posEnd; ++i) {
		if (string[i] == open_bracket) {
			contOpenBrackets++;
		} else if (string[i] == close_bracket) {
			contOpenBrackets--;
			if (contOpenBrackets == 0)
				return 0;
		}
	}
	return (contOpenBrackets == 1) ? true : false;
}
const char *my_tokenizer_nextToken(MyTokenizer *tk) {
	if (tk->line == NULL || tk->line[0] == '\0')
		return NULL;
	if (tk->joinMultipleDelimiters) {
		for (;;) {
			char c = tk->line[0];
			if (c == '\0')
				return NULL;
			else if (c == tk->delimiter1 || c == tk->delimiter2
					|| c == tk->delimiter3)
				tk->line++;
			else
				break;
		}
	}
	int64_t pos = 0, contOpenBrackets = 0;
	char c;
	while ((c = tk->line[pos]) != '\0') {
		if (tk->useBrackets) {
			if (c == tk->open_bracket) {
				contOpenBrackets++;
			} else if (c == tk->close_bracket) {
				contOpenBrackets--;
				if (contOpenBrackets < 0)
					my_log_error("unbalanced brackets %s\n", tk->original_line);
			}
		}
		if ((c == tk->delimiter1 || c == tk->delimiter2 || c == tk->delimiter3)
				&& contOpenBrackets == 0)
			break;
		pos++;
	}
	if (contOpenBrackets > 0)
		my_log_error("unbalanced brackets %s\n", tk->original_line);
	if (tk->buffer_size <= pos) {
		tk->buffer_size = MAX(2 * tk->buffer_size, pos + 1 + 64);
		MY_REALLOC(tk->buffer, tk->buffer_size, char);
	}
	if (tk->useBrackets && tk->line[0] == tk->open_bracket
			&& tk->line[pos - 1] == tk->close_bracket
			&& my_is_matched_bracket(tk->line, 0, pos - 1, tk->open_bracket,
					tk->close_bracket)) {
		strncpy(tk->buffer, tk->line + 1, pos - 2);
		tk->buffer[pos - 2] = '\0';
	} else {
		strncpy(tk->buffer, tk->line, pos);
		tk->buffer[pos] = '\0';
	}
	if (tk->line[pos] == '\0')
		tk->line = NULL;
	else
		tk->line += pos + 1;
	return tk->buffer;
}
static bool isWordChar(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9') || (c == '_');
}
static const char *my_tokenizer_nextWord(MyTokenizer *tk) {
	if (tk->line == NULL || tk->line[0] == '\0')
		return NULL;
	for (;;) {
		char c = tk->line[0];
		if (c == '\0')
			return NULL;
		else if (!isWordChar(c))
			tk->line++;
		else
			break;
	}
	int64_t pos = 0;
	for (;;) {
		char c = tk->line[pos];
		if (c == '\0' || !isWordChar(c))
			break;
		pos++;
	}
	if (tk->buffer_size <= pos) {
		tk->buffer_size = MAX(2 * tk->buffer_size, pos + 1 + 64);
		MY_REALLOC(tk->buffer, tk->buffer_size, char);
	}
	strncpy(tk->buffer, tk->line, pos);
	tk->buffer[pos] = '\0';
	if (tk->line[pos] == '\0')
		tk->line = NULL;
	else
		tk->line += pos + 1;
	return tk->buffer;
}

char *my_tokenizer_nextToken_newString(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	char *string = my_newString_string(token);
	return string;
}
bool my_tokenizer_hasNext(MyTokenizer *tk) {
	const char *lineaPrev = tk->line;
	const char *token = my_tokenizer_nextToken(tk);
	tk->line = lineaPrev;
	return (token == NULL) ? 0 : 1;
}
bool my_tokenizer_isNext(MyTokenizer *tk, const char *value) {
	const char *lineaPrev = tk->line;
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		return 0;
	if (my_string_equals(token, value)) {
		return 1;
	} else {
		tk->line = lineaPrev;
		return 0;
	}
}
double my_tokenizer_nextDouble(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		my_log_error("error parsing '%s'\n", tk->original_line);
	return my_parse_double(token);
}
double my_tokenizer_nextDouble0(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		return 0;
	return my_parse_double(token);
}
double my_tokenizer_nextFraction(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		my_log_error("error parsing '%s'\n", tk->original_line);
	return my_parse_fraction(token);
}
double my_tokenizer_nextSeconds(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		my_log_error("error parsing '%s'\n", tk->original_line);
	return my_parse_seconds(token);
}
int64_t my_tokenizer_nextInt(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		my_log_error("error parsing '%s'\n", tk->original_line);
	return my_parse_int(token);
}
int64_t my_tokenizer_nextInt0(MyTokenizer *tk) {
	const char *token = my_tokenizer_nextToken(tk);
	if (token == NULL)
		return 0;
	return my_parse_int(token);
}
const char *my_tokenizer_getCurrentTail(MyTokenizer *tk) {
	return tk->line;
}
void my_tokenizer_release(MyTokenizer *tk) {
	MY_FREE(tk->buffer);
	MY_FREE(tk);
}
void my_tokenizer_releaseValidateEnd(MyTokenizer *tk) {
	const char *s = my_tokenizer_nextToken(tk);
	if (s != NULL)
		my_log_error("error parsing %s (was not parsed %s)\n",
				tk->original_line, s);
	my_tokenizer_release(tk);
}
MyVectorString *my_tokenizer_split(MyTokenizer *tk) {
	MyVectorString *arr = my_vectorString_new();
	while (my_tokenizer_hasNext(tk)) {
		my_vectorString_add(arr, my_tokenizer_nextToken_newString(tk));
	}
	return arr;
}

MyVectorInt *my_tokenizer_splitInt(MyTokenizer *tk) {
	MyVectorInt *arr = my_vectorInt_new();
	while (my_tokenizer_hasNext(tk)) {
		const char *token = my_tokenizer_nextToken(tk);
		int64_t value = my_parse_int(token);
		my_vectorInt_add(arr, value);
	}
	return arr;
}

MyVectorDouble *my_tokenizer_splitDouble(MyTokenizer *tk) {
	MyVectorDouble *arr = my_vectorDouble_new();
	while (my_tokenizer_hasNext(tk)) {
		const char *token = my_tokenizer_nextToken(tk);
		double value = my_parse_fraction(token);
		my_vectorDouble_add(arr, value);
	}
	return arr;
}
MyVectorString *my_tokenizer_splitLine(const char* line, char token_delimiter) {
	if (line == NULL)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(line, token_delimiter);
	MyVectorString *arr = my_tokenizer_split(tk);
	my_tokenizer_releaseValidateEnd(tk);
	return arr;
}
MyVectorString *my_tokenizer_splitInWords(const char* line) {
	if (line == NULL)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(line, '\0');
	MyVectorString *arr = my_vectorString_new();
	const char *word;
	while ((word = my_tokenizer_nextWord(tk)) != NULL) {
		my_vectorString_add(arr, my_newString_string(word));
	}
	my_tokenizer_release(tk);
	return arr;
}

MyVectorInt *my_tokenizer_splitLineInt(const char* line, char token_delimiter) {
	if (line == NULL)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(line, token_delimiter);
	MyVectorInt *arr = my_tokenizer_splitInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
	return arr;
}

MyVectorDouble *my_tokenizer_splitLineDouble(const char* line,
		char token_delimiter) {
	if (line == NULL)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(line, token_delimiter);
	MyVectorDouble *arr = my_tokenizer_splitDouble(tk);
	my_tokenizer_releaseValidateEnd(tk);
	return arr;
}

