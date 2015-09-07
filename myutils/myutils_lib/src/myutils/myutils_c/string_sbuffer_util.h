/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_STRING_SBUFFER_UTIL_H
#define MY_STRING_SBUFFER_UTIL_H

#include "../myutils_c.h"

typedef struct MyStringBuffer MyStringBuffer;

MyStringBuffer *my_stringbuf_new();
void my_stringbuf_appendString(MyStringBuffer *sb, const char* text);
void my_stringbuf_appendChar(MyStringBuffer *sb, char character);
void my_stringbuf_appendInt(MyStringBuffer *sb, int64_t number);
void my_stringbuf_appendFloat(MyStringBuffer *sb, float number);
void my_stringbuf_appendDouble(MyStringBuffer *sb, double number);
void my_stringbuf_appendDoubleRound(MyStringBuffer *sb, double number, int64_t maxDecimals) ;
const char *my_stringbuf_getCurrentBuffer(MyStringBuffer *sb);
void my_stringbuf_release(MyStringBuffer *sb);
char *my_stringbuf_releaseReturnBuffer(MyStringBuffer *sb);

#endif
