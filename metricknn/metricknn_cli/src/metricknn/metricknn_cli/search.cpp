/*
 * Copyright (C) 2014-2015, ORAND S.A. <http://www.orand.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "metricknn_cli.hpp"

static MknnIndex *load_index(OptionsIndex &options, MknnDataset *reference,
		MknnDistance *distance) {
	MknnIndex *index = NULL;
	if (options.load_file != "") {
		index = mknn_index_restore(options.load_file.c_str(), reference, true,
				distance, true, NULL, true);
	} else {
		std::string param =
				(options.string_index == "") ?
						default_index : options.string_index;
		index = mknn_index_newPredefined(
				mknn_indexParams_newParseString(param.c_str()),
				true, reference, true, distance, true);
		if (options.save_file != "")
			mknn_index_save(index, options.save_file.c_str());
	}
	return index;
}
static MknnResolverParams *load_resolver_params(OptionsSearch &options) {
	int knn = default_knn;
	double range = default_range;
	int max_threads = my::parallel::getNumberOfCores();
	if (options.knn == "")
		knn = my::parse::stringToInt(options.knn);
	if (options.range == "")
		range = my::parse::stringToDouble(options.range);
	if (options.max_threads == "")
		max_threads = my::parse::stringToInt(options.max_threads);
	MknnResolverParams *params_resolver = mknn_resolverParams_newEmpty();
	mknn_resolverParams_setKnn(params_resolver, knn);
	mknn_resolverParams_setRange(params_resolver, range);
	mknn_resolverParams_setMaxThreads(params_resolver, max_threads);
	mknn_resolverParams_parseString(params_resolver,
			options.string_search.c_str());
	return params_resolver;
}
static std::string print_command_line(std::vector<std::string> &args) {
	std::stringstream ss;
	for (int i = 0; i < (int) args.size(); ++i) {
		if (i > 0)
			ss << " ";
		std::string cmd = args[i];
		bool has_spaces = my::string::indexOf(cmd, " ") >= 0
				|| my::string::indexOf(cmd, ";") >= 0;
		if (has_spaces)
			ss << "\"";
		ss << cmd;
		if (has_spaces)
			ss << "\"";
	}
	return ss.str();
}
static void print_summary_start(MknnDataset *query_dataset,
		MknnResolver *resolver) {
	MknnIndex *index = mknn_resolver_getIndex(resolver);
	MknnDataset *search_dataset = mknn_index_getSearchDataset(index);
	MknnDistance *distance = mknn_index_getDistance(index);
	std::cout << " query objects=" << mknn_dataset_getNumObjects(query_dataset)
			<< std::endl << " search objects="
			<< mknn_dataset_getNumObjects(search_dataset) << std::endl
			<< " distance="
			<< mknn_distanceParams_toString(
					mknn_distance_getParameters(distance)) << std::endl
			<< " index="
			<< mknn_indexParams_toString(mknn_index_getParameters(index))
			<< std::endl << " kNN="
			<< mknn_resolverParams_getKnn(mknn_resolver_getParameters(resolver))
			<< std::endl << " range="
			<< my::toString::doubleValue(
					mknn_resolverParams_getRange(
							mknn_resolver_getParameters(resolver))) << std::endl
			<< " threads="
			<< mknn_resolverParams_getMaxThreads(
					mknn_resolver_getParameters(resolver)) << std::endl;
}
static void print_summary_end(MknnResult *result) {
	MknnResolver *resolver = mknn_result_getResolver(result);
	int64_t max_threads = mknn_resolverParams_getMaxThreads(
			mknn_resolver_getParameters(resolver));
	MknnDataset *search_dataset = mknn_index_getSearchDataset(
			mknn_resolver_getIndex(resolver));
	std::cout << mknn_result_getNumQueries(result) << " query objects, "
			<< mknn_dataset_getNumObjects(search_dataset) << " search objects, "
			<< my::toString::hhmmssfff(mknn_result_getTotalSearchTime(result))
			<< " elapsed time, "
			<< mknn_result_getTotalDistanceEvaluations(result)
			<< " distance evaluations, " << max_threads << " threads"
			<< std::endl;
}
static void print_query(MknnDataset *query_dataset, int64_t i,
		MknnDataset *descriptionQ, MknnPrinter *printerQ, std::ofstream &out) {
	std::string stId = my::toString::intValue(i);
	std::string desQ = "";
	if (descriptionQ != NULL && i < mknn_dataset_getNumObjects(descriptionQ)) {
		char *txt = (char*) mknn_dataset_getObject(descriptionQ, i);
		desQ = std::string(txt);
	}
	std::string prQ = "";
	if (printerQ != NULL) {
		void *object = mknn_dataset_getObject(query_dataset, i);
		char *txt = mknn_printer_objectToNewString(printerQ, object);
		prQ = std::string(txt);
		free(txt);
	}
	std::stringstream ss;
	ss << "id_query=" << stId;
	if (desQ != "")
		ss << "\tdescription=" << desQ;
	if (prQ != "")
		ss << "\tquery=" << prQ;
	if (out.is_open())
		out << ss.str() << std::endl;
	else
		std::cout << ss.str() << std::endl;
}
static void print_nn(bool with_headers, MknnDataset *search_dataset, int64_t j,
		int64_t position, double distance, MknnDataset *descriptionObj,
		MknnPrinter *printerR, std::ofstream &out) {
	std::string stRank = my::toString::intValue(j + 1);
	std::string stPos = my::toString::intValue(position);
	std::string stDist = my::toString::doubleValue(distance);
	std::string desObj = "";
	if (descriptionObj != NULL
			&& position < mknn_dataset_getNumObjects(descriptionObj)) {
		char* txt = (char*) mknn_dataset_getObject(descriptionObj, position);
		desObj = std::string(txt);
	}
	std::string stObj = "";
	if (printerR != NULL) {
		void *object = mknn_dataset_getObject(search_dataset, position);
		char *txt = mknn_printer_objectToNewString(printerR, object);
		stObj = std::string(txt);
		free(txt);
	}
	std::stringstream ss;
	ss << "\t" << (with_headers ? "rank=" : "") << stRank << "\t"
			<< (with_headers ? "id=" : "") << stPos << "\t"
			<< (with_headers ? "dist=" : "") << stDist;
	if (desObj != "")
		ss << "\t" << (with_headers ? "description=" : "") << desObj;
	if (stObj != "")
		ss << "\t" << (with_headers ? "object=" : "") << stObj;
	if (out.is_open())
		out << ss.str() << std::endl;
	else
		std::cout << ss.str() << std::endl;
}

static MknnDataset *load_description(std::string file_description,
		MknnDataset *obj_dataset) {
	if (file_description == "")
		return NULL;
	MknnDataset *description = mknn_datasetLoader_ParseStringsFile(
			file_description.c_str());
	int64_t size1 = mknn_dataset_getNumObjects(obj_dataset);
	int64_t size2 = mknn_dataset_getNumObjects(description);
	if (size1 != size2) {
		throw std::runtime_error(
				"ERROR: lines in " + file_description
						+ " do not coincide with number of query objects ("
						+ my::toString::intValue(size2) + " objects but "
						+ my::toString::intValue(size1) + " lines)");
	}
	return description;
}

static MknnPrinter *load_printer(std::string print_objects,
		MknnDataset *obj_dataset) {
	MknnDomain *dom = mknn_dataset_getDomain(obj_dataset);
	if (print_objects == "yes"
			|| (print_objects == "" && mknn_domain_isGeneralDomainVector(dom)
					&& mknn_domain_vector_getNumDimensions(dom) < 10)) {
		MknnPrinter *printer = mknn_printer_new(dom);
		mknn_printer_setVectorFormat(printer, "(", ",", ")");
		mknn_printer_setMultiobjectFormat(printer, "{", ";", "}");
		return printer;
	}
	return NULL;
}
static void print_results(OptionsOutput &opt_output, MknnResult *result) {
	print_summary_end(result);
	if (opt_output.print_knn == "no")
		return;
	MknnResolver *resolver = mknn_result_getResolver(result);
	MknnDataset *query_dataset = mknn_result_getQueryDataset(result);
	//automatic option prints only when the number of knn and query object are not too much
	if (opt_output.print_knn == ""
			&& mknn_resolverParams_getKnn(mknn_resolver_getParameters(resolver))
					* mknn_dataset_getNumObjects(query_dataset) > 50)
		return;
	MknnIndex *index = mknn_resolver_getIndex(resolver);
	MknnDataset *search_dataset = mknn_index_getSearchDataset(index);
	MknnDataset *descriptionQ = load_description(
			opt_output.file_description_query, query_dataset);
	MknnDataset *descriptionR = load_description(
			opt_output.file_description_reference, search_dataset);
	MknnPrinter *printerQ = load_printer(opt_output.print_objects_query,
			query_dataset);
	MknnPrinter *printerR = load_printer(opt_output.print_objects_reference,
			search_dataset);
	std::ofstream out;
	if (opt_output.file_output != "") {
		out.open(opt_output.file_output);
		std::string line = print_command_line(opt_output.args);
		out << "#" << line << std::endl;
		out << "#\trank\tid_object\tdistance" << std::endl;
	}
	bool with_headers = (opt_output.file_output == "");
	for (int64_t i = 0; i < mknn_result_getNumQueries(result); ++i) {
		print_query(query_dataset, i, descriptionQ, printerQ, out);
		MknnResultQuery *resq = mknn_result_getResultQuery(result, i);
		for (int64_t j = 0; j < resq->num_nns; ++j) {
			print_nn(with_headers, search_dataset, j, resq->nn_position[j],
					resq->nn_distance[j], descriptionR, printerR, out);
		}
	}
	out.close();
}
void search_process(OptionsDataset &opt_query, OptionsDataset &opt_ref,
		OptionsDistance &opt_dist, OptionsIndex &opt_index,
		OptionsSearch &opt_search, OptionsOutput &opt_output) {
	std::cout << "loading query dataset" << std::endl;
	MknnDataset *query_dataset = load_dataset(opt_query);
	if (query_dataset == NULL)
		throw std::runtime_error("query dataset is empty");
	std::cout << "loading reference dataset" << std::endl;
	MknnDataset *search_dataset = load_dataset(opt_ref);
	if (search_dataset == NULL)
		throw std::runtime_error("reference dataset is empty");
	std::cout << "loading index" << std::endl;
	MknnDistance *distance = load_distance(opt_dist);
	if (distance == NULL)
		throw std::runtime_error("must enter the distance to compare objects.");
	MknnIndex *index = load_index(opt_index, search_dataset, distance);
	MknnResolverParams *params_resolver = load_resolver_params(opt_search);
	MknnResolver *resolver = mknn_index_newResolver(index, params_resolver,
	true);
	print_summary_start(query_dataset, resolver);
	std::cout << my::timer::currentClock() << " starting search" << std::endl;
	MknnResult *result = mknn_resolver_search(resolver, true, query_dataset,
	true);
	std::cout << my::timer::currentClock() << " search completed" << std::endl;
	print_results(opt_output, result);
	mknn_result_release(result);
	mknn_index_release(index);
}
