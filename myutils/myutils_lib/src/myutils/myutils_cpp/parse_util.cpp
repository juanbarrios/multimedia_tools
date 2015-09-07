/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "parse_util.hpp"
#include <stdexcept>
#include <cstdlib>
#include <limits>
#include "../myutils_c.h"

using namespace my;

void parse::setDefaultLocale() {
	my_parse_setDefaultLocale();
}

int parse::stringToInt(std::string string, bool acceptEmptyString,
		int valueEmptyString) {
	if (string == "") {
		if (acceptEmptyString)
			return valueEmptyString;
		else
			throw std::runtime_error("parse integer: \"" + string + "\"");
	}
	if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		throw std::runtime_error("parse integer: \"" + string + "\"");
	errno = 0;
	char *endptr = NULL;
	long int val = std::strtol(string.c_str(), &endptr, 10);
	if (endptr == string.c_str() || *endptr != '\0' || errno != 0)
		throw std::runtime_error("parse integer: \"" + string + "\"");
	return val;
}
long long parse::stringToLong(std::string string, bool acceptEmptyString,
		long long valueEmptyString) {
	if (string == "") {
		if (acceptEmptyString)
			return valueEmptyString;
		else
			throw std::runtime_error("parse long: \"" + string + "\"");
	}
	if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		throw std::runtime_error("parse long: \"" + string + "\"");
	errno = 0;
	char *endptr = NULL;
	long long val = std::strtoll(string.c_str(), &endptr, 10);
	if (endptr == string.c_str() || *endptr != '\0' || errno != 0)
		throw std::runtime_error("parse long: \"" + string + "\"");
	return val;
}
double parse::stringToDouble(std::string string, bool acceptEmptyString,
		double valueEmptyString) {
	if (string == "") {
		if (acceptEmptyString)
			return valueEmptyString;
		else
			throw std::runtime_error("parse double: \"" + string + "\"");
	}
	if (string::equals_ignorecase(string, "INF")
			|| string::equals_ignorecase(string, "NAN")) {
		return std::numeric_limits<double>::max();
	} else if (string::equals_ignorecase(string, "-INF")) {
		return std::numeric_limits<double>::lowest();
	} else if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		throw std::runtime_error("parse double: \"" + string + "\"");
	errno = 0;
	char *endptr = NULL;
	double val = std::strtod(string.c_str(), &endptr);
	if (endptr == string.c_str() || *endptr != '\0' || errno != 0)
		throw std::runtime_error("parse double: \"" + string + "\"");
	return val;
}
float parse::stringToFloat(std::string string, bool acceptEmptyString,
		float valueEmptyString) {
	if (string == "") {
		if (acceptEmptyString)
			return valueEmptyString;
		else
			throw std::runtime_error("parse float: \"" + string + "\"");
	}
	if (string::equals_ignorecase(string, "INF")
			|| string::equals_ignorecase(string, "NAN")) {
		return std::numeric_limits<float>::max();
	} else if (string::equals_ignorecase(string, "-INF")) {
		return std::numeric_limits<float>::lowest();
	} else if ((string[0] < '0' || string[0] > '9') && string[0] != '-')
		throw std::runtime_error("parse float: \"" + string + "\"");
	errno = 0;
	char *endptr = NULL;
	float val = std::strtof(string.c_str(), &endptr);
	if (endptr == string.c_str() || *endptr != '\0' || errno != 0)
		throw std::runtime_error("parse float: \"" + string + "\"");
	return val;
}
std::vector<int> parse::stringListToInt(const std::vector<std::string> &list,
bool acceptEmptyString, int valueEmptyString) {
	std::vector<int> v;
	v.reserve(list.size());
	for (std::string s : list) {
		v.push_back(parse::stringToInt(s, acceptEmptyString, valueEmptyString));
	}
	return v;
}
std::vector<long long> parse::stringListToLong(
		const std::vector<std::string> &list,
		bool acceptEmptyString, long long valueEmptyString) {
	std::vector<long long> v;
	v.reserve(list.size());
	for (std::string s : list) {
		v.push_back(
				parse::stringToLong(s, acceptEmptyString, valueEmptyString));
	}
	return v;
}

std::vector<double> parse::stringListToDouble(
		const std::vector<std::string> &list,
		bool acceptEmptyString, double valueEmptyString) {
	std::vector<double> v;
	v.reserve(list.size());
	for (std::string s : list) {
		v.push_back(
				parse::stringToDouble(s, acceptEmptyString, valueEmptyString));
	}
	return v;
}
std::vector<float> parse::stringListToFloat(
		const std::vector<std::string> &list,
		bool acceptEmptyString, float valueEmptyString) {
	std::vector<float> v;
	v.reserve(list.size());
	for (std::string s : list) {
		v.push_back(
				parse::stringToFloat(s, acceptEmptyString, valueEmptyString));
	}
	return v;
}

double parse::fractionToDouble(std::string fraction_string) {
	return my_parse_fraction(fraction_string.c_str());
}
double parse::timestampToDouble(std::string timestamp_string) {
	return my_parse_seconds(timestamp_string.c_str());
}

bool parse::stringToBool(std::string string) {
	return my_parse_bool(string.c_str());
}

