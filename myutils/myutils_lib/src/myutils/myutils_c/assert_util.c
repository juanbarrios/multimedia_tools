/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "assert_util.h"

void my_assert_equalInt(const char *description, int64_t val1, int64_t val2) {
	if (val1 != val2)
		my_log_error(
				"assert failed, \"%s\" values do not match (%"PRIi64" != %"PRIi64")\n",
				description, val1, val2);
}
void my_assert_notEqualInt(const char *description, int64_t val1, int64_t val2) {
	if (val1 == val2)
		my_log_error(
				"assert failed, \"%s\" values must not match (%"PRIi64" == %"PRIi64")\n",
				description, val1, val2);
}
void my_assert_greaterInt(const char *description, int64_t actualVal,
		int64_t refVal) {
	if (actualVal <= refVal)
		my_log_error(
				"assert failed, \"%s\" must be greater than %"PRIi64" but is %"PRIi64"\n",
				description, refVal, actualVal);
}
void my_assert_greaterEqual_int(const char *description, int64_t actualVal,
		int64_t refVal) {
	if (actualVal < refVal)
		my_log_error(
				"assert failed, \"%s\" must be greater or equal than %"PRIi64" but is %"PRIi64"\n",
				description, refVal, actualVal);
}
void my_assert_lessInt(const char *description, int64_t actualVal,
		int64_t refVal) {
	if (actualVal >= refVal)
		my_log_error(
				"assert failed, \"%s\" must be less than %"PRIi64" but is %"PRIi64"\n",
				description, refVal, actualVal);
}
void my_assert_lessEqualInt(const char *description, int64_t actualVal,
		int64_t refVal) {
	if (actualVal > refVal)
		my_log_error(
				"assert failed, \"%s\" must be less or equal than %"PRIi64" but is %"PRIi64"\n",
				description, refVal, actualVal);
}
void my_assert_indexRangeInt(const char *description, int64_t actualVal,
		int64_t arrayLength) {
	if (actualVal < 0 || actualVal >= arrayLength)
		my_log_error(
				"assert failed, array index '%s' out of bounds (index=%"PRIi64", array length=%"PRIi64")\n",
				description, actualVal, arrayLength);
}

void my_assert_equalDouble(const char *description, double val1, double val2) {
	if (val1 != val2)
		my_log_error("assert failed, \"%s\" values do not match (%lf != %lf)\n",
				description, val1, val2);
}
void my_assert_greaterDouble(const char *description, double actualVal,
		double refVal) {
	if (actualVal <= refVal)
		my_log_error(
				"assert failed, \"%s\" must be greater than %lf but is %lf\n",
				description, refVal, actualVal);
}
void my_assert_greaterEqualDouble(const char *description, double actualVal,
		double refVal) {
	if (actualVal < refVal)
		my_log_error(
				"assert failed, \"%s\" must be greater or equal than %lf but is %lf\n",
				description, refVal, actualVal);
}
void my_assert_lessDouble(const char *description, double actualVal,
		double refVal) {
	if (actualVal >= refVal)
		my_log_error("assert failed, \"%s\" must be less than %lf but is %lf\n",
				description, refVal, actualVal);
}
void my_assert_lessEqualDouble(const char *description, double actualVal,
		double refVal) {
	if (actualVal > refVal)
		my_log_error(
				"assert failed, \"%s\" must be less or equal than %lf but is %lf\n",
				description, refVal, actualVal);
}
void my_assert_isTrue(const char *description, bool val) {
	if (!val)
		my_log_error("assert failed, \"%s\" must be TRUE\n", description);
}
void my_assert_isFalse(const char *description, bool val) {
	if (val)
		my_log_error("assert failed, \"%s\" must be FALSE\n", description);
}
void my_assert_isNull(const char *description, const void *ptr) {
	if (ptr != NULL)
		my_log_error("assert failed, \"%s\" must be NULL\n", description);
}
void my_assert_notNull(const char *description, const void *ptr) {
	if (ptr == NULL)
		my_log_error("assert failed, \"%s\" must not be NULL\n", description);
}
void my_assert_notEmpty(const char *description, const char *string) {
	if (my_string_equals(string, ""))
		my_log_error("assert failed, \"%s\" must not be empty\n", description);
}
void my_assert_equalString(const char *description, const char* val1,
		const char *val2) {
	if (!my_string_equals(val1, val2))
		my_log_error("assert failed, \"%s\" values do not match (%s != %s)\n",
				description, val1, val2);
}
void my_assert_equalPtr(const char *description, const void* val1,
		const void *val2) {
	if (val1 != val2)
		my_log_error("assert failed, \"%s\" pointers do not match\n",
				description);
}
void my_assert_prefixString(const char *description, const char* string,
		const char *prefix) {
	if (string == NULL || !my_string_startsWith(string, prefix))
		my_log_error("assert failed, \"%s\" %s should start with %s\n",
				description, string, prefix);
}
void my_assert_fileExists(const char *filename) {
	if (filename == NULL || !my_io_existsFile(filename))
		my_log_error("could not locate file %s\n", filename);
}
void my_assert_fileNotExists(const char *filename) {
	if (filename == NULL)
		my_log_error("the filename is null\n");
	if (my_io_existsFile(filename))
		my_log_error("file %s already exists\n", filename);
}
void my_assert_dirExists(const char *dirname) {
	if (dirname == NULL || !my_io_existsDir(dirname))
		my_log_error("could not locate directory %s\n", dirname);
}
void my_assert_dirNotExists(const char *dirname) {
	if (dirname != NULL && my_io_existsDir(dirname))
		my_log_error("directory %s already exists\n", dirname);
}
