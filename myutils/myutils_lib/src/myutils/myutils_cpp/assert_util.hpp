/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_ASSERT_UTIL_HPP
#define MY_ASSERT_UTIL_HPP

#include "../myutils_cpp.hpp"

namespace my {
class assert {
public:
	static void equalInt(std::string description, long long val1, long long val2);
	static void greaterInt(std::string description, long long actualVal,
			long long refVal);
	static void greaterEqual_int(std::string description, long long actualVal,
			long long refVal);
	static void lessInt(std::string description, long long actualVal,
			long long refVal);
	static void lessEqualInt(std::string description, long long actualVal,
			long long refVal);
	static void indexRangeInt(std::string description, long long actualVal,
			long long arrayLength);

	static void equalDouble(std::string description, double val1, double val2);
	static void greaterDouble(std::string description, double actualVal,
			double refVal);
	static void greaterEqualDouble(std::string description, double actualVal,
			double refVal);
	static void lessDouble(std::string description, double actualVal,
			double refVal);
	static void lessEqualDouble(std::string description, double actualVal,
			double refVal);

	static void isTrue(std::string description, bool val);
	static void isFalse(std::string description, bool val);
	static void isNull(std::string description, const void *ptr);
	static void notNull(std::string description, const void *ptr);

	static void notEmpty(std::string description, std::string string);
	static void equalString(std::string description, std::string  val1,
			std::string val2);
	static void equalPtr(std::string description, const void* val1,
			const void *val2);
	static void prefixString(std::string description, std::string  string,
			std::string prefix);
	static void fileExists(std::string filename);
	static void fileNotExists(std::string filename);
	static void dirExists(std::string dirname);
	static void dirNotExists(std::string dirname);
};
}
#endif
