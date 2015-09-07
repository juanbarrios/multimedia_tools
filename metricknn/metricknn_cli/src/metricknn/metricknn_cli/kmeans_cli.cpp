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

class OptionsKmeans {
public:
	int numCenters;
	int max_threads;
	std::string paramIndex, paramResolver;
	std::string save_state;
	double save_state_timer;
	std::string load_state;
	double endOnMovedVectors, endOnMovedCenters, endAtSeconds;
	long long endAtIteration;
	std::vector<double> incrementalSizes;bool useDefaultIncrementalSizes;bool
			init_random, init_sss;
	std::string save_centroids;
	std::string save_assignations;
	std::string parseCentersFilename;
	std::string centersDatatype;
	OptionsKmeans() :
			numCenters(0), max_threads(0), save_state_timer(0), endOnMovedVectors(
					0), endOnMovedCenters(0), endAtSeconds(0), endAtIteration(
					0), useDefaultIncrementalSizes(
			false), init_random(
			false), init_sss(
			false) {
	}
};

static double default_save_state_timer = 180;
static std::string default_mknn_index = "LINEARSCAN";
static std::string default_mknn_resolver = "";

static void print_kmeans_options(bool detailed) {
	std::cout << "    -numCenters [num]" << std::endl;
	if (detailed) {
		std::cout << "       Number of centroids to compute." << std::endl;
	}
	std::cout << "    -saveState [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to save kmeans state.\n" << std::endl;
	std::cout << "    -saveStateTimer [num]" << std::endl;
	if (detailed)
		std::cout << "        Seconds to save kmeans state. Default="
				<< default_save_state_timer << "\n" << std::endl;
	std::cout << "    -loadState [filename]" << std::endl;
	if (detailed)
		std::cout << "        Filename to load kmeans state.\n" << std::endl;
	std::cout << "    -initCentersRandom" << std::endl;
	if (detailed)
		std::cout
				<< "        Initialize centroids with random sample of vectors.\n"
				<< std::endl;
	std::cout << "    -initCentersSSS" << std::endl;
	if (detailed)
		std::cout
				<< "        Initialize centroids with Sparse Spatial Selection of vectors.\n"
				<< std::endl;
	std::cout << "    -parseCentersFilename [filename]" << std::endl;
	if (detailed)
		std::cout << "        Read text file and initialize centroids.\n"
				<< std::endl;
	std::cout << "    -centersDatatype [datatype]" << std::endl;
	if (detailed) {
		std::cout << "        The datatype of centroids.\n" << std::endl;
		print_datatypes("        ");
	}
	std::cout << "    -endOnMovedVectors [fraction or number]" << std::endl;
	if (detailed)
		std::cout
				<< "        Terminate if moved vectors are less than value. If between 0 and 1 means a fraction of total size, otherwise is the size number.\n"
				<< std::endl;
	std::cout << "    -endOnMovedCenters [fraction or number]" << std::endl;
	if (detailed)
		std::cout
				<< "        Terminate if moved centers are less than value. If between 0 and 1 means a fraction of total centers, otherwise is the number of centers.\n"
				<< std::endl;
	std::cout << "    -endAtIteration [number]" << std::endl;
	if (detailed)
		std::cout << "        Terminate at iteration number.\n" << std::endl;
	std::cout << "    -endAtSeconds [number]" << std::endl;
	if (detailed)
		std::cout
				<< "        Terminate when the processing time is equal or higher than the given number of seconds.\n"
				<< std::endl;
	std::cout << "    -useDefaultIncrementalSizes" << std::endl;
	if (detailed)
		std::cout
				<< "        Compute K-Means on random subsets with default incremental sizes.\n"
				<< std::endl;
	std::cout << "    -addIncrementalSize [fraction or number]" << std::endl;
	if (detailed)
		std::cout
				<< "        Compute K-Means on a random subsets with the given size. If between 0 and 1 means a fraction of total size, otherwise is the size number.\n"
				<< std::endl;
	std::cout << "    -printCentroids [filename]" << std::endl;
	if (detailed)
		std::cout << "        Save centroids to filename.\n" << std::endl;
	std::cout << "    -printAssignations [filename]" << std::endl;
	if (detailed)
		std::cout << "        Save assignations to filename.\n" << std::endl;
	std::cout << "    -maxThreads [num]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Maximum number of parallel threads to resolve clustering."
				<< std::endl;
		std::cout << "       Default: number of cores.\n" << std::endl;
	}
	std::cout << "    -paramIndex [index_string]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Index to compute between centroids at each iteration."
				<< std::endl;
		std::cout << "       Default: " << default_mknn_index << ".\n"
				<< std::endl;
	}
	std::cout << "    -paramResolver [resolver_string]" << std::endl;
	if (detailed) {
		std::cout
				<< "       Resolver to create at the 1-NN search at each vector at each iteration."
				<< std::endl;
		std::cout << "       Default: " << default_mknn_resolver << ".\n"
				<< std::endl;
	}
	std::cout << "    -list_indexes" << std::endl;
	if (detailed)
		std::cout << "        Lists all pre-defined indexes and exits.\n"
				<< std::endl;
	std::cout << "    -help_index [id_index]" << std::endl;
	if (detailed)
		std::cout << "        Prints help for a given index and exits.\n"
				<< std::endl;
}
static void print_help(std::vector<std::string> &args, bool detailed) {
	std::cout << "Usage: " << my::collection::args_getBinaryName(args) << " "
			<< args.at(1) << " [options] ..." << std::endl << std::endl;
	std::cout << "    -help" << std::endl;
	if (detailed)
		std::cout << "       Shows this detailed help.\n" << std::endl;
#ifndef NO_OPENCV
	std::cout << "    -demo ..." << std::endl;
	if (detailed)
		std::cout << "       Runs a demo of kmeans algorithms.\n" << std::endl;
#endif
	std::cout << std::endl << "DATASET OPTIONS" << std::endl;
	print_dataset_options(detailed, "");
	std::cout << "" << std::endl;
	std::cout << std::endl << "DISTANCE OPTIONS" << std::endl;
	print_distance_options(detailed);
	std::cout << "" << std::endl;
	std::cout << std::endl << "KMEANS OPTIONS" << std::endl;
	print_kmeans_options(detailed);
	std::cout << "" << std::endl;
}

static bool parse_kmeans_opt(OptionsKmeans &opt, std::vector<std::string> &args,
		unsigned int &i) {
	if (my::collection::is_next_arg_equal("-numCenters", args, i)) {
		opt.numCenters = my::collection::next_arg_int(args, i);
	} else if (my::collection::is_next_arg_equal("-saveState", args, i)) {
		opt.save_state = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-saveStateTimer", args, i)) {
		opt.save_state_timer = my::collection::next_arg_double(args, i);
	} else if (my::collection::is_next_arg_equal("-loadState", args, i)) {
		opt.load_state = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(opt.load_state);
	} else if (my::collection::is_next_arg_equal("-initCentersRandom", args,
			i)) {
		opt.init_random = true;
	} else if (my::collection::is_next_arg_equal("-initCentersSSS", args, i)) {
		opt.init_sss = true;
	} else if (my::collection::is_next_arg_equal("-parseCentersFilename", args,
			i)) {
		opt.parseCentersFilename = my::collection::next_arg(args, i);
		VALIDATE_FILE_EXISTS(opt.parseCentersFilename);
	} else if (my::collection::is_next_arg_equal("-centersDatatype", args, i)) {
		opt.centersDatatype = my::collection::next_arg(args, i);
		VALIDATE_DATATYPE(opt.centersDatatype);
	} else if (my::collection::is_next_arg_equal("-endOnMovedVectors", args,
			i)) {
		opt.endOnMovedVectors = my::collection::next_arg_double(args, i);
	} else if (my::collection::is_next_arg_equal("-endOnMovedCenters", args,
			i)) {
		opt.endOnMovedCenters = my::collection::next_arg_double(args, i);
	} else if (my::collection::is_next_arg_equal("-endAtIteration", args, i)) {
		opt.endAtIteration = my::collection::next_arg_int(args, i);
	} else if (my::collection::is_next_arg_equal("-endAtSeconds", args, i)) {
		opt.endAtSeconds = my::collection::next_arg_double(args, i);
	} else if (my::collection::is_next_arg_equal("-useDefaultIncrementalSizes",
			args, i)) {
		opt.useDefaultIncrementalSizes = true;
	} else if (my::collection::is_next_arg_equal("-printCentroids", args, i)) {
		opt.save_centroids = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-printAssignations", args,
			i)) {
		opt.save_assignations = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-maxThreads", args, i)) {
		opt.max_threads = my::collection::next_arg_int(args, i);
	} else if (my::collection::is_next_arg_equal("-paramIndex", args, i)) {
		opt.paramIndex = my::collection::next_arg(args, i);
	} else if (my::collection::is_next_arg_equal("-paramResolver", args, i)) {
		opt.paramResolver = my::collection::next_arg(args, i);
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
static void run_kmeans(OptionsDataset &opt_dataset, OptionsDistance &opt_dist,
		OptionsKmeans &opt_kmeans) {
	MknnDataset *dataset = load_dataset(opt_dataset);
	if (dataset == NULL)
		throw std::runtime_error("dataset is empty");
	MknnDistance *distance = load_distance(opt_dist);
	MknnKmeansAlgorithm *kmeans = mknn_kmeans_new();
	mknn_kmeans_setDataset(kmeans, dataset);
	if (opt_kmeans.centersDatatype != "") {
		MknnDatatype dt = GET_DATATYPE(opt_kmeans.centersDatatype);
		mknn_kmeans_setDefaultCentroidsDatatype(kmeans, dt);
	}
	if (my::io::existsFile(opt_kmeans.load_state)) {
		std::cout << "loading kmeans state in " << opt_kmeans.load_state
				<< std::endl;
		mknn_kmeans_loadState(kmeans, opt_kmeans.load_state.c_str());
	} else {
		if (distance == NULL)
			throw std::runtime_error(
					"must enter the distance to compare objects.");
		if (opt_kmeans.numCenters == 0)
			throw std::runtime_error(
					"must enter the number of centroids to compute.");
		mknn_kmeans_setDistance(kmeans, distance);
		mknn_kmeans_setNumCentroids(kmeans, opt_kmeans.numCenters);
		if (my::io::existsFile(opt_kmeans.parseCentersFilename)) {
			MknnDataset *centroids = mknn_datasetLoader_ParseVectorFile(
					opt_kmeans.parseCentersFilename.c_str(),
					mknn_kmeans_getDefaultCentroidsDatatype(kmeans));
			mknn_kmeans_setInitialCentroids(kmeans, centroids, true);
		} else if (opt_kmeans.init_random) {
			mknn_kmeans_initCentroidsRandom(kmeans);
		} else if (opt_kmeans.init_sss) {
			mknn_kmeans_initCentroidsSSS(kmeans);
		}
	}
	if (opt_kmeans.max_threads > 0)
		mknn_kmeans_setMaxThreads(kmeans, opt_kmeans.max_threads);
	if (opt_kmeans.paramIndex != "")
		mknn_kmeans_setParametersMknnIndex(kmeans,
				opt_kmeans.paramIndex.c_str());
	if (opt_kmeans.paramResolver != "")
		mknn_kmeans_setParametersMknnResolver(kmeans,
				opt_kmeans.paramResolver.c_str());
	mknn_kmeans_setTermitationCriteria(kmeans, opt_kmeans.endAtIteration,
			opt_kmeans.endAtSeconds, opt_kmeans.endOnMovedVectors,
			opt_kmeans.endOnMovedCenters);
	if (opt_kmeans.useDefaultIncrementalSizes)
		mknn_kmeans_addDefaultSubsetRuns(kmeans);
	if (opt_kmeans.incrementalSizes.size() > 0) {
		for (double size : opt_kmeans.incrementalSizes) {
			mknn_kmeans_addSubsetRun(kmeans, size, opt_kmeans.endAtIteration,
					opt_kmeans.endAtSeconds, opt_kmeans.endOnMovedVectors,
					opt_kmeans.endOnMovedCenters);
		}
	}
	if (opt_kmeans.save_state != "")
		mknn_kmeans_setAutoSaveState(kmeans, opt_kmeans.save_state.c_str(),
				(opt_kmeans.save_state_timer == 0) ?
						default_save_state_timer : opt_kmeans.save_state_timer);
	mknn_kmeans_perform(kmeans);
	if (opt_kmeans.save_centroids != "") {
		MknnDataset *centroids = mknn_kmeans_getCentroids(kmeans, false);
		mknn_dataset_printObjectsTextFile(centroids,
				opt_kmeans.save_centroids.c_str());
	}
	if (opt_kmeans.save_assignations != "") {
		int64_t *assignations = mknn_kmeans_getAssignations(kmeans, false);
		std::vector<std::string> lines;
		int64_t size = mknn_dataset_getNumObjects(dataset);
		lines.reserve(size);
		for (int64_t i = 0; i < size; ++i)
			lines.push_back(my::toString::intValue(assignations[i]));
		my::io::saveLinesFile(opt_kmeans.save_assignations.c_str(), lines);
	}
	mknn_kmeans_release(kmeans);
}

int main_demo_kmeans(std::vector<std::string> &args, unsigned int i);

int main_kmeans(std::vector<std::string> &args) {
	if (args.size() < 3) {
		print_help(args, false);
		return EXIT_FAILURE;
	}
	OptionsDataset opt_dataset;
	OptionsDistance opt_dist;
	OptionsKmeans opt_kmeans;
	//read parameters
	unsigned int i = 2;
	while (i < args.size()) {
		if (my::collection::is_next_arg_equal("-help", args, i)) {
			print_help(args, true);
			return EXIT_SUCCESS;
#ifndef NO_OPENCV
		} else if (my::collection::is_next_arg_equal("-demo", args, i)) {
			main_demo_kmeans(args, i);
			return EXIT_SUCCESS;
#endif
		} else if (parse_dataset_opt(opt_dataset, "", args, i)) {
			continue;
		} else if (parse_distance_opt(opt_dist, args, i)) {
			continue;
		} else if (parse_kmeans_opt(opt_kmeans, args, i)) {
			continue;
		} else {
			throw std::runtime_error(
					"unknown parameter " + my::collection::next_arg(args, i));
		}
	}
	run_kmeans(opt_dataset, opt_dist, opt_kmeans);
	return EXIT_SUCCESS;
}
