/*
 * Copyright (C) 2014-2015, ORAND S.A. <http://www.orand.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef METRICKNN_CLI_HPP
#define METRICKNN_CLI_HPP

#include <vector>
#include <string>
#include <metricknn/metricknn_cpp.hpp>
#include <metricknn/metricknn_cpp/metricknn_impl.hpp>
#include <myutils/myutils_cpp.hpp>
#include <metricknn/metricknn_c.h>
#include <metricknn/metricknn_c/metricknn_impl.h>
#include <myutils/myutils_c.h>

class InputVectorsRandom {
public:
	std::string num_vectors;
	std::string dimension;
	std::string min_value;
	std::string max_value;
	std::string datatype;
};
class InputVectorsText {
public:
	std::string filename;
	std::string datatype;
};
class InputVectorsRaw {
public:
	std::string filename;
	std::string rows;
	std::string columns;
	std::string datatype;
};
class InputData {
public:
	std::string strings_filename;
	std::string dataset_filename;
	InputVectorsRandom vectors_random;
	InputVectorsText vectors_text;
	InputVectorsRaw vectors_raw;
};

class OptionsDataset {
public:
	std::vector<InputData> inputList;bool concatenate_input, combine_input;
	std::string randomSample;
	OptionsDataset() :
			concatenate_input(false), combine_input(false) {
	}
};
class OptionsDistance {
public:
	std::string string_distance;
	std::string save_file;
	std::string load_file;
};
class OptionsIndex {
public:
	std::string string_index;
	std::string save_file;
	std::string load_file;
};
class OptionsSearch {
public:
	std::string knn;
	std::string range;
	std::string max_threads;
	std::string string_search;
};
class OptionsOutput {
public:
	std::string file_output;
	std::string print_knn, print_objects_query, print_objects_reference;
	std::string file_description_query;
	std::string file_description_reference;
	std::vector<std::string> &args;
	OptionsOutput(std::vector<std::string> &args) :
			args(args) {
	}
};

extern std::string default_index;
extern int default_knn;
extern double default_range;

void search_process(OptionsDataset &opt_query, OptionsDataset &opt_ref,
		OptionsDistance &opt_dist, OptionsIndex &opt_index,
		OptionsSearch &opt_search, OptionsOutput &opt_output);

void print_datatypes(std::string pref);
void VALIDATE_FILE_EXISTS(std::string filename);
void VALIDATE_FILE_NOT_EXISTS(std::string filename);
void VALIDATE_DATATYPE(std::string datatype);
MknnDatatype GET_DATATYPE(std::string datatype);

MknnDataset *load_dataset(OptionsDataset &options);
MknnDistance *load_distance(OptionsDistance &options);

void print_dataset_options(bool detailed, std::string prefix);
void print_dataset_options_brief(bool detailed, std::string prefix);
void print_distance_options(bool detailed);

bool parse_dataset_opt(OptionsDataset &opt, std::string prefix,
		std::vector<std::string> &args, unsigned int &i);
bool parse_distance_opt(OptionsDistance &opt_dist,
		std::vector<std::string> &args, unsigned int &i);

#endif
