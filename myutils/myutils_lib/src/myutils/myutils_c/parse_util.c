/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "parse_util.h"
#include <locale.h>

MY_MUTEX_NEWSTATIC(priv_mutex_locale);

void my_parse_setDefaultLocale() {
	MY_MUTEX_LOCK(priv_mutex_locale);
	setlocale(LC_ALL, "POSIX");
	MY_MUTEX_UNLOCK(priv_mutex_locale);
}

uint8_t my_parse_uint8(const char *string) {
	int64_t n = my_parse_int(string);
	if (n <= 0)
		return 0;
	if (n >= 255)
		return 255;
	return (uint8_t) n;
}
int64_t my_parse_int(const char *string) {
	my_assert_notNull("string", string);
	if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		my_log_error("error my_parse_int: \"%s\"\n", string);
	errno = 0;
	char *endptr = NULL;
	long long val = strtoll(string, &endptr, 10);
	if (endptr == string || *endptr != '\0' || errno != 0)
		my_log_error("error my_parse_int: \"%s\"\n", string);
	int64_t myval = (int64_t) val;
	if (myval != val)
		my_log_error("error my_parse_int: \"%s\"\n", string);
	return myval;
//return atoll(string);
}
int64_t my_parse_int0(const char *string) {
	if (string == NULL || string[0] == '\0')
		return 0;
	return my_parse_int(string);
}
//entero entre posicion desdeIncluido y el fin
int64_t my_parse_int_isubFromEnd(const char *string, int64_t desdeIncluido) {
	my_assert_notNull("string", string);
	const char *st2 = string + desdeIncluido;
	return my_parse_int(st2);
}
//entero entre posicion desdeIncluido y el fin
int64_t my_parse_int_isubFromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaNoIncluido) {
	my_assert_notNull("string", string);
	char *st2 = my_subStringI_fromTo(string, desdeIncluido, hastaNoIncluido);
	int64_t val = my_parse_int(st2);
	MY_FREE(st2);
	return val;
}
int64_t my_parse_int_csubFirstFirst(const char *string,
		char charDesdeNoIncluido, char charHastaNoIncluido) {
	char *st = my_subStringC_firstFirst(string, charDesdeNoIncluido,
			charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubFirstLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	char *st = my_subStringC_firstLast(string, charDesdeNoIncluido,
			charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubLastFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	char *st = my_subStringC_lastFirst(string, charDesdeNoIncluido,
			charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubLastLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	char *st = my_subStringC_lastLast(string, charDesdeNoIncluido,
			charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubLastEnd(const char *string, char charDesdeNoIncluido) {
	char *st = my_subStringC_lastEnd(string, charDesdeNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubFirstEnd(const char *string, char charDesdeNoIncluido) {
	char *st = my_subStringC_firstEnd(string, charDesdeNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubStartLast(const char *string, char charHastaNoIncluido) {
	char *st = my_subStringC_startLast(string, charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}
int64_t my_parse_int_csubStartFirst(const char *string,
		char charHastaNoIncluido) {
	char *st = my_subStringC_startFirst(string, charHastaNoIncluido);
	int64_t val = my_parse_int(st);
	MY_FREE(st);
	return val;
}

double my_parse_double(const char *string) {
	my_assert_notNull("string", string);
	if (my_string_equals_ignorecase(string, "INF")
			|| my_string_equals_ignorecase(string, "NAN"))
		return DBL_MAX;
	else if (my_string_equals_ignorecase(string, "-INF"))
		return -DBL_MAX;
	else if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		my_log_error("error my_parse_double: \"%s\"\n", string);
	errno = 0;
	char *endptr = NULL;
	double val = strtod(string, &endptr);
	if (endptr == string || *endptr != '\0' || errno != 0)
		my_log_error("error my_parse_double: \"%s\"\n", string);
	return val;
//return atof(string);
}
double my_parse_double0(const char *string) {
	if (string == NULL || string[0] == '\0')
		return 0;
	return my_parse_double(string);
}

double my_parse_double_isubFromEnd(const char *string, int64_t desdeIncluido) {
	my_assert_notNull("string", string);
	const char *st2 = string + desdeIncluido;
	return my_parse_double(st2);
}
double my_parse_double_isubFromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaNoIncluido) {
	my_assert_notNull("string", string);
	char *st2 = my_subStringI_fromTo(string, desdeIncluido, hastaNoIncluido);
	double val = my_parse_double(st2);
	MY_FREE(st2);
	return val;
}
double my_parse_double_csubFirstEnd(const char *string,
		char charDesdeNoIncluido) {
	char *st = my_subStringC_firstEnd(string, charDesdeNoIncluido);
	double val = my_parse_double(st);
	MY_FREE(st);
	return val;
}

double my_parse_fraction(const char *string) {
	my_assert_notNull("string", string);
	if (strchr(string, '/') == NULL)
		return my_parse_double(string);
	char *num = my_subStringC_startLast(string, '/');
	char *den = my_subStringC_lastEnd(string, '/');
	double frac = my_parse_double(num) / my_parse_double(den);
	if (!MY_IS_REAL(frac))
		my_log_error("error my_parse_fraction %s\n", string);
	MY_FREE(num);
	MY_FREE(den);
	return frac;
}
//seconds in format [[hh:]mm:]ss.d
double my_parse_seconds(const char *timestring) {
	if (timestring == NULL)
		return 0;
	MyTokenizer *tk = my_tokenizer_new(timestring, ':');
	char *p1 = my_tokenizer_nextToken_newString(tk);
	char *p2 = my_tokenizer_nextToken_newString(tk);
	char *p3 = my_tokenizer_nextToken_newString(tk);
	my_tokenizer_releaseValidateEnd(tk);
	double secs = 0;
	if (p1 != NULL && p2 != NULL && p3 != NULL)
		secs = my_parse_int(p1) * 3600 + my_parse_int(p2) * 60
				+ my_parse_double(p3);
	else if (p1 != NULL && p2 != NULL)
		secs = my_parse_int(p1) * 60 + my_parse_double(p2);
	else if (p1 != NULL)
		secs = my_parse_double(p1);
	free(p1);
	free(p2);
	free(p3);
	return secs;
}
bool my_parse_bool(const char *string) {
	if (string == NULL) {
		return false;
	} else if (my_string_equals_ignorecase(string, "true")
			|| my_string_equals_ignorecase(string, "yes")
			|| my_string_equals_ignorecase(string, "y")
			|| my_string_equals_ignorecase(string, "1")) {
		return true;
	} else if (my_string_equals_ignorecase(string, "false")
			|| my_string_equals_ignorecase(string, "no")
			|| my_string_equals_ignorecase(string, "n")
			|| my_string_equals_ignorecase(string, "0")) {
		return false;
	} else {
		my_log_error(
				"unknown boolean value \"%s\". Valid values are true/false, yes/no, y/n, 1/0.\n",
				string);
		return false;
	}
}
