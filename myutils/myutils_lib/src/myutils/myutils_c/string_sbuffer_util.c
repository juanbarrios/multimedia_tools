/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "string_sbuffer_util.h"

struct MyStringBuffer {
	char *buffer;
	size_t end_position;
	size_t buffer_size;
};
MyStringBuffer *my_stringbuf_new() {
	MyStringBuffer *sb = MY_MALLOC(1, MyStringBuffer);
	return sb;
}
void my_stringbuf_appendString(MyStringBuffer *sb, const char* text) {
	if (text == NULL)
		text = "null";
	size_t length = strlen(text);
	if (length == 0)
		return;
	if (sb->end_position + length >= sb->buffer_size) {
		sb->buffer_size = MAX(2 * sb->buffer_size,
				sb->end_position + length + 64);
		MY_REALLOC(sb->buffer, sb->buffer_size, char);
	}
	strcpy(sb->buffer + sb->end_position, text);
	sb->end_position += length;
}
void my_stringbuf_appendChar(MyStringBuffer *sb, char character) {
	if (sb->end_position + 1 >= sb->buffer_size) {
		sb->buffer_size = MAX(2 * sb->buffer_size, 64);
		MY_REALLOC(sb->buffer, sb->buffer_size, char);
	}
	sb->buffer[sb->end_position] = character;
	sb->buffer[sb->end_position + 1] = '\0';
	sb->end_position++;
}
void my_stringbuf_appendInt(MyStringBuffer *sb, int64_t number) {
	char *st = my_newString_int(number);
	my_stringbuf_appendString(sb, st);
	free(st);
}
void my_stringbuf_appendFloat(MyStringBuffer *sb, float number) {
	char *st = my_newString_float(number);
	my_stringbuf_appendString(sb, st);
	free(st);
}
void my_stringbuf_appendDouble(MyStringBuffer *sb, double number) {
	char *st = my_newString_double(number);
	my_stringbuf_appendString(sb, st);
	free(st);
}
void my_stringbuf_appendDoubleRound(MyStringBuffer *sb, double number,
		int64_t maxDecimals) {
	char *st = my_newString_doubleDec(number, maxDecimals);
	my_stringbuf_appendString(sb, st);
	free(st);
}
/*puntero al buffer interno. este puntero puede cambiar luego
 * de un append..() por lo que cualquier referencia retornada
 * queda invalida luego de un append.*/
const char *my_stringbuf_getCurrentBuffer(MyStringBuffer *sb) {
	return sb->buffer;
}
void my_stringbuf_release(MyStringBuffer *sb) {
	free(sb->buffer);
	free(sb);
}
//must free the returned string
char *my_stringbuf_releaseReturnBuffer(MyStringBuffer *sb) {
	char *st = sb->buffer;
	free(sb);
	return st;
}
