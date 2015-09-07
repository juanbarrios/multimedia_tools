/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_STRING_UTIL_H
#define MY_STRING_UTIL_H

#include "../myutils_c.h"

bool my_string_equals(const char *string1, const char *string2);
bool my_string_equals_ignorecase(const char *string1, const char *string2);
bool my_string_startsWith(const char *string, const char *prefix);
bool my_string_startsWith_ignorecase(const char *string, const char *prefix);
bool my_string_endsWith(const char *string, const char *suffix);
bool my_string_endsWith_ignorecase(const char *string, const char *suffix);

int64_t my_string_indexOf(const char *string, const char *substring);
int64_t my_string_lastIndexOf(const char *string, const char *substring);

char *my_subStringI_fromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaSinIncluir);
char *my_subStringI_fromEnd(const char *string, int64_t desdeIncluido);
char *my_subStringI_fromTo2(const char *string, int64_t desdeIncluido,
		int64_t hastaSinIncluir,
		bool trimBlancos);

char *my_subStringC_firstFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
char *my_subStringC_firstLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
char *my_subStringC_lastFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
char *my_subStringC_lastLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
char *my_subStringC_lastEnd(const char *string, char charDesdeNoIncluido);
char *my_subStringC_firstEnd(const char *string, char charDesdeNoIncluido);
char *my_subStringC_startLast(const char *string, char charHastaNoIncluido);
char *my_subStringC_startFirst(const char *string, char charHastaNoIncluido);

void my_string_toUpper(char *string);
void my_string_replaceChar(char* string, char charSrc, char charDst);
void my_string_truncate(char* string, int64_t maxLength);
void my_string_trimDecimalZeros(char *strDecimal);

extern const char *my_newString_concat_header;
#define my_newString_concat(string1,...) (my_newString_concat_varargs(my_newString_concat_header, (char*)string1, __VA_ARGS__ , my_newString_concat_header))
char *my_newString_concat_varargs(const char *header, ...);

char *my_newString_format(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

char *my_newString_string(const char* string);
char *my_newString_replaceOne(const char* string, const char *substringToRemove,
		const char *stringToAdd);
char *my_newString_replaceAll(const char* string, const char *substringToRemove,
		const char *stringToAdd);
char *my_newString_escapeNoAlphaNum(const char *string);

char *my_newString_int(int64_t value);
char *my_newString_float(float value);
char *my_newString_double(double value);
char *my_newString_doubleDec(double value, int64_t maxDecimals);
char *my_newString_bool(bool value);
char *my_newString_hhmmssfff(double seconds);
char *my_newString_hhmmss(double seconds);
char *my_newString_hhmmss_forced(double seconds);
char *my_newString_diskSpace(int64_t numBytes);

char *my_newString_arrayUint8(uint8_t *array, int64_t dim, char separator);
char *my_newString_arrayInt(int64_t *array, int64_t dim, char separator);
char *my_newString_arrayFloat(float *array, int64_t dim, char separator);
char *my_newString_arrayDouble(double *array, int64_t dim, char separator);

int my_string_strcmpFilenames(const char *f1, const char *f2);

#endif
