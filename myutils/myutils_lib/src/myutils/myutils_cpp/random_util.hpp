/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_RANDOM_UTIL_HPP
#define MY_RANDOM_UTIL_HPP

#include "../myutils_cpp.hpp"

namespace my {
class random {
public:

	static void setSeed(long long seed);

	static long long intValue(long long minValueIncluded,
			long long maxValueNotIncluded);
	static std::vector<long long> intList(long long minValueIncluded,
			long long maxValueNotIncluded, size_t size);

	static double doubleValue(double minValueIncluded,
			double maxValueNotIncluded);
	static std::vector<double> doubleList(double minValueIncluded,
			double maxValueNotIncluded, size_t size);

	static std::vector<long long> permutation(long long minValueIncluded,
			long long maxValueNotIncluded);

	static std::vector<long long> intList_noRepetitions(
			long long minValueIncluded, long long maxValueNotIncluded,
			size_t size);

};
}

#endif
