/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <pvcd_commands.h>

typedef int (*function_main)(int argc, char **argv);

class Main {
public:
	Main() :
			main_function(NULL) {
	}
	Main(std::string namePrefix, function_main main_function) :
			name_prefix(namePrefix), main_function(main_function) {
	}
	bool startsWith(std::string binaryName) {
		if (binaryName.length() < name_prefix.length())
			return false;
		return (strncasecmp(binaryName.c_str(), name_prefix.c_str(),
				name_prefix.length()) == 0);
	}
	function_main getMainFunction() {
		return main_function;
	}

private:
	std::string name_prefix;
	function_main main_function;
};

static std::string getBinaryName(const char *string) {
	int lastPos = 0;
	char c;
	for (int i = 0; (c = string[i]) != '\0'; ++i) {
		if (c == '/' || c == '\\')
			lastPos = i + 1;
	}
	return std::string(string + lastPos);
}

static void init_run() {
	//this method is run prior to any method
}
static void finalize_run() {
	//this method is run after any method
}

int main(int argc, char **argv) {
	try {
		std::vector<Main> af;
		af.push_back(Main("pvcd_db", pvcd_processDB));
		af.push_back(Main("pvcd_detect", pvcd_copyDetection));
		af.push_back(Main("pvcd_localMatches", pvcd_viewLocalMatches));
		af.push_back(Main("pvcd_search", pvcd_similaritySearch));
		af.push_back(Main("pvcd_mergeLocalToGlobal", pvcd_mergeLocalToGlobal));
		af.push_back(Main("pvcd_mergeNN", pvcd_mergeNN));
		af.push_back(Main("pvcd_evaluate", pvcd_evaluate));
		af.push_back(Main("pvcd_view", pvcd_viewImageVideos));
		std::string binaryName = getBinaryName(argv[0]);
		pvcd_system_setAfterInitFunction(init_run);
		pvcd_system_setBeforeExitFunction(finalize_run);
		for (Main main : af) {
			if (main.startsWith(binaryName)) {
				function_main main_function = main.getMainFunction();
				int ret = main_function(argc, argv);
				return ret;
			}
		}
		std::cout << "unknown command '" << binaryName << "'" << std::endl;
		return EXIT_FAILURE;
	} catch (const std::exception& ex) {
		std::cout << "[std::exception] " << ex.what() << std::endl;
	} catch (...) {
		std::cout << "[exception] " << std::endl;
	}
	return EXIT_FAILURE;
}
