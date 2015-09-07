/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_PARSE_UTIL_HPP
#define MY_PARSE_UTIL_HPP

#include "../myutils_cpp.hpp"

namespace my {

class parse {
public:

	static void setDefaultLocale();

	static int stringToInt(std::string string, bool acceptEmptyString = false,
			int valueEmptyString = 0);
	static long long stringToLong(std::string string, bool acceptEmptyString =
			false, long long valueEmptyString = 0);
	static double stringToDouble(std::string string, bool acceptEmptyString =
			false, double valueEmptyString = 0);
	static float stringToFloat(std::string string, bool acceptEmptyString =
			false, float valueEmptyString = 0);

	static std::vector<int> stringListToInt(
			const std::vector<std::string> &list,
			bool acceptEmptyString = false, int valueEmptyString = 0);
	static std::vector<long long> stringListToLong(
			const std::vector<std::string> &list,
			bool acceptEmptyString = false, long long valueEmptyString = 0);
	static std::vector<double> stringListToDouble(
			const std::vector<std::string> &list,
			bool acceptEmptyString = false, double valueEmptyString = 0);
	static std::vector<float> stringListToFloat(
			const std::vector<std::string> &list,
			bool acceptEmptyString = false, float valueEmptyString = 0);

	static double fractionToDouble(std::string fraction_string);
	static double timestampToDouble(std::string timestamp_string);

	static bool stringToBool(std::string string);

};
}
#endif
