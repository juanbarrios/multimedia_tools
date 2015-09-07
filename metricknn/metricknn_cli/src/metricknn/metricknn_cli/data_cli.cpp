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

class OptionsConvert {
public:
	std::string new_datatype;
	std::string function_convert;
};
class OptionsExport {
public:
	std::string printTxt_filename;
	std::string printRaw_filename;
	std::string save_filename;bool overwrite;
	OptionsExport() :
			overwrite(false) {
	}
};

static void print_help(std::vector<std::string> &args, bool detailed) {
	std::cout << "Usage: " << my::collection::args_getBinaryName(args) << " "
			<< args.at(1) << " [options] ..." << std::endl << std::endl;
	std::cout << std::endl << "INPUT OPTIONS" << std::endl;
	print_dataset_options(detailed, "");
	std::cout << std::endl << "CONVERT OPTIONS" << std::endl;
	std::cout << "    -convert_datatype [datatype]" << std::endl;
	std::cout << "    -convert_values [function_name]" << std::endl;
	if (detailed) {
		std::cout
				<< "        Convert each object value using a function. Valid functions: COPY SQRT."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << std::endl << "EXPORT OPTIONS" << std::endl;
	std::cout << "    -print_txt [filename]" << std::endl;
	if (detailed) {
		std::cout
				<< "        Prints all the objects in the dataset in text format to 'filename'."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -print_raw [filename]" << std::endl;
	if (detailed) {
		std::cout
				<< "        Prints all the objects in the dataset in raw format to 'filename'."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -save_dataset [filename]" << std::endl;
	if (detailed) {
		std::cout << "       Saves the loaded dataset to 'filename'."
				<< std::endl;
		std::cout << "" << std::endl;
	}
	std::cout << "    -overwrite" << std::endl;
	if (detailed) {
		std::cout << "       Enables the overwriting of files." << std::endl;
		std::cout << "" << std::endl;
	}
}

static bool parse_convert_options(OptionsConvert &opt,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-convert_values", args, i)) {
		opt.function_convert = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-convert_datatype", args,
			i)) {
		opt.new_datatype = my::collection::next_arg(args, i);
		VALIDATE_DATATYPE(opt.new_datatype);
	} else {
		return false;
	}
	return true;
}
static bool parse_export_options(OptionsExport &opt,
		std::vector<std::string> &args, unsigned int &i) {
	if (my::collection::is_next_arg_equal("-print_txt", args, i)) {
		opt.printTxt_filename = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-print_raw", args, i)) {
		opt.printRaw_filename = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-save_dataset", args, i)) {
		opt.save_filename = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-overwrite", args, i)) {
		opt.overwrite = true;
	} else {
		return false;
	}
	return true;
}
static MknnDataset *convert_dataset(MknnDataset *dataset_in,
		OptionsConvert &opt_convert) {
	if (opt_convert.new_datatype == "" && opt_convert.function_convert == "")
		return NULL;
	if (!mknn_domain_isGeneralDomainVector(mknn_dataset_getDomain(dataset_in)))
		throw std::runtime_error("conversion is only supported for vectors");
	MknnDatatype datatype_in = mknn_domain_vector_getDimensionDataType(
			mknn_dataset_getDomain(dataset_in));
	int64_t num_vecs = mknn_dataset_getNumObjects(dataset_in);
	int64_t num_dims = mknn_domain_vector_getNumDimensions(
			mknn_dataset_getDomain(dataset_in));
	MknnDatatype datatype_out =
			(opt_convert.new_datatype == "") ?
					datatype_in : GET_DATATYPE(opt_convert.new_datatype);
	int64_t size_dim = mknn_datatype_sizeof(datatype_out);
	char *data = MY_MALLOC(num_vecs * num_dims * size_dim, char);
	MknnDataset *dataset_out = mknn_datasetLoader_PointerCompactVectors(data,
	true, num_vecs, num_dims, datatype_out);
	if (opt_convert.function_convert == ""
			|| my::string::equals_ignorecase(opt_convert.function_convert,
					"copy")) {
		my_function_copy_vector funcCopy = my_datatype_getFunctionCopyVector(
				mknn_datatype_convertMknn2My(datatype_in),
				mknn_datatype_convertMknn2My(datatype_out));
		for (int64_t i = 0; i < num_vecs; ++i) {
			void *vector1 = mknn_dataset_getObject(dataset_in, i);
			void *vector2 = mknn_dataset_getObject(dataset_out, i);
			funcCopy(vector1, vector2, num_dims);
		}
	} else if (my::string::equals_ignorecase(opt_convert.function_convert,
			"sqrt")) {
		my_function_copyOperate_vector funcCopyOper =
				my_datatype_getFunctionCopyOperateVector(
						mknn_datatype_convertMknn2My(datatype_in),
						mknn_datatype_convertMknn2My(datatype_out));
		for (int64_t i = 0; i < num_vecs; ++i) {
			void *vector1 = mknn_dataset_getObject(dataset_in, i);
			void *vector2 = mknn_dataset_getObject(dataset_out, i);
			funcCopyOper(vector1, vector2, num_dims, sqrt);
		}
	} else {
		throw std::runtime_error(
				"unknown function '" + opt_convert.function_convert + "'");
	}
	return dataset_out;
}
static void export_dataset(OptionsExport &opt_export, MknnDataset *dataset) {
	if (opt_export.save_filename != "") {
		mknn_dataset_save(dataset, opt_export.save_filename.c_str());
	}
	if (opt_export.printRaw_filename != "") {
		mknn_dataset_printObjectsRawFile(dataset,
				opt_export.printRaw_filename.c_str());
	}
	if (opt_export.printTxt_filename != "") {
		mknn_dataset_printObjectsTextFile(dataset,
				opt_export.printTxt_filename.c_str());
	}
}
int main_data(std::vector<std::string> &args) {
	if (args.size() < 3) {
		print_help(args, false);
		return EXIT_FAILURE;
	}
	OptionsDataset opt_dataset;
	OptionsConvert opt_convert;
	OptionsExport opt_export;
	//read parameters
	unsigned int i = 2;
	while (i < args.size()) {
		if (my::collection::is_next_arg_equal("-help", args, i)) {
			print_help(args, true);
			exit(EXIT_SUCCESS);
		} else if (parse_dataset_opt(opt_dataset, "", args, i)) {
			continue;
		} else if (parse_convert_options(opt_convert, args, i)) {
			continue;
		} else if (parse_export_options(opt_export, args, i)) {
			continue;
		} else {
			throw std::runtime_error(
					"unknown parameter " + my::collection::next_arg(args, i));
		}
	}
	if (!opt_export.overwrite) {
		VALIDATE_FILE_NOT_EXISTS(opt_export.save_filename);
		VALIDATE_FILE_NOT_EXISTS(opt_export.printRaw_filename);
		VALIDATE_FILE_NOT_EXISTS(opt_export.printTxt_filename);
	}
	MknnDataset *dataset_in = load_dataset(opt_dataset);
	if (dataset_in == NULL)
		throw std::runtime_error("input dataset is empty");
	MknnDataset *dataset_out = convert_dataset(dataset_in, opt_convert);
	if (dataset_out == NULL)
		export_dataset(opt_export, dataset_in);
	else
		export_dataset(opt_export, dataset_out);
	return EXIT_SUCCESS;
}
