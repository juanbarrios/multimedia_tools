/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_ASSERT_UTIL_H
#define MY_ASSERT_UTIL_H

#include "../myutils_c.h"

void my_assert_equalInt(const char *description, int64_t val1, int64_t val2);
void my_assert_notEqualInt(const char *description, int64_t val1, int64_t val2);
void my_assert_greaterInt(const char *description, int64_t actualVal,
		int64_t refVal);
void my_assert_greaterEqual_int(const char *description, int64_t actualVal,
		int64_t refVal);
void my_assert_lessInt(const char *description, int64_t actualVal,
		int64_t refVal);
void my_assert_lessEqualInt(const char *description, int64_t actualVal,
		int64_t refVal);
void my_assert_indexRangeInt(const char *description, int64_t actualVal,
		int64_t arrayLength);

void my_assert_equalDouble(const char *description, double val1, double val2);
void my_assert_greaterDouble(const char *description, double actualVal,
		double refVal);
void my_assert_greaterEqualDouble(const char *description, double actualVal,
		double refVal);
void my_assert_lessDouble(const char *description, double actualVal,
		double refVal);
void my_assert_lessEqualDouble(const char *description, double actualVal,
		double refVal);

void my_assert_isTrue(const char *description, bool val);
void my_assert_isFalse(const char *description, bool val);
void my_assert_isNull(const char *description, const void *ptr);
void my_assert_notNull(const char *description, const void *ptr);

void my_assert_notEmpty(const char *description, const char *string);
void my_assert_equalString(const char *description, const char* val1,
		const char *val2);
void my_assert_equalPtr(const char *description, const void* val1,
		const void *val2);
void my_assert_prefixString(const char *description, const char* string,
		const char *prefix);
void my_assert_fileExists(const char *filename);
void my_assert_fileNotExists(const char *filename);
void my_assert_dirExists(const char *dirname);
void my_assert_dirNotExists(const char *dirname);

#endif
