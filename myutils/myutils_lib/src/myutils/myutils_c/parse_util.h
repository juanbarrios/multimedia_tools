/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_PARSE_UTIL_H
#define MY_PARSE_UTIL_H

#include "../myutils_c.h"

void my_parse_setDefaultLocale();

uint8_t my_parse_uint8(const char *string);

int64_t my_parse_int(const char *string);
int64_t my_parse_int0(const char *string);
int64_t my_parse_int_isubFromEnd(const char *string, int64_t desdeIncluido);
int64_t my_parse_int_isubFromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaNoIncluido);
int64_t my_parse_int_csubFirstFirst(const char *string,
		char charDesdeNoIncluido, char charHastaNoIncluido);
int64_t my_parse_int_csubFirstLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
int64_t my_parse_int_csubLastFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
int64_t my_parse_int_csubLastLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido);
int64_t my_parse_int_csubLastEnd(const char *string, char charDesdeNoIncluido);
int64_t my_parse_int_csubFirstEnd(const char *string, char charDesdeNoIncluido);
int64_t my_parse_int_csubStartLast(const char *string, char charHastaNoIncluido);
int64_t my_parse_int_csubStartFirst(const char *string,
		char charHastaNoIncluido);

double my_parse_double(const char *string);
double my_parse_double0(const char *string);
double my_parse_double_isubFromEnd(const char *string, int64_t desdeIncluido);
double my_parse_double_isubFromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaNoIncluido);
double my_parse_double_csubFirstEnd(const char *string,
		char charDesdeNoIncluido);

double my_parse_fraction(const char *string);
double my_parse_seconds(const char *timestring);

bool my_parse_bool(const char *string);

#endif
