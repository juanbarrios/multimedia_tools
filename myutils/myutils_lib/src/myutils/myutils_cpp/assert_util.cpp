/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "assert_util.hpp"
#include <stdexcept>
#include "../myutils_c.h"

using namespace my;

void assert::equalInt(std::string description, long long val1, long long val2) {
	my_assert_equalInt(description.c_str(), val1, val2);
}
void assert::greaterInt(std::string description, long long actualVal,
		long long refVal) {
	my_assert_greaterInt(description.c_str(), actualVal, refVal);
}
void assert::greaterEqual_int(std::string description, long long actualVal,
		long long refVal) {
	my_assert_greaterEqual_int(description.c_str(), actualVal, refVal);
}
void assert::lessInt(std::string description, long long actualVal,
		long long refVal) {
	my_assert_lessInt(description.c_str(), actualVal, refVal);
}
void assert::lessEqualInt(std::string description, long long actualVal,
		long long refVal) {
	my_assert_lessEqualInt(description.c_str(), actualVal, refVal);
}
void assert::indexRangeInt(std::string description, long long actualVal,
		long long arrayLength) {
	my_assert_indexRangeInt(description.c_str(), actualVal, arrayLength);
}

void assert::equalDouble(std::string description, double val1, double val2) {
	my_assert_equalDouble(description.c_str(), val1, val2);
}
void assert::greaterDouble(std::string description, double actualVal,
		double refVal) {
	my_assert_greaterDouble(description.c_str(), actualVal, refVal);
}
void assert::greaterEqualDouble(std::string description, double actualVal,
		double refVal) {
	my_assert_greaterEqualDouble(description.c_str(), actualVal, refVal);
}
void assert::lessDouble(std::string description, double actualVal,
		double refVal) {
	my_assert_lessDouble(description.c_str(), actualVal, refVal);
}
void assert::lessEqualDouble(std::string description, double actualVal,
		double refVal) {
	my_assert_lessEqualDouble(description.c_str(), actualVal, refVal);
}

void assert::isTrue(std::string description, bool val) {
	my_assert_isTrue(description.c_str(), val);
}
void assert::isFalse(std::string description, bool val) {
	my_assert_isFalse(description.c_str(), val);
}
void assert::isNull(std::string description, const void *ptr) {
	my_assert_isNull(description.c_str(), ptr);
}
void assert::notNull(std::string description, const void *ptr) {
	my_assert_notNull(description.c_str(), ptr);
}

void assert::notEmpty(std::string description, std::string string) {
	if (string == "")
		throw std::runtime_error(
				"assert failed, '" + description + "' must not be empty");
}
void assert::equalString(std::string description, std::string val1,
		std::string val2) {
	if (val1 != val2)
		throw std::runtime_error(
				"assert failed, '" + description + "' values do not match ("
						+ val1 + " != " + val2 + ")");
}
void assert::equalPtr(std::string description, const void* val1,
		const void *val2) {
	my_assert_equalPtr(description.c_str(), val1, val2);
}
void assert::prefixString(std::string description, std::string string,
		std::string prefix) {
	my_assert_prefixString(description.c_str(), string.c_str(), prefix.c_str());
}

void assert::fileExists(std::string filename) {
	my_assert_fileExists(filename.c_str());
}
void assert::fileNotExists(std::string filename) {
	my_assert_fileNotExists(filename.c_str());
}
void assert::dirExists(std::string dirname) {
	my_assert_dirExists(dirname.c_str());
}
void assert::dirNotExists(std::string dirname) {
	my_assert_dirNotExists(dirname.c_str());
}

