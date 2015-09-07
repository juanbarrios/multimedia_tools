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

std::string default_index = "LINEARSCAN";
int default_knn = 2;
double default_range = 0;

static void print_index_options(bool detailed) {
	std::cout << "    -index [index_string]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Index id and build parameters in the format: ID,parameter1=value1,parameter2=value2...."
				<< std::endl;
		std::cout << "       Default: " << default_index << std::endl;
	}
	std::cout << "    -index_save_file [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to save index.\n" << std::endl;
	std::cout << "    -index_load_file [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to load index.\n" << std::endl;
	std::cout << "    -list_indexes" << std::endl;
	if (detailed)
		std::cout << "        Lists all pre-defined indexes and exits.\n"
				<< std::endl;
	std::cout << "    -help_index [id_index]" << std::endl;
	if (detailed)
		std::cout << "        Prints help for a given index and exits.\n"
				<< std::endl;
}
static void print_search_options(bool detailed) {
	std::cout << "    -knn [num]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Number of nearest neighbors to retrieve for each query object."
				<< std::endl;
		std::cout << "       Default: " << my::toString::intValue(default_knn)
				<< std::endl;
	}
	std::cout << "    -range [value]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Search radius. A value less or equal than 0 means infinity range."
				<< std::endl;
		std::cout << "       Default: "
				<< my::toString::doubleValue(default_range) << std::endl;
	}
	std::cout << "    -search [search_string]" << std::endl;
	if (detailed)
		std::cout << "       Index search parameters.\n" << std::endl;
	std::cout << "    -maxThreads [num]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Maximum number of parallel threads to resolve searches."
				<< std::endl;
		std::cout << "       Default: number of cores.\n" << std::endl;
	}
}
static void print_output_options(bool detailed) {
	std::cout << "    -file_output [filename]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Print results to a file instead of standard output."
				<< std::endl;
		std::cout << std::endl;
	}
	std::cout << "    -description_query [filename]" << std::endl;
	std::cout << "    -description_reference [filename]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Loads a file containing a description for each object in the dataset, and in addition"
				<< std::endl;
		std::cout
				<< "       to the position of the object prints the line at the same position in 'filename'."
				<< std::endl;
		std::cout
				<< "       There must be as many lines in 'filename' as objects in the dataset."
				<< std::endl;
		std::cout << std::endl;
	}
	std::cout << "    -print_knn  |  -no_print_knn" << std::endl;
	if (detailed) {
		std::cout
				<< "       Prints the result of the search. By default prints the knn if they are just a few.\n"
				<< std::endl;
		std::cout << std::endl;
	}
	std::cout << "    -print_objects_query  |  -no_print_objects_query"
			<< std::endl;
	std::cout << "    -print_objects_reference  |  -no_print_objects_reference"
			<< std::endl;
	if (detailed) {
		std::cout
				<< "       In addition to the position, print the corresponding object. By default it prints the object if they are short."
				<< std::endl;
		std::cout << std::endl;
	}
}
static void print_help(std::vector<std::string> &args, bool detailed) {
	std::cout << "Usage: " << my::collection::args_getBinaryName(args) << " "
			<< args.at(1) << " [options] ..." << std::endl << std::endl;
	if (detailed) {
		std::cout << "\nMetricKnn is a library for similarity search.\n"
				<< std::endl;
	}
	std::cout << std::endl;
	std::cout << "    -help" << std::endl;
	if (detailed)
		std::cout << "       Shows this detailed help.\n" << std::endl;
	std::cout << std::endl << "QUERY DATASET OPTIONS" << std::endl;
	if (detailed)
		std::cout << "       The dataset with query objects.\n" << std::endl;
	print_dataset_options(detailed, "query_");
	std::cout << std::endl << "REFERENCE DATASET OPTIONS" << std::endl;
	if (detailed)
		std::cout << "       The dataset with objects to search in.\n"
				<< std::endl;
	print_dataset_options_brief(detailed, "reference_");
	if (detailed)
		std::cout << "       Analogous to query dataset options.\n"
				<< std::endl;
	std::cout << std::endl << "DISTANCE OPTIONS" << std::endl;
	print_distance_options(detailed);
	std::cout << std::endl << "INDEX OPTIONS" << std::endl;
	print_index_options(detailed);
	std::cout << std::endl << "SEARCH OPTIONS" << std::endl;
	print_search_options(detailed);
	std::cout << std::endl << "OUTPUT OPTIONS" << std::endl;
	print_output_options(detailed);
	std::cout << std::endl;
}

static bool parse_index_opt(OptionsIndex &opt_index,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-index", args, i)) {
		opt_index.string_index = my::collection::next_arg(args, i);
		if (!mknn_predefIndex_testIndexId(opt_index.string_index.c_str())) {
			std::cout << "Invalid index " << opt_index.string_index
					<< std::endl;
			std::cout << "Use -list_indexes to see valid indexes." << std::endl;
			exit(EXIT_FAILURE);
		}
	} else if (my::collection::is_next_arg_equal("-index_save_file", args, i)) {
		opt_index.save_file = my::collection::next_arg(args, i);
		VALIDATE_FILE_NOT_EXISTS(opt_index.save_file);
	} else if (my::collection::is_next_arg_equal("-index_load_file", args, i)) {
		opt_index.load_file = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(opt_index.load_file);
	} else if (my::collection::is_next_arg_equal("-list_indexes", args, i)) {
		mknn_predefIndex_helpListIndexes();
		exit(EXIT_SUCCESS);
	} else if (my::collection::is_next_arg_equal("-help_index", args, i)) {
		std::string id = my::collection::next_arg(args, i);
		mknn_predefIndex_helpPrintIndex(id.c_str());
		exit(EXIT_SUCCESS);
	} else {
		return false;
	}
	return true;
}
static bool parse_search_opt(OptionsSearch &opt_search,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-knn", args, i)) {
		opt_search.knn = my::collection::next_arg_int(args, i);
	} else if (my::collection::is_next_arg_equal("-range", args, i)) {
		opt_search.range = my::collection::next_arg_double(args, i);
	} else if (my::collection::is_next_arg_equal("-search", args, i)) {
		opt_search.string_search = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-maxThreads", args, i)) {
		opt_search.max_threads = my::collection::next_arg_int(args, i);
	} else {
		return false;
	}
	return true;
}
static bool parse_output_opt(OptionsOutput &opt_output,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-print_knn", args, i)) {
		opt_output.print_knn = "yes";
	} else if (my::collection::is_next_arg_equal("-no_print_knn", args, i)) {
		opt_output.print_knn = "no";
	} else if (my::collection::is_next_arg_equal("-file_output", args, i)) {
		opt_output.file_output = my::collection::next_arg(args, i);
		VALIDATE_FILE_NOT_EXISTS(opt_output.file_output);
	} else if (my::collection::is_next_arg_equal("-description_query", args,
			i)) {
		opt_output.file_description_query = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-description_reference", args,
			i)) {
		opt_output.file_description_reference = my::collection::next_arg(args,
				i);
	} else if (my::collection::is_next_arg_equal("-print_objects_query", args,
			i)) {
		opt_output.print_objects_query = "yes";
	} else if (my::collection::is_next_arg_equal("-no_print_objects_query",
			args, i)) {
		opt_output.print_objects_query = "no";
	} else if (my::collection::is_next_arg_equal("-print_objects_reference",
			args, i)) {
		opt_output.print_objects_reference = "yes";
	} else if (my::collection::is_next_arg_equal("-no_print_objects_reference",
			args, i)) {
		opt_output.print_objects_reference = "no";
	} else {
		return false;
	}
	return true;
}
int main_search(std::vector<std::string> &args) {
	if (args.size() < 3) {
		print_help(args, false);
		return EXIT_FAILURE;
	}
	OptionsDataset opt_query;
	OptionsDataset opt_ref;
	OptionsDistance opt_dist;
	OptionsIndex opt_index;
	OptionsSearch opt_search;
	OptionsOutput opt_output(args);
	//read parameters
	unsigned int i = 2;
	while (i < args.size()) {
		if (my::collection::is_next_arg_equal("-help", args, i)) {
			print_help(args, true);
			return EXIT_SUCCESS;
		} else if (parse_dataset_opt(opt_query, "query_", args, i)) {
			continue;
		} else if (parse_dataset_opt(opt_ref, "reference_", args, i)) {
			continue;
		} else if (parse_distance_opt(opt_dist, args, i)) {
			continue;
		} else if (parse_index_opt(opt_index, args, i)) {
			continue;
		} else if (parse_search_opt(opt_search, args, i)) {
			continue;
		} else if (parse_output_opt(opt_output, args, i)) {
			continue;
		} else {
			throw std::runtime_error(
					"unknown parameter " + my::collection::next_arg(args, i));
		}
	}
	//make the search
	search_process(opt_query, opt_ref, opt_dist, opt_index, opt_search,
			opt_output);
	return EXIT_SUCCESS;
}
