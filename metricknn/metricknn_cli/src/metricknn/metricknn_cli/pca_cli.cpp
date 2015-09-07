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

class OptionsPca {
public:
	std::string save_state;
	std::string load_state;
	std::string newDimension;
	std::string output_datatype;
	std::string output_filename;
	std::string output_txt;
};

static void print_pca_options(bool detailed) {
	std::cout << "    -saveState [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to save kmeans state.\n" << std::endl;
	std::cout << "    -loadState [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to load kmeans state.\n" << std::endl;
	std::cout << "    -newDimension [num_dimensions]" << std::endl;
	if (detailed)
		std::cout << "        Number of dimensions to reduce the dataset.\n"
				<< std::endl;
	std::cout << "    -output_dataset [filename] [dim_datatype]" << std::endl;
	if (detailed) {
		std::cout << "       Filename and datatype of the transformed dataset."
				<< std::endl;
		print_datatypes("         ");
		std::cout << "" << std::endl;
	}
	std::cout << "    -output_txt [filename]" << std::endl;
	if (detailed) {
		std::cout << "       Filename of the transformed dataset." << std::endl;
		print_datatypes("         ");
		std::cout << "" << std::endl;
	}
}
static void print_help(std::vector<std::string> &args, bool detailed) {
	std::cout << "Usage: " << my::collection::args_getBinaryName(args) << " "
			<< args.at(1) << " [options] ..." << std::endl << std::endl;
	std::cout << "    -help" << std::endl;
	if (detailed)
		std::cout << "       Shows this detailed help.\n" << std::endl;
	std::cout << std::endl << "DATASET TO COMPUTE PCA" << std::endl;
	print_dataset_options(detailed, "pca_");
	std::cout << "" << std::endl;
	std::cout << std::endl << "DATASET TO TRANSFORM" << std::endl;
	print_dataset_options(detailed, "input_");
	std::cout << "" << std::endl;
	std::cout << std::endl << "PCA PROCESS OPTIONS" << std::endl;
	print_pca_options(detailed);
	std::cout << "" << std::endl;
}

static bool parse_pca_opt(OptionsPca &opt, std::vector<std::string> &args,
		unsigned int &i) {
	if (my::collection::is_next_arg_equal("-saveState", args, i)) {
		opt.save_state = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-loadState", args, i)) {
		opt.load_state = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(opt.load_state);
	} else if (my::collection::is_next_arg_equal("-newDimension", args, i)) {
		opt.newDimension = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-output_dataset", args, i)) {
		opt.output_filename = my::collection::next_arg(args, i);
		opt.output_datatype = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-output_txt", args, i)) {
		opt.output_txt = my::collection::next_arg(args, i);
	} else {
		return false;
	}
	return true;
}
static void run_pca(OptionsDataset &opt_input_dataset,
		OptionsDataset &opt_transform_dataset, OptionsPca &opt_pca) {
	MknnPcaAlgorithm *pca = mknn_pca_new();
	if (my::io::existsFile(opt_pca.load_state)) {
		std::cout << "loading PCA from " << opt_pca.load_state << std::endl;
		mknn_pca_restore(pca, opt_pca.load_state.c_str());
	} else {
		MknnDataset *dataset = load_dataset(opt_input_dataset);
		if (dataset == NULL)
			throw std::runtime_error(
					"must enter a PCA state to load or a dataset to compute PCA.");
		mknn_pca_addDatasetToVectorStats(pca, dataset);
		mknn_pca_computeTransformationMatrix(pca);
		if (opt_pca.save_state != "") {
			std::cout << "saving PCA to " << opt_pca.save_state << std::endl;
			mknn_pca_save(pca, opt_pca.save_state.c_str(), true);
		}
	}
	MknnDataset *dataset_in = load_dataset(opt_transform_dataset);
	if (dataset_in != NULL) {
		int64_t dims_in = mknn_pca_getInputDimension(pca);
		int64_t dims_in2 = mknn_domain_vector_getNumDimensions(
				mknn_dataset_getDomain(dataset_in));
		my::assert::equalInt("input dimension", dims_in2, dims_in);
		long long dims_out = 0;
		if (opt_pca.newDimension != "")
			dims_out = my::parse::stringToInt(opt_pca.newDimension);
		if (dims_out <= 0 || dims_out > dims_in)
			dims_out = dims_in;
		MknnDatatype dt_out = MKNN_DATATYPE_FLOATING_POINT_32bits;
		if (opt_pca.output_datatype != "")
			dt_out = GET_DATATYPE(opt_pca.output_datatype);
		else if (mknn_datatype_isDouble(
				mknn_domain_vector_getDimensionDataType(
						mknn_dataset_getDomain(dataset_in))))
			dt_out = MKNN_DATATYPE_FLOATING_POINT_64bits;
		int64_t numvec = mknn_dataset_getNumObjects(dataset_in);
		char* data = MY_MALLOC(numvec * dims_out * mknn_datatype_sizeof(dt_out),
				char);
		MknnDataset *dataset_out = mknn_datasetLoader_PointerCompactVectors(
				data, true, numvec, dims_out, dt_out);
		mknn_pca_transform_dataset(pca, dataset_in, dataset_out);
		if (opt_pca.output_filename != "")
			mknn_dataset_save(dataset_out, opt_pca.output_filename.c_str());
		if (opt_pca.output_txt != "") {
			mknn_dataset_printObjectsTextFile(dataset_out,
					opt_pca.output_txt.c_str());
		}
		mknn_dataset_release(dataset_out);
	}
	mknn_pca_release(pca);
}
int main_pca(std::vector<std::string> &args) {
	if (args.size() < 3) {
		print_help(args, false);
		return EXIT_FAILURE;
	}
	OptionsDataset opt_dataset_pca;
	OptionsDataset opt_dataset_input;
	OptionsPca opt_pca;
	//read parameters
	unsigned int i = 2;
	while (i < args.size()) {
		if (my::collection::is_next_arg_equal("-help", args, i)) {
			print_help(args, true);
			return EXIT_SUCCESS;
		} else if (parse_dataset_opt(opt_dataset_pca, "pca_", args, i)) {
			continue;
		} else if (parse_dataset_opt(opt_dataset_input, "input_", args, i)) {
			continue;
		} else if (parse_pca_opt(opt_pca, args, i)) {
			continue;
		} else {
			throw std::runtime_error(
					"unknown parameter " + my::collection::next_arg(args, i));
		}
	}
	run_pca(opt_dataset_pca, opt_dataset_input, opt_pca);
	return EXIT_SUCCESS;
}
