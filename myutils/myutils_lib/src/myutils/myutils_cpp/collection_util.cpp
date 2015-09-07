/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "assert_util.hpp"

using namespace my;

std::vector<std::string> collection::args_to_vector(int argc, char **argv) {
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i) {
		args.push_back(std::string(argv[i]));
	}
	return args;
}
std::string collection::args_getBinaryName(std::vector<std::string> &args) {
	if (args.size() == 0)
		return "";
	return my::io::getFilename(args[0]);
}
std::string collection::next_arg(std::vector<std::string> &args,
		unsigned int &current_position) {
	if (current_position < args.size()) {
		std::string arg = args[current_position];
		current_position++;
		return arg;
	}
	return "";
}

long long collection::next_arg_int(std::vector<std::string> &args,
		unsigned int &current_position) {
	std::string arg = collection::next_arg(args, current_position);
	if (arg != "")
		return my::parse::stringToInt(arg);
	return 0;
}
double collection::next_arg_double(std::vector<std::string> &args,
		unsigned int &current_position) {
	std::string arg = collection::next_arg(args, current_position);
	if (arg != "")
		return my::parse::stringToDouble(arg);
	return 0;
}
bool collection::is_next_arg_equal(std::string expected_arg,
		std::vector<std::string> &args, unsigned int &current_position) {
	if (current_position < args.size()) {
		std::string arg = args[current_position];
		if (my::string::equals_ignorecase(arg, expected_arg)) {
			current_position++;
			return true;
		}
	}
	return false;
}
bool collection::is_next_arg_startsWith(std::string expected_prefix,
		std::vector<std::string> &args, unsigned int current_position) {
	if (current_position < args.size()) {
		std::string arg = args[current_position];
		if (my::string::startsWith_ignorecase(arg, expected_prefix)) {
			return true;
		}
	}
	return false;
}
