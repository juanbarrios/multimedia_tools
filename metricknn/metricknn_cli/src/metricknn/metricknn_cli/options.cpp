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

void print_dataset_options(bool detailed, std::string prefix) {
	std::cout << "    -" << prefix << "strings_file [filename]" << std::endl;
	if (detailed) {
		std::cout << "       Loads strings from file 'filename'." << std::endl;
		std::cout << "       File format: one string per line." << std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix << "vectors_text [filename] [dim_datatype]"
			<< std::endl;
	if (detailed) {
		std::cout << "       Loads vectors from file 'filename' in text format."
				<< std::endl;
		std::cout
				<< "       File format: one vector per line, each coordinate separated by one or more space or tabs."
				<< std::endl;
		std::cout
				<< "       Each coordinate is casted to a value in 'dim_datatype', which impacts the needed memory to store data."
				<< std::endl;
		print_datatypes("         ");
		std::cout
				<< "       If text parsing takes a lot of time, consider using option -"
				<< prefix << "save_file in order" << std::endl;
		std::cout
				<< "       to generate a binary file to be loaded in future experiments."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix
			<< "vectors_raw [filename] [rows] [columns] [dim_datatype]"
			<< std::endl;
	if (detailed) {
		std::cout
				<< "       Loads vectors from file 'filename' in binary format."
				<< std::endl;
		std::cout
				<< "       File format: all vectors coordinates in binary format, the file size must match the expected"
				<< std::endl;
		std::cout
				<< "       number of bytes computed as rows x columns x sizeof(datatype)."
				<< std::endl;
		print_datatypes("         ");
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix
			<< "vectors_random [num_vectors] [dimensions] [dim_min_value] [dim_max_value] [dim_datatype]"
			<< std::endl;
	if (detailed) {
		std::cout
				<< "       A dataset is created by generating 'num_vectors' vectors of length 'dimensions'."
				<< std::endl;
		std::cout
				<< "       Each coordinate is a number between 'dim_min_value' (included) and 'dim_max_value' (excluded)."
				<< std::endl;
		std::cout
				<< "       Each coordinate is casted to a value in 'dim_datatype', which impacts the needed memory to store data."
				<< std::endl;
		std::cout << "       Valid datatypes are the same than in option -"
				<< prefix << "vectors_file." << std::endl;
		std::cout
				<< "       The random generation may take a lot of time, consider using option -"
				<< prefix << "save_file in order" << std::endl;
		std::cout
				<< "       to generate a binary file to be loaded in future experiments."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix << "load_dataset [filename]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Loads objects from file 'filename' in binary MKNN format (created by -save_dataset)."
				<< std::endl;
		std::cout
				<< "       The objects in 'filename' are stored in binary format for efficient loading."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix << "concatenate" << std::endl;
	if (detailed) {
		std::cout
				<< "       If the input considers more than one source, i.e., one or more occurrences of"
				<< std::endl;
		std::cout << "       -" << prefix << "strings_file, -" << prefix
				<< "vectors_file, -" << prefix << "vectors_random or -"
				<< prefix << "load_dataset," << std::endl;
		std::cout
				<< "       all the loaded objects will be CONCATENATED one after the other to produce the final dataset."
				<< std::endl;
		std::cout
				<< "       For example, if dataset A contains N vectors of D coordinates, and dataset B contains M"
				<< std::endl;
		std::cout
				<< "       vectors of D coordinates, the concatenation produces a new dataset containing N+M vectors."
				<< std::endl;
		std::cout << "       of D coordinates." << std::endl;
		std::cout
				<< "       In order to produce a valid dataset, all the domains of the object sources must coincide."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix << "combine" << std::endl;
	if (detailed) {
		std::cout
				<< "       If the input considers more than one source, i.e., one or more occurrences of"
				<< std::endl;
		std::cout << "       -" << prefix << "strings_file, -" << prefix
				<< "vectors_file, -" << prefix << "vectors_random or -"
				<< prefix << "load_dataset," << std::endl;
		std::cout
				<< "       all the loaded objects will be MIXED to produce a MULTIOBJECT dataset."
				<< std::endl;
		std::cout
				<< "       For example, if dataset A contains N vectors of D coordinates, and dataset B contains N vectors"
				<< std::endl;
		std::cout
				<< "       of E coordinates, the combination produces a dataset with N objects, where each object is an array"
				<< std::endl;
		std::cout
				<< "       of length 2 whose first value is a D-dimensional vector and second value is an E-dimensional vector."
				<< std::endl;
		std::cout
				<< "       In order to produce a MULTIOBJECT dataset, all the sources must contain the same number of objects,"
				<< std::endl;
		std::cout
				<< "       but they can contain different domains. The combined objects must be compared with a multi-metric distance."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -" << prefix << "random_sample [number]" << std::endl;
	if (detailed) {
		std::cout << "       Selects a random sample of the full dataset. "
				<< std::endl;
		std::cout
				<< "       If the value of [number] is greater or equal 1 it corresponds to the number of elements to sample, "
				<< std::endl;
		std::cout
				<< "       If the value of [number] is between 0 and 1 (exclusive)  it corresponds to the fraction of elements to sample, e.g. 0.5 means half size."
				<< std::endl;
	}
}
void print_dataset_options_brief(bool detailed, std::string prefix) {
	std::cout << "    -" << prefix << "strings_file [filename]" << std::endl;
	std::cout << "    -" << prefix << "vectors_text [filename] [dim_datatype]"
			<< std::endl;
	std::cout << "    -" << prefix
			<< "vectors_raw [filename] [rows] [columns] [dim_datatype]"
			<< std::endl;
	std::cout << "    -" << prefix
			<< "vectors_random [num_vectors] [dimensions] [dim_min_value] [dim_max_value] [dim_datatype]"
			<< std::endl;
	std::cout << "    -" << prefix << "load_dataset [filename]" << std::endl;
	std::cout << "    -" << prefix << "concatenate" << std::endl;
	std::cout << "    -" << prefix << "combine" << std::endl;
}
void print_distance_options(bool detailed) {
	std::cout << "    -distance [distance_string]" << std::endl;
	if (detailed)
		std::cout
				<< "       Distance id and parameters in the format: ID,parameter1=value1,parameter2=value2...\n"
				<< std::endl;
	std::cout << "    -distance_save_file [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to save distance.\n" << std::endl;
	std::cout << "    -distance_load_file [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to load distance.\n" << std::endl;
	std::cout << "    -list_distances" << std::endl;
	if (detailed)
		std::cout << "        Lists all pre-defined distances and exits.\n"
				<< std::endl;
	std::cout << "    -help_distance [id_distance]" << std::endl;
	if (detailed)
		std::cout << "        Prints help for a given distance and exits.\n"
				<< std::endl;
}

bool parse_dataset_opt(OptionsDataset &opt, std::string prefix,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-" + prefix + "strings_file", args,
			i)) {
		InputData input;
		input.strings_filename = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(input.strings_filename);
		opt.inputList.push_back(input);
	} else if (my::collection::is_next_arg_equal("-" + prefix + "vectors_text",
			args, i)) {
		InputData input;
		input.vectors_text.filename = my::collection::next_arg(args, i);
		input.vectors_text.datatype = my::collection::next_arg(args, i);
		if (input.vectors_text.filename == ""
				|| input.vectors_text.datatype == "") {
			throw std::runtime_error(
					"Incomplete parameters: vectors_txt [filename] [dim_datatype]");
		}
		VALIDATE_FILE_EXISTS(input.vectors_text.filename);
		VALIDATE_DATATYPE(input.vectors_text.datatype);
		opt.inputList.push_back(input);
	} else if (my::collection::is_next_arg_equal("-" + prefix + "vectors_raw",
			args, i)) {
		InputData input;
		input.vectors_raw.filename = my::collection::next_arg(args, i);
		input.vectors_raw.rows = my::collection::next_arg(args, i);
		input.vectors_raw.columns = my::collection::next_arg(args, i);
		input.vectors_raw.datatype = my::collection::next_arg(args, i);
		if (input.vectors_raw.filename == "" || input.vectors_raw.rows == ""
				|| input.vectors_raw.columns == ""
				|| input.vectors_raw.datatype == "") {
			throw std::runtime_error(
					"Incomplete parameters: -" + prefix
							+ "vectors_raw [filename] [rows] [columns] [dim_datatype]");
		}
		VALIDATE_FILE_EXISTS(input.vectors_raw.filename);
		VALIDATE_DATATYPE(input.vectors_raw.datatype);
		opt.inputList.push_back(input);
	} else if (my::collection::is_next_arg_equal(
			"-" + prefix + "vectors_random", args, i)) {
		InputData input;
		input.vectors_random.num_vectors = my::collection::next_arg(args, i);
		input.vectors_random.dimension = my::collection::next_arg(args, i);
		input.vectors_random.min_value = my::collection::next_arg(args, i);
		input.vectors_random.max_value = my::collection::next_arg(args, i);
		input.vectors_random.datatype = my::collection::next_arg(args, i);
		if (input.vectors_random.num_vectors == ""
				|| input.vectors_random.dimension == ""
				|| input.vectors_random.min_value == ""
				|| input.vectors_random.max_value == ""
				|| input.vectors_random.datatype == "") {
			throw std::runtime_error(
					"Incomplete parameters: -" + prefix
							+ "vectors_random [num_vectors] [dimensions] [dim_min_value] [dim_max_value] [dim_datatype]");
		}
		VALIDATE_DATATYPE(input.vectors_random.datatype);
		opt.inputList.push_back(input);
	} else if (my::collection::is_next_arg_equal("-" + prefix + "load_dataset",
			args, i)) {
		InputData input;
		input.dataset_filename = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(input.dataset_filename);
		opt.inputList.push_back(input);
	} else if (my::collection::is_next_arg_equal("-" + prefix + "concatenate",
			args, i)) {
		opt.concatenate_input = true;
		opt.combine_input = false;
	} else if (my::collection::is_next_arg_equal("-" + prefix + "combine", args,
			i)) {
		opt.concatenate_input = false;
		opt.combine_input = true;
	} else if (my::collection::is_next_arg_equal("-" + prefix + "random_sample",
			args, i)) {
		opt.randomSample = my::collection::next_arg(args, i);
	} else {
		return false;
	}
	return true;
}

bool parse_distance_opt(OptionsDistance &opt_dist,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-distance", args, i)) {
		opt_dist.string_distance = my::collection::next_arg(args, i);
		if (!mknn_predefDistance_testDistanceId(
				opt_dist.string_distance.c_str())) {
			std::cout << "Invalid distance " << opt_dist.string_distance
					<< std::endl;
			std::cout << "Use -list_distances to see valid distances."
					<< std::endl;
			exit(EXIT_FAILURE);
		}
	} else if (my::collection::is_next_arg_equal("-distance_save_file", args,
			i)) {
		opt_dist.save_file = my::collection::next_arg(args, i);
		VALIDATE_FILE_NOT_EXISTS(opt_dist.save_file);
	} else if (my::collection::is_next_arg_equal("-distance_load_file", args,
			i)) {
		opt_dist.load_file = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(opt_dist.load_file);
	} else if (my::collection::is_next_arg_equal("-list_distances", args, i)) {
		mknn_predefDistance_helpListDistances();
		exit(EXIT_SUCCESS);
	} else if (my::collection::is_next_arg_equal("-help_distance", args, i)) {
		std::string id = my::collection::next_arg(args, i);
		mknn_predefDistance_helpPrintDistance(id.c_str());
		exit(EXIT_SUCCESS);
	} else {
		return false;
	}
	return true;
}
