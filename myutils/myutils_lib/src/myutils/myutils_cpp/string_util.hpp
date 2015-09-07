/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_STRING_UTIL_HPP
#define MY_STRING_UTIL_HPP

#include "../myutils_cpp.hpp"

#include <sstream>

namespace my {
class string {
public:
	static bool equals(std::string string1, std::string string2);
	static bool equals_ignorecase(std::string string1, std::string string2);
	static bool startsWith(std::string string, std::string prefix);
	static bool startsWith_ignorecase(std::string string, std::string prefix);
	static bool endsWith(std::string string, std::string suffix);
	static bool endsWith_ignorecase(std::string string, std::string suffix);

	static long long indexOf(std::string string, std::string substring);
	static long long lastIndexOf(std::string string, std::string substring);

	static std::vector<std::string> split(std::string string, const char delim);
	static std::string join(const std::vector<std::string>& elements,
			std::string delim);

};

class subStringI {
public:
	static std::string fromTo(std::string string, long long desdeIncluido,
			long long hastaSinIncluir);
	static std::string fromEnd(std::string string, long long desdeIncluido);
	static std::string fromTo2(std::string string, long long desdeIncluido,
			long long hastaSinIncluir, bool trimBlancos);

};

class subStringC {
public:
	static std::string firstFirst(std::string string, char charDesdeNoIncluido,
			char charHastaNoIncluido);
	static std::string firstLast(std::string string, char charDesdeNoIncluido,
			char charHastaNoIncluido);
	static std::string lastFirst(std::string string, char charDesdeNoIncluido,
			char charHastaNoIncluido);
	static std::string lastLast(std::string string, char charDesdeNoIncluido,
			char charHastaNoIncluido);
	static std::string lastEnd(std::string string, char charDesdeNoIncluido);
	static std::string firstEnd(std::string string, char charDesdeNoIncluido);
	static std::string startLast(std::string string, char charHastaNoIncluido);
	static std::string startFirst(std::string string, char charHastaNoIncluido);

};

class toString {
public:
	static std::string intValue(long long value);
	static std::string floatValue(float value);
	static std::string doubleValue(double value, long long maxDecimals = -1);
	static std::string boolValue(bool value);
	static std::string hhmmssfff(double seconds);
	static std::string hhmmss(double seconds);
	static std::string hhmmss_forced(double seconds);
	static std::string diskSpace(long long numBytes);

	template<typename T> static std::string collection(T const &list,
			const std::string string_start, const std::string string_delim,
			const std::string string_end) {
		std::ostringstream os;
		os << string_start;
		bool first = true;
		for (auto const &value : list) {
			if (first) {
				first = false;
			} else {
				os << string_delim;
			}
			os << value;
		}
		os << string_end;
		return os.str();
	}

};

}

#endif
