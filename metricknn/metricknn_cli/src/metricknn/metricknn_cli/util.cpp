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

void print_datatypes(std::string pref) {
	std::cout << pref << "Valid datatypes are:" << std::endl;
	std::cout << pref << "  Floating point types: FLOAT, DOUBLE" << std::endl;
	std::cout << pref << "  Integer signed types: INT8, INT16, INT32, INT64"
			<< std::endl;
	std::cout << pref
			<< "  Integer unsigned types: UINT8, UINT16, UINT32, UINT64"
			<< std::endl;
}

void VALIDATE_FILE_EXISTS(std::string filename) {
	if (filename != "" && !my::io::existsFile(filename)) {
		std::cout << "file " << filename << " does not exist" << std::endl;
		exit(EXIT_FAILURE);
	}
}
void VALIDATE_FILE_NOT_EXISTS(std::string filename) {
	if (filename != "" && my::io::existsFile(filename)) {
		std::cout << "file " << filename << " already exists" << std::endl;
		exit(EXIT_FAILURE);
	}
}
void VALIDATE_DATATYPE(std::string datatype) {
	MknnDatatype dtype;
	if (!mknn_datatype_parseString(datatype.c_str(), &dtype)) {
		std::cout << "Invalid datatype " << datatype << std::endl;
		print_datatypes("");
		exit(EXIT_FAILURE);
	}
}
MknnDatatype GET_DATATYPE(std::string datatype) {
	MknnDatatype dtype;
	if (!mknn_datatype_parseString(datatype.c_str(), &dtype)) {
		std::cout << "Invalid datatype " << datatype << std::endl;
		print_datatypes("");
		exit(EXIT_FAILURE);
	}
	return dtype;
}
static MknnDataset *load_input(InputData &input) {
	if (input.dataset_filename != "") {
		std::cout << "loading file " << input.dataset_filename << std::endl;
		return mknn_dataset_restore(input.dataset_filename.c_str());
	} else if (input.strings_filename != "") {
		std::cout << "loading file " << input.strings_filename << std::endl;
		return mknn_datasetLoader_ParseStringsFile(
				input.strings_filename.c_str());
	} else if (input.vectors_text.filename != "") {
		VALIDATE_DATATYPE(input.vectors_text.datatype);
		MknnDatatype dtype = GET_DATATYPE(input.vectors_text.datatype);
		std::cout << "loading file " << input.vectors_text.filename
				<< std::endl;
		return mknn_datasetLoader_ParseVectorFile(
				input.vectors_text.filename.c_str(), dtype);
	} else if (input.vectors_raw.filename != "") {
		MknnDatatype dtype = GET_DATATYPE(input.vectors_raw.datatype);
		long long rows = my::parse::stringToInt(input.vectors_raw.rows);
		long long columns = my::parse::stringToInt(input.vectors_raw.columns);
		long long expected_size = rows * columns * mknn_datatype_sizeof(dtype);
		long long filesize = my::io::getFilesize(
				input.vectors_raw.filename.c_str());
		if (expected_size != filesize)
			throw std::runtime_error(
					"error file size " + input.vectors_raw.filename
							+ " does not match expected size: "
							+ my::toString::intValue(expected_size) + " != "
							+ my::toString::intValue(filesize));
		std::cout << "loading file " << input.vectors_raw.filename << std::endl;
		int64_t size = 0;
		void *data = my_io_loadFileBytes(input.vectors_raw.filename.c_str(),
				&size);
		if (size != filesize)
			throw std::runtime_error(
					"can't read bytes from " + input.vectors_raw.filename + " ("
							+ my::toString::intValue(size) + " != "
							+ my::toString::intValue(filesize) + ")");
		return mknn_datasetLoader_PointerCompactVectors(data, true, rows,
				columns, dtype);
	} else if (input.vectors_random.num_vectors != "") {
		MknnDatatype dtype;
		if (!mknn_datatype_parseString(input.vectors_random.datatype.c_str(),
				&dtype))
			throw std::runtime_error(
					"error converting " + input.vectors_random.datatype);
		std::cout << "generating " << input.vectors_random.num_vectors
				<< " random vectors..." << std::endl;
		return mknn_datasetLoader_UniformRandomVectors(
				my::parse::stringToInt(input.vectors_random.num_vectors),
				my::parse::stringToInt(input.vectors_random.dimension),
				my::parse::stringToDouble(input.vectors_random.min_value),
				my::parse::stringToDouble(input.vectors_random.max_value),
				dtype);
	}
	return NULL;
}
MknnDataset *load_dataset(OptionsDataset &options) {
	MknnDataset *dataset = NULL;
	if (options.inputList.size() == 1) {
		dataset = load_input(options.inputList.at(0));
	} else if (options.inputList.size() > 1) {
		if (!options.combine_input && !options.concatenate_input)
			throw std::runtime_error(
					"The multi dataset must define concatenate or combine");
		int num = options.inputList.size();
		MknnDataset **datasets = new MknnDataset*[num];
		for (int i = 0; i < num; ++i) {
			datasets[i] = load_input(options.inputList.at(i));
			if (datasets[i] == NULL)
				throw std::runtime_error("could not load dataset");
		}
		if (options.concatenate_input) {
			std::cout << "concatenating datasets..." << std::endl;
			dataset = mknn_datasetLoader_Concatenate(num, datasets, true);
		} else if (options.combine_input) {
			std::cout << " combining modalities..." << std::endl;
			dataset = mknn_datasetLoader_MultiObject(num, datasets, true);
		}
		free(datasets);
	}
	if (dataset != NULL && options.randomSample != "") {
		std::cout << "computing random sample " << options.randomSample << "..."
				<< std::endl;
		double sample_size = my::parse::stringToDouble(options.randomSample);
		dataset = mknn_datasetLoader_SubsetRandomSample(dataset, sample_size,
		true);
	}
	return dataset;
}
MknnDistance *load_distance(OptionsDistance &options) {
	MknnDistance *distance = NULL;
	if (options.load_file != "") {
		distance = mknn_distance_restore(options.load_file.c_str(), NULL, true);
	} else if (options.string_distance != "") {
		distance = mknn_distance_newPredefined(
				mknn_distanceParams_newParseString(
						options.string_distance.c_str()),
				true);
		if (options.save_file != "")
			mknn_distance_save(distance, options.save_file.c_str());
	}
	return distance;
}
