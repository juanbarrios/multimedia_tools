/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "string_util.h"

bool my_string_equals(const char *string1, const char *string2) {
	if (string1 == NULL || string2 == NULL)
		return false;
	return strcmp(string1, string2) == 0;
}
bool my_string_equals_ignorecase(const char *string1, const char *string2) {
	if (string1 == NULL || string2 == NULL)
		return false;
	return strcasecmp(string1, string2) == 0;
}
bool my_string_startsWith(const char *string, const char *prefix) {
	if (string == NULL || prefix == NULL)
		return false;
	return strncmp(string, prefix, strlen(prefix)) == 0;
}
bool my_string_startsWith_ignorecase(const char *string, const char *prefix) {
	if (string == NULL || prefix == NULL)
		return false;
	return strncasecmp(string, prefix, strlen(prefix)) == 0;
}
bool my_string_endsWith(const char *string, const char *suffix) {
	if (string == NULL || suffix == NULL)
		return false;
	int64_t l1 = strlen(string);
	int64_t l2 = strlen(suffix);
	if (l1 < l2)
		return false;
	return strcmp(string + (l1 - l2), suffix) == 0;
}
bool my_string_endsWith_ignorecase(const char *string, const char *suffix) {
	if (string == NULL || suffix == NULL)
		return false;
	int64_t l1 = strlen(string);
	int64_t l2 = strlen(suffix);
	if (l1 < l2)
		return false;
	return strcasecmp(string + (l1 - l2), suffix) == 0;
}

int64_t my_string_indexOf(const char *string, const char *substring) {
	char* pos = strstr(string, substring);
	return (pos == NULL) ? -1 : pos - string;
}
int64_t my_string_lastIndexOf(const char *string, const char *substring) {
	const char* pos = string;
	int64_t lastPos = -1;
	while ((pos = strstr(pos, substring)) != NULL) {
		lastPos = pos - string;
		pos++;
	}
	return lastPos;
}
//hay que hacer MY_FREE de lo retornado
char *my_subStringI_fromTo(const char *string, int64_t desdeIncluido,
		int64_t hastaSinIncluir) {
	return my_subStringI_fromTo2(string, desdeIncluido, hastaSinIncluir, 0);
}
//hay que hacer MY_FREE de lo retornado
char *my_subStringI_fromEnd(const char *string, int64_t desdeIncluido) {
	return my_subStringI_fromTo2(string, desdeIncluido, -1, 0);
}
//hay que hacer MY_FREE de lo retornado
//my_subStringI_fromTo y elimina los blancos iniciales y finales del my_subStringI_fromTo
char *my_subStringI_fromTo2(const char *string, int64_t desdeIncluido,
		int64_t hastaSinIncluir,
		bool trimBlancos) {
	int64_t n = strlen(string);
	if (hastaSinIncluir < 0)
		hastaSinIncluir = n;
	if (desdeIncluido < 0 || hastaSinIncluir > n
			|| desdeIncluido > hastaSinIncluir)
		my_log_error(
				"out of bounds largo=%"PRIi64": [%"PRIi64",%"PRIi64"]\nstring='%s'\n",
				n, desdeIncluido, hastaSinIncluir, string);
	if (trimBlancos) {
		while (desdeIncluido < hastaSinIncluir && string[desdeIncluido] == ' ')
			desdeIncluido++;
		while (desdeIncluido < hastaSinIncluir
				&& string[hastaSinIncluir - 1] == ' ')
			hastaSinIncluir--;
	}
	char *st = MY_MALLOC_NOINIT(hastaSinIncluir - desdeIncluido + 1, char);
	if (desdeIncluido < hastaSinIncluir)
		strncpy(st, string + desdeIncluido, hastaSinIncluir - desdeIncluido);
	st[hastaSinIncluir - desdeIncluido] = '\0';
	return st;
}
//retorna desde la primera aparicion del char desde, o del inicio si no esta,
//hasta la primera aparicion del char hasta despues del desde.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_firstFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	const char *d = strchr(string, charDesdeNoIncluido);
	d = (d == NULL) ? string : d + 1;
	const char *h = strchr(d, charHastaNoIncluido);
	if (h == NULL)
		return my_subStringI_fromTo(string, d - string, strlen(string));
	else
		return my_subStringI_fromTo(string, d - string, h - string);
}
//retorna desde la primera aparicion del char desde, o del inicio si no esta,
//hasta la ultima aparicion del char hasta despues del desde.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_firstLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	const char *d = strchr(string, charDesdeNoIncluido);
	d = (d == NULL) ? string : d + 1;
	const char *h = strrchr(d, charHastaNoIncluido);
	if (h == NULL)
		return my_subStringI_fromTo(string, d - string, strlen(string));
	else
		return my_subStringI_fromTo(string, d - string, h - string);
}
//retorna desde la ultima aparicion del char desde, o del inicio si no esta,
//hasta la ultima aparicion del char hasta despues del desde.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_lastFirst(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	const char *d = strrchr(string, charDesdeNoIncluido);
	d = (d == NULL) ? string : d + 1;
	const char *h = strchr(d, charHastaNoIncluido);
	if (h == NULL)
		return my_subStringI_fromTo(string, d - string, strlen(string));
	else
		return my_subStringI_fromTo(string, d - string, h - string);
}
//retorna desde la ultima aparicion del char desde, o del inicio si no esta,
//hasta la ultima aparicion del char hasta despues del desde.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_lastLast(const char *string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	const char *d = strrchr(string, charDesdeNoIncluido);
	d = (d == NULL) ? string : d + 1;
	const char *h = strrchr(d, charHastaNoIncluido);
	if (h == NULL)
		return my_subStringI_fromTo(string, d - string, strlen(string));
	else
		return my_subStringI_fromTo(string, d - string, h - string);
}
//retorna desde la ultima aparicion del char desde, o del inicio si no esta.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_lastEnd(const char *string, char charDesdeNoIncluido) {
	char *d = strrchr(string, charDesdeNoIncluido);
	if (d == NULL)
		return my_newString_string(string);
	else
		return my_subStringI_fromTo(string, d + 1 - string, strlen(string));
}
//retorna desde la primera aparicion del char desde, o del inicio si no esta.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_firstEnd(const char *string, char charDesdeNoIncluido) {
	char *d = strchr(string, charDesdeNoIncluido);
	if (d == NULL)
		return my_newString_string(string);
	else
		return my_subStringI_fromTo(string, d + 1 - string, strlen(string));
}
//retorna desde el inicio hasta la ultima aparicion del char hasta.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_startLast(const char *string, char charHastaNoIncluido) {
	char *h = strrchr(string, charHastaNoIncluido);
	if (h == NULL)
		return my_newString_string(string);
	else
		return my_subStringI_fromTo(string, 0, h - string);
}
//retorna desde el inicio hasta la primera aparicion del char hasta.
//hay que hacer MY_FREE de lo retornado
char *my_subStringC_startFirst(const char *string, char charHastaNoIncluido) {
	char *h = strchr(string, charHastaNoIncluido);
	if (h == NULL)
		return my_newString_string(string);
	else
		return my_subStringI_fromTo(string, 0, h - string);
}
//hay que hacer MY_FREE de lo retornado
char* my_newString_string(const char* string) {
	if (string == NULL)
		return NULL;
	char* s = MY_MALLOC(strlen(string) + 1, char);
	strcpy(s, string);
	return s;
}
const char* my_newString_concat_header = "my_newString_concat_header";

//hay que hacer MY_FREE de lo retornado
char *my_newString_concat_varargs(const char *header, ...) {
	//concatenar hasta encontrar el mismo argumento inicial
	if (header != my_newString_concat_header)
		my_log_error("error in my_newString_concat\n");
	va_list arguments;
	va_start(arguments, header);
	int64_t largo = 0;
	for (;;) {
		char *st = va_arg(arguments, char*);
		if (st == header)
			break;
		if (st != NULL)
			largo += strlen(st);
	}
	va_end(arguments);
	int64_t pos = 0;
	char *string = MY_MALLOC_NOINIT(largo + 1, char);
	va_start(arguments, header);
	for (;;) {
		char *st = va_arg(arguments, char*);
		if (st == header)
			break;
		if (st != NULL) {
			for (int64_t k = 0; st[k] != '\0'; ++k)
				string[pos++] = st[k];
		}
	}
	va_end(arguments);
	string[pos] = '\0';
	return string;
}
char *my_newString_format(const char *format, ...) {
	va_list args;
	int64_t buffSize = 250;
	char *strBuff = NULL;
	for (;;) {
		MY_REALLOC(strBuff, buffSize, char);
		//vsnprintf:
		// if the number of characters to write is greater than count, these
		// functions return -1 indicating that output has been truncated.
		//vsnprintf:
		//If the output was truncated the return value is the number of characters
		//which would have been written to the final string if enough space had been available.
		va_start(args, format);
		int64_t n = vsnprintf(strBuff, buffSize - 1, format, args);
		va_end(args);
		if (n < 0) {
			buffSize = 2 * buffSize + 1;
		} else if (n >= (buffSize - 1)) {
			buffSize = n + 2;
		} else {
			break;
		}
	}
	return strBuff;
}
void my_string_toUpper(char *string) {
	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] >= 'a' && string[i] <= 'z')
			string[i] = string[i] - 'a' + 'A';
	}
}
void my_string_replaceChar(char* string, char charSrc, char charDst) {
	for (size_t i = 0; string[i] != '\0'; i++) {
		if (string[i] == charSrc)
			string[i] = charDst;
	}
}
void my_string_truncate(char* string, int64_t maxLength) {
	int64_t n = strlen(string);
	if (n > maxLength)
		string[maxLength] = '\0';
}
//hay que hacer free de lo retornado
char *my_newString_replaceOne(const char* string, const char *substringToRemove,
		const char *stringToAdd) {
	int64_t pos = my_string_indexOf(string, substringToRemove);
	if (pos < 0)
		return my_newString_string(string);
	int64_t len = strlen(substringToRemove);
	char *pref = my_subStringI_fromTo(string, 0, pos);
	char *st = my_newString_concat(pref, stringToAdd, string + pos + len);
	MY_FREE(pref);
	return st;
}
char *my_newString_replaceAll(const char* string, const char *substringToRemove,
		const char *stringToAdd) {
	int64_t pos, len = strlen(substringToRemove);
	MyStringBuffer *sb = my_stringbuf_new();
	while ((pos = my_string_indexOf(string, substringToRemove)) >= 0) {
		char *pref = my_subStringI_fromTo(string, 0, pos);
		my_stringbuf_appendString(sb, pref);
		MY_FREE(pref);
		my_stringbuf_appendString(sb, stringToAdd);
		string += pos + len;
	}
	my_stringbuf_appendString(sb, string);
	return my_stringbuf_releaseReturnBuffer(sb);
}
char *my_newString_escapeNoAlphaNum(const char *string) {
	MyStringBuffer *sb = my_stringbuf_new();
	char c;
	for (size_t i = 0; (c = string[i]) != '\0'; i++) {
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'
				|| c == ',') {
			my_stringbuf_appendChar(sb, c);
		} else {
			char *s = my_newString_format("%%%02X", c & 0xff);
			my_stringbuf_appendString(sb, s);
			free(s);
		}
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}

//hay que hacer free de lo retornado
char *my_newString_int(int64_t value) {
	return my_newString_format("%"PRIi64"", value);
}
void my_string_trimDecimalZeros(char *strDecimal) {
	int64_t posPunto = my_string_indexOf(strDecimal, ".");
	if (posPunto < 0)
		return;
	int64_t posU = strlen(strDecimal);
	while (posU > 1 && strDecimal[posU - 1] == '0') {
		strDecimal[posU - 1] = '\0';
		posU--;
	}
	if (strDecimal[posU - 1] == '.')
		strDecimal[posU - 1] = '\0';
}
//hay que hacer free de lo retornado
char *my_newString_float(float value) {
	if (value == ((int64_t) value)) {
		return my_newString_format("%"PRIi64"", (int64_t) value);
	} else {
		char *str = my_newString_format("%1.7f", value);
		my_string_trimDecimalZeros(str);
		return str;
	}
}
//hay que hacer free de lo retornado
char *my_newString_double(double value) {
	return my_newString_doubleDec(value, -1);
}
char *my_newString_doubleDec(double value, int64_t maxDecimals) {
	if (isnan(value))
		return my_newString_string("NAN");
	else if (value == DBL_MAX || (isinf(value) && value > 0))
		return my_newString_string("INF");
	else if (value == -DBL_MAX || (isinf(value) && value < 0))
		return my_newString_string("-INF");
	else if (value == ((int64_t) value))
		return my_newString_format("%"PRIi64"", (int64_t) value);
	else if (maxDecimals == 0)
		return my_newString_format("%"PRIi64"", my_math_round_int(value));
	else {
		char *str;
		if (maxDecimals == 1)
			str = my_newString_format("%1.1lf", value);
		else if (maxDecimals == 2)
			str = my_newString_format("%1.2lf", value);
		else if (maxDecimals == 3)
			str = my_newString_format("%1.3lf", value);
		else if (maxDecimals == 4)
			str = my_newString_format("%1.4lf", value);
		else if (maxDecimals == 5)
			str = my_newString_format("%1.5lf", value);
		else if (maxDecimals == 6)
			str = my_newString_format("%1.6lf", value);
		else if (maxDecimals == 7)
			str = my_newString_format("%1.7lf", value);
		else if (maxDecimals == 8)
			str = my_newString_format("%1.8lf", value);
		else if (maxDecimals == 9)
			str = my_newString_format("%1.9lf", value);
		else if (maxDecimals == 10)
			str = my_newString_format("%1.10lf", value);
		else if (maxDecimals == 11)
			str = my_newString_format("%1.11lf", value);
		else if (maxDecimals == 12)
			str = my_newString_format("%1.12lf", value);
		else if (maxDecimals == 13)
			str = my_newString_format("%1.13lf", value);
		else if (maxDecimals == 14)
			str = my_newString_format("%1.14lf", value);
		else
			str = my_newString_format("%1.15lf", value);
		my_string_trimDecimalZeros(str);
		return str;
	}
}
char *my_newString_bool(bool value) {
	return my_newString_string(value ? "TRUE" : "FALSE");
}
static char *internal_tohhmmssfff(double seconds, bool forceHH, bool forceMM,
bool withFFF) {
	bool isNeg = (seconds < 0);
	int64_t secs = my_math_floor_int(fabs(seconds));
	int64_t hours = secs / 3600;
	secs -= hours * 3600;
	int64_t mins = secs / 60;
	secs -= mins * 60;
	MyStringBuffer *sb = my_stringbuf_new();
	if (isNeg)
		my_stringbuf_appendChar(sb, '-');
	bool flag0 = false;
	if (hours > 0 || forceHH) {
		if (hours < 10 && forceHH)
			my_stringbuf_appendChar(sb, '0');
		my_stringbuf_appendInt(sb, hours);
		my_stringbuf_appendChar(sb, ':');
		flag0 = true;
	}
	if (hours > 0 || mins > 0 || forceMM) {
		if (mins < 10)
			my_stringbuf_appendChar(sb, '0');
		my_stringbuf_appendInt(sb, mins);
		my_stringbuf_appendChar(sb, ':');
		flag0 = true;
	}
	double fraction = my_math_fractionalPart(seconds);
	if (secs < 10 && flag0)
		my_stringbuf_appendChar(sb, '0');
	if (fraction > 0 && withFFF) {
		char *st = my_newString_doubleDec(fraction + secs, 3);
		my_stringbuf_appendString(sb, st);
		free(st);
	} else {
		my_stringbuf_appendInt(sb, secs);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
char *my_newString_hhmmssfff(double seconds) {
	return internal_tohhmmssfff(seconds, false, true, true);
}
char *my_newString_hhmmss(double seconds) {
	return internal_tohhmmssfff(seconds, false, true, false);
}
char *my_newString_hhmmss_forced(double seconds) {
	return internal_tohhmmssfff(seconds, true, true, false);
}
static const int64_t KB = 1024;
static const int64_t MB = 1024 * 1024;
static const int64_t GB = 1024 * 1024 * 1024;
char *my_newString_diskSpace(int64_t numBytes) {
	if (numBytes > GB)
		return my_newString_format("%1.1lf GB", numBytes / (double) GB);
	else if (numBytes > MB)
		return my_newString_format("%1.1lf MB", numBytes / (double) MB);
	else
		return my_newString_format("%1.1lf KB", numBytes / (double) KB);
}

char *my_newString_arrayUint8(uint8_t *array, int64_t dim, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < dim; ++i) {
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendInt(sb, array[i]);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
char *my_newString_arrayInt(int64_t *array, int64_t dim, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < dim; ++i) {
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendInt(sb, array[i]);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
char *my_newString_arrayFloat(float *array, int64_t dim, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < dim; ++i) {
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendFloat(sb, array[i]);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
char *my_newString_arrayDouble(double *array, int64_t dim, char separator) {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < dim; ++i) {
		if (i > 0)
			my_stringbuf_appendChar(sb, separator);
		my_stringbuf_appendDouble(sb, array[i]);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}

void priv_copiar_sgteParte(const char *str, char *buff, int64_t *desde,
		int64_t numeros) {
	int64_t esNum, posStr = *desde, posBuff = 0;
	while (str[posStr] != '\0') {
		esNum = str[posStr] >= '0' && str[posStr] <= '9';
		if ((numeros && !esNum) || (!numeros && esNum))
			break;
		buff[posBuff] = str[posStr];
		posBuff++;
		posStr++;
	}
	buff[posBuff] = '\0';
	*desde = posStr;
}
int my_string_strcmpFilenames(const char *f1, const char *f2) {
	char *st1, *st2;
	if ((st1 = strrchr(f1, '/')) != NULL && (st2 = strrchr(f2, '/')) != NULL) {
		int64_t pos1 = st1 - f1;
		int64_t pos2 = st2 - f2;
		if (pos1 == pos2 && strncasecmp(f1, f2, pos1) == 0) {
			f1 = f1 + pos1 + 1;
			f2 = f2 + pos1 + 1;
		}
	}
	int64_t pos1 = 0, pos2 = 0;
	int64_t l1 = strlen(f1);
	int64_t l2 = strlen(f2);
	char buff1[l1 + 1], buff2[l2 + 1];
	while (pos1 < l1 && pos2 < l2) {
		priv_copiar_sgteParte(f1, buff1, &pos1, 0);
		priv_copiar_sgteParte(f2, buff2, &pos2, 0);
		int64_t n = strcasecmp(buff1, buff2);
		if (n != 0)
			return n;
		priv_copiar_sgteParte(f1, buff1, &pos1, 1);
		priv_copiar_sgteParte(f2, buff2, &pos2, 1);
		int64_t i1 = (buff1[0] == '\0') ? -1 : my_parse_int(buff1);
		int64_t i2 = (buff2[0] == '\0') ? -1 : my_parse_int(buff2);
		if (i1 != i2)
			return i1 > i2 ? 1 : -1;
	}
	return strcmp(f1, f2);
}
