/*
 * Copyright (C) 2014-2015, ORAND S.A. <http://www.orand.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include <iostream>
#include <stdexcept>
#include "metricknn_cli.hpp"

int main_data(std::vector<std::string> &args);
int main_search(std::vector<std::string> &args);
int main_kmeans(std::vector<std::string> &args);
int main_pca(std::vector<std::string> &args);

static void print_version() {
	std::cout << "This file is part of MetricKnn. http://metricknn.org/" << std::endl;
	std::cout << "MetricKnn is made available under the terms of the BSD 2-Clause License." << std::endl;
	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
	std::cout << "" << std::endl;
#ifdef VERSION_NAME
#define STR_VERSION #VERSION_NAME
	std::cout << "Compiled version: " << STR_VERSION << std::endl;
#endif
}

int main(int argc, char **argv) {
	std::vector<std::string> args = my::collection::args_to_vector(argc, argv);
	if (args.size() < 2) {
		std::cout << "Usage: " <<  my::collection::args_getBinaryName(args) << std::endl;
		std::cout << "    -data ...           Loads, combines and exports datasets." << std::endl;
		std::cout << "    -search ...           Performs Similarity Search." << std::endl;
		std::cout << "    -kmeans ...           Performs K-means clustering algorithm." << std::endl;
		std::cout << "    -pca ...              Computes PCA algorithm." << std::endl;
		std::cout << "    -version              Prints MetricKnn version number and exits." << std::endl;
		return EXIT_FAILURE;
	}
	try {
		unsigned int i = 1;
		if (my::collection::is_next_arg_equal("-data", args, i)) {
			return main_data(args);
		} else if (my::collection::is_next_arg_equal("-search", args, i)) {
			return main_search(args);
		} else if (my::collection::is_next_arg_equal("-kmeans", args, i)) {
			return main_kmeans(args);
		} else if (my::collection::is_next_arg_equal("-pca", args, i)) {
			return main_pca(args);
		} else if (my::collection::is_next_arg_equal("-version", args, i)) {
			print_version();
			return EXIT_SUCCESS;
		} else {
			std::cout << "unknown option " << my::collection::next_arg(args, i)
					<< std::endl;
		}
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR" << std::endl;
	}
	return EXIT_FAILURE;
}
