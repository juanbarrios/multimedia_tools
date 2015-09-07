/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "string_util.hpp"
#include "../myutils_c.h"

using namespace my;

static std::string TOS(char *string) {
	std::string s(string);
	free(string);
	return s;
}

bool string::equals(std::string string1, std::string string2) {
	return my_string_equals(string1.c_str(), string2.c_str());
}
bool string::equals_ignorecase(std::string string1, std::string string2) {
	return my_string_equals_ignorecase(string1.c_str(), string2.c_str());
}
bool string::startsWith(std::string string, std::string prefix) {
	return my_string_startsWith(string.c_str(), prefix.c_str());
}
bool string::startsWith_ignorecase(std::string string, std::string prefix) {
	return my_string_startsWith_ignorecase(string.c_str(), prefix.c_str());
}
bool string::endsWith(std::string string, std::string suffix) {
	return my_string_endsWith(string.c_str(), suffix.c_str());
}
bool string::endsWith_ignorecase(std::string string, std::string suffix) {
	return my_string_endsWith_ignorecase(string.c_str(), suffix.c_str());
}
long long string::indexOf(std::string string, std::string substring) {
	return my_string_indexOf(string.c_str(), substring.c_str());
}
long long string::lastIndexOf(std::string string, std::string substring) {
	return my_string_lastIndexOf(string.c_str(), substring.c_str());
}
std::string subStringI::fromTo(std::string string, long long desdeIncluido,
		long long hastaSinIncluir) {
	return TOS(
			my_subStringI_fromTo(string.c_str(), desdeIncluido,
					hastaSinIncluir));
}
std::string subStringI::fromEnd(std::string string, long long desdeIncluido) {
	return TOS(my_subStringI_fromEnd(string.c_str(), desdeIncluido));
}
std::string subStringI::fromTo2(std::string string, long long desdeIncluido,
		long long hastaSinIncluir, bool trimBlancos) {
	return TOS(
			my_subStringI_fromTo2(string.c_str(), desdeIncluido,
					hastaSinIncluir, trimBlancos));
}
std::string subStringC::firstFirst(std::string string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	return TOS(
			my_subStringC_firstFirst(string.c_str(), charDesdeNoIncluido,
					charHastaNoIncluido));
}
std::string subStringC::firstLast(std::string string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	return TOS(
			my_subStringC_firstLast(string.c_str(), charDesdeNoIncluido,
					charHastaNoIncluido));
}
std::string subStringC::lastFirst(std::string string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	return TOS(
			my_subStringC_lastFirst(string.c_str(), charDesdeNoIncluido,
					charHastaNoIncluido));
}
std::string subStringC::lastLast(std::string string, char charDesdeNoIncluido,
		char charHastaNoIncluido) {
	return TOS(
			my_subStringC_lastLast(string.c_str(), charDesdeNoIncluido,
					charHastaNoIncluido));
}
std::string subStringC::lastEnd(std::string string, char charDesdeNoIncluido) {
	return TOS(my_subStringC_lastEnd(string.c_str(), charDesdeNoIncluido));
}
std::string subStringC::firstEnd(std::string string, char charDesdeNoIncluido) {
	return TOS(my_subStringC_firstEnd(string.c_str(), charDesdeNoIncluido));
}
std::string subStringC::startLast(std::string string,
		char charHastaNoIncluido) {
	return TOS(my_subStringC_startLast(string.c_str(), charHastaNoIncluido));
}
std::string subStringC::startFirst(std::string string,
		char charHastaNoIncluido) {
	return TOS(my_subStringC_startFirst(string.c_str(), charHastaNoIncluido));
}

std::string toString::intValue(long long value) {
	return TOS(my_newString_int(value));
}
std::string toString::floatValue(float value) {
	return TOS(my_newString_float(value));
}
std::string toString::doubleValue(double value, long long maxDecimals) {
	if (maxDecimals < 0)
		return TOS(my_newString_double(value));
	else
		return TOS(my_newString_doubleDec(value, maxDecimals));
}
std::string toString::boolValue(bool value) {
	return TOS(my_newString_bool(value));
}
std::string toString::hhmmssfff(double seconds) {
	return TOS(my_newString_hhmmssfff(seconds));
}
std::string toString::hhmmss(double seconds) {
	return TOS(my_newString_hhmmss(seconds));
}
std::string toString::hhmmss_forced(double seconds) {
	return TOS(my_newString_hhmmss_forced(seconds));
}
std::string toString::diskSpace(long long numBytes) {
	return TOS(my_newString_diskSpace(numBytes));
}

#include <ostream>
#include <sstream>
#include <iterator>

std::vector<std::string> string::split(std::string string, const char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(string);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (!item.empty())
			elems.push_back(item);
	}
	return elems;
}
std::string string::join(const std::vector<std::string>& elements,
		std::string delim) {
	switch (elements.size()) {
	case 0:
		return "";
	case 1:
		return elements[0];
	default:
		std::ostringstream os;
		std::copy(elements.begin(), elements.end() - 1,
				std::ostream_iterator<std::string>(os, delim.c_str()));
		os << *elements.rbegin();
		return os.str();
	}
}
