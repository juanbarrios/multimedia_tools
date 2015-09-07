/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

static void writeOutfile(const char *outFilename, CmdParams *cmd_params,
		const char *outFormat, MyVectorObj *queryTxts, int64_t maxNNWrite,
		double maxDistWrite) {
	char *nameSegments = NULL;
	if (my_string_equals(outFormat, "SS"))
		nameSegments = "segments";
	if (my_string_equals(outFormat, "SSVector"))
		nameSegments = "vectors";
	FILE *out = my_io_openFileWrite1Config(outFilename, "PVCD", outFormat, 1,
			1);
	char *stCmd = get_command_line(cmd_params);
	fprintf(out, "#%s\n", stCmd);
	MY_FREE(stCmd);
	for (int64_t i = 0; i < my_vectorObj_size(queryTxts); ++i) {
		struct QueryTxt *query = my_vectorObj_get(queryTxts, i);
		fprintf(out, "query=%s\t%s=%"PRIi64"\tsearchTime=%1.1lf\n",
				query->name_file, nameSegments, query->num_queries,
				query->search_time);
		for (int64_t j = 0; j < query->num_queries; ++j) {
			fprintf(out, "%s", query->name_query[j]);
			bool flagAdded = false;
			for (int64_t k = 0; k < my_vectorObj_size(query->nns[j]); ++k) {
				struct NNTxt *nn = my_vectorObj_get(query->nns[j], k);
				if (k >= maxNNWrite || nn->distance > maxDistWrite)
					break;
				char *st = my_newString_double(nn->distance);
				fprintf(out, "\t%s\t%s", nn->name_nn, st);
				MY_FREE(st);
				flagAdded = true;
			}
			if (!flagAdded)
				fprintf(out, "\t");
			fprintf(out, "\n");
		}
	}
	fclose(out);
}
static int compareNNs(const void*a, const void*b) {
	struct NNTxt *na = *(struct NNTxt**) a;
	struct NNTxt *nb = *(struct NNTxt**) b;
	return my_compare_double(na->distance, nb->distance);
}
#define MIN_DIST_IDENTICAL_OBJECTS 1
static void mergeQueries(struct QueryTxt *queryBase, struct QueryTxt *queryAdd,
		int64_t maxNN) {
	if (queryBase->num_queries != queryAdd->num_queries)
		my_log_error("numVectors do not match %"PRIi64" != %"PRIi64"\n",
				queryBase->num_queries, queryAdd->num_queries);
	for (int64_t i = 0; i < queryBase->num_queries; ++i) {
		my_assert_equalString("nameVectors", queryBase->name_query[i],
				queryAdd->name_query[i]);
		MyVectorObj *baseNNs = queryBase->nns[i];
		MyVectorObj *newNNs = queryAdd->nns[i];
#if MIN_DIST_IDENTICAL_OBJECTS
		for (int64_t j = 0; j < my_vectorObj_size(baseNNs); ++j) {
			struct NNTxt *nnBase = my_vectorObj_get(baseNNs, j);
			for (int64_t k = 0; k < my_vectorObj_size(newNNs); ++k) {
				struct NNTxt *nnNew = my_vectorObj_get(newNNs, k);
				if (my_string_equals(nnBase->name_nn, nnNew->name_nn)) {
					if (nnNew->distance < nnBase->distance)
						nnBase->distance = nnNew->distance;
					my_vectorObj_remove(newNNs, k);
					k--;
				}
			}
		}
#endif
		my_vectorObj_addAll(baseNNs, newNNs);
		my_vectorObj_qsort(baseNNs, compareNNs);
		while (my_vectorObj_size(baseNNs) > maxNN) {
			struct NNTxt *nn = my_vectorObj_remove(baseNNs,
					my_vectorObj_size(baseNNs) - 1);
			MY_FREE_MULTI(nn->name_nn, nn);
		}
		my_vectorObj_release(queryAdd->nns[i], 0);
		MY_FREE(queryAdd->name_query[i]);
	}
}

static void addSearchTime(MyMapStringObj *name2searchTimes,
		struct QueryTxt *qAdd) {
	MyVectorDouble *times = my_mapStringObj_get(name2searchTimes,
			qAdd->name_file);
	if (times == NULL) {
		times = my_vectorDouble_new();
		my_mapStringObj_add(name2searchTimes, qAdd->name_file, times);
	}
	my_vectorDouble_add(times, qAdd->search_time);
}
static void updateSearchTimes(MyMapStringObj *name2query,
		MyMapStringObj *name2searchTimes, const char *typeSearchTime) {
	MyVectorObj *list = my_mapStringObj_valuesVector(name2query);
	for (int64_t i = 0; i < my_vectorObj_size(list); ++i) {
		struct QueryTxt *query = my_vectorObj_get(list, i);
		MyVectorDouble *times = my_mapStringObj_get(name2searchTimes,
				query->name_file);
		if (my_string_equals(typeSearchTime, "SUM")) {
			double sum = 0;
			for (int64_t j = 0; j < my_vectorDouble_size(times); ++j) {
				sum += my_vectorDouble_get(times, j);
			}
			query->search_time = sum;
		} else if (my_string_equals(typeSearchTime, "AVG")) {
			double sum = 0;
			for (int64_t j = 0; j < my_vectorDouble_size(times); ++j) {
				sum += my_vectorDouble_get(times, j);
			}
			query->search_time = my_math_round_int(
					sum / (double) my_vectorDouble_size(times));
		} else if (my_string_equals(typeSearchTime, "MAX")) {
			double max = 0;
			for (int64_t j = 0; j < my_vectorDouble_size(times); ++j) {
				double t = my_vectorDouble_get(times, j);
				if (t > max)
					max = t;
			}
			query->search_time = max;
		} else
			my_log_error("unknown typeSearchTime %s\n", typeSearchTime);
		my_vectorDouble_release(times);
	}
	my_vectorObj_release(list, false);
	my_mapStringObj_release(name2searchTimes, false, false);
}
static int64_t getMaxNNLength(MyVectorObj *ssFile) {
	int64_t maxLength = 0;
	for (int64_t i = 0; i < my_vectorObj_size(ssFile); ++i) {
		struct QueryTxt *qtxt = my_vectorObj_get(ssFile, i);
		for (int64_t j = 0; j < qtxt->num_queries; ++j) {
			MyVectorObj *nnsList = qtxt->nns[j];
			int64_t len = my_vectorObj_size(nnsList);
			if (len > maxLength)
				maxLength = len;
		}
	}
	return maxLength;
}
static void scaleDistances(MyVectorObj *ssFile, double scaleValue) {
	for (int64_t i = 0; i < my_vectorObj_size(ssFile); ++i) {
		struct QueryTxt *qtxt = my_vectorObj_get(ssFile, i);
		for (int64_t j = 0; j < qtxt->num_queries; ++j) {
			MyVectorObj *nnsList = qtxt->nns[j];
			for (int64_t k = 0; k < my_vectorObj_size(nnsList); ++k) {
				struct NNTxt *nn = my_vectorObj_get(nnsList, k);
				nn->distance *= scaleValue;
			}
		}
	}
}
#define NNWRITE_SAME_FIRST -2
static MyVectorObj * NN_mergeByConcatenation(MyVectorString *filenames,
		const char *outFilename, MyVectorInt *maxNNLoad,
		MyVectorDouble *maxDistLoad, int64_t *maxNNWrite,
		MyVectorDouble *distsNorm, MyVectorDouble *distsWeight,
		const char *typeSearchTime) {
	MyMapStringObj *name2query = my_mapStringObj_newCaseSensitive();
	MyMapStringObj *name2searchTimes = my_mapStringObj_newCaseSensitive();
	for (int64_t i = 0; i < my_vectorString_size(filenames); ++i) {
		const char *filename = my_vectorString_get(filenames, i);
		MyVectorObj *vq = loadSsFileTxt(filename,
				my_vectorInt_get(maxNNLoad, i),
				my_vectorDouble_get(maxDistLoad, i));
		double w = my_vectorDouble_get(distsNorm, i)
				* my_vectorDouble_get(distsWeight, i);
		if (w != 1)
			scaleDistances(vq, w);
		if (*maxNNWrite == NNWRITE_SAME_FIRST)
			*maxNNWrite = getMaxNNLength(vq);
		for (int64_t j = 0; j < my_vectorObj_size(vq); ++j) {
			struct QueryTxt *qAdd = my_vectorObj_get(vq, j);
			addSearchTime(name2searchTimes, qAdd);
			struct QueryTxt *qBase = my_mapStringObj_get(name2query,
					qAdd->name_file);
			if (qBase == NULL && i == 0) {
				my_mapStringObj_add(name2query, qAdd->name_file, qAdd);
			} else if (qBase == NULL && i > 0) {
				my_log_info("could not locate query '%s'\n", qAdd->name_file);
			} else {
				mergeQueries(qBase, qAdd, *maxNNWrite);
				MY_FREE_MULTI(qAdd->nns, qAdd->name_query, qAdd->name_file,
						qAdd);
			}
		}
	}
	updateSearchTimes(name2query, name2searchTimes, typeSearchTime);
	my_log_info_time("merge complete, %"PRIi64" queries, %"PRIi64" files\n",
			my_mapStringObj_size(name2query), my_vectorString_size(filenames));
	MyVectorObj *list = my_mapStringObj_valuesVector(name2query);
	my_mapStringObj_release(name2query, false, false);
	return list;
}
/***************/
static void mergeNns(MyVectorObj *baseNNs, MyVectorObj *addNNs,
		double distNotFound, int64_t numPrevs) {
	for (int64_t j = 0; j < my_vectorObj_size(baseNNs); ++j) {
		struct NNTxt *nnBase = my_vectorObj_get(baseNNs, j);
		bool found = false;
		for (int64_t k = 0; k < my_vectorObj_size(addNNs); ++k) {
			struct NNTxt *nnAdd = my_vectorObj_get(addNNs, k);
			if (my_string_equals(nnBase->name_nn, nnAdd->name_nn)) {
				nnBase->distance += nnAdd->distance;
				my_vectorObj_remove(addNNs, k);
				found = true;
				break;
			}
		}
		if (!found)
			nnBase->distance += distNotFound;
	}
	for (int64_t k = 0; k < my_vectorObj_size(addNNs); ++k) {
		struct NNTxt *nnAdd = my_vectorObj_get(addNNs, k);
		nnAdd->distance += distNotFound * numPrevs;
		my_vectorObj_add(baseNNs, nnAdd);
	}
	my_vectorObj_qsort(baseNNs, compareNNs);
}
static void mergeCommonSegments(struct QueryTxt *queryBase,
		struct QueryTxt *queryAdd, double distNotFound, int64_t numPrevs) {
	if (queryBase->num_queries != queryAdd->num_queries)
		my_log_error("numVectors do not match %"PRIi64" != %"PRIi64"\n",
				queryBase->num_queries, queryAdd->num_queries);
	for (int64_t i = 0; i < queryBase->num_queries; ++i) {
		my_assert_equalString("nameVectors", queryBase->name_query[i],
				queryAdd->name_query[i]);
		mergeNns(queryBase->nns[i], queryAdd->nns[i], distNotFound, numPrevs);
	}
}
static MyVectorObj * NN_mergeByDistSum(MyVectorString *filenames,
		const char *outFilename, MyVectorInt *maxNNLoad,
		MyVectorDouble *maxDistLoad, int64_t *maxNNWrite,
		MyVectorDouble *distsNorm, MyVectorDouble *distsWeight,
		double distNotFound, const char *typeSearchTime) {
	MyMapStringObj *name2query = my_mapStringObj_newCaseSensitive();
	MyMapStringObj *name2searchTimes = my_mapStringObj_newCaseSensitive();
	for (int64_t i = 0; i < my_vectorString_size(filenames); ++i) {
		const char *filename = my_vectorString_get(filenames, i);
		MyVectorObj *vq = loadSsFileTxt(filename,
				my_vectorInt_get(maxNNLoad, i),
				my_vectorDouble_get(maxDistLoad, i));
		double w = my_vectorDouble_get(distsNorm, i)
				* my_vectorDouble_get(distsWeight, i);
		if (w != 1)
			scaleDistances(vq, w);
		if (*maxNNWrite == NNWRITE_SAME_FIRST)
			*maxNNWrite = getMaxNNLength(vq);
		for (int64_t j = 0; j < my_vectorObj_size(vq); ++j) {
			struct QueryTxt *qAdd = my_vectorObj_get(vq, j);
			addSearchTime(name2searchTimes, qAdd);
			struct QueryTxt *qBase = my_mapStringObj_get(name2query,
					qAdd->name_file);
			if (qBase == NULL && i == 0) {
				my_mapStringObj_add(name2query, qAdd->name_file, qAdd);
			} else if (qBase == NULL && i > 0) {
				my_log_info("could not locate query '%s'\n", qAdd->name_file);
			} else {
				mergeCommonSegments(qBase, qAdd, distNotFound, i);
			}
		}
	}
	updateSearchTimes(name2query, name2searchTimes, typeSearchTime);
	my_log_info_time("merge complete, %"PRIi64" queries, %"PRIi64" files\n",
			my_mapStringObj_size(name2query), my_vectorString_size(filenames));
	MyVectorObj *list = my_mapStringObj_valuesVector(name2query);
	my_mapStringObj_release(name2query, false, false);
	return list;
}
/******************/
static void convertDistanceToVote(struct QueryTxt *queryAdd) {
	for (int64_t i = 0; i < queryAdd->num_queries; ++i) {
		MyVectorObj *nns = queryAdd->nns[i];
		for (int64_t j = 0; j < my_vectorObj_size(nns); ++j) {
			struct NNTxt *nnBase = my_vectorObj_get(nns, j);
			nnBase->distance = 1 / (double) (j + 1);
		}
	}
}
static void sumVotes(MyVectorObj *baseNNs, MyVectorObj *addNNs) {
	for (int64_t j = 0; j < my_vectorObj_size(baseNNs); ++j) {
		struct NNTxt *nnBase = my_vectorObj_get(baseNNs, j);
		for (int64_t k = 0; k < my_vectorObj_size(addNNs); ++k) {
			struct NNTxt *nnAdd = my_vectorObj_get(addNNs, k);
			if (my_string_equals(nnBase->name_nn, nnAdd->name_nn)) {
				nnBase->distance += nnAdd->distance;
				my_vectorObj_remove(addNNs, k);
				break;
			}
		}
	}
	my_vectorObj_addAll(baseNNs, addNNs);
}

static void mergeListsSummingVotes(struct QueryTxt *queryBase,
		struct QueryTxt *queryAdd) {
	if (queryBase->num_queries != queryAdd->num_queries)
		my_log_error("numVectors do not match %"PRIi64" != %"PRIi64"\n",
				queryBase->num_queries, queryAdd->num_queries);
	for (int64_t i = 0; i < queryBase->num_queries; ++i) {
		my_assert_equalString("nameVectors", queryBase->name_query[i],
				queryAdd->name_query[i]);
		sumVotes(queryBase->nns[i], queryAdd->nns[i]);
	}
}
static void convertVotesToDistance(struct QueryTxt *queryAdd) {
	for (int64_t i = 0; i < queryAdd->num_queries; ++i) {
		MyVectorObj *nns = queryAdd->nns[i];
		my_vectorObj_qsort(nns, compareNNs);
		for (int64_t j = 0; j < my_vectorObj_size(nns); ++j) {
			struct NNTxt *nnBase = my_vectorObj_get(nns, j);
			nnBase->distance = my_vectorObj_size(nns) - j;
		}
		my_vectorObj_qsort(nns, compareNNs);
	}
}
static MyVectorObj *NN_mergeByRankAggregation(MyVectorString *filenames,
		const char *outFilename, MyVectorInt *maxNNLoad,
		MyVectorDouble *maxDistLoad, int64_t *maxNNWrite,
		const char *typeSearchTime) {
	MyMapStringObj *name2query = my_mapStringObj_newCaseSensitive();
	MyMapStringObj *name2searchTimes = my_mapStringObj_newCaseSensitive();
	for (int64_t i = 0; i < my_vectorString_size(filenames); ++i) {
		const char *filename = my_vectorString_get(filenames, i);
		MyVectorObj *vq = loadSsFileTxt(filename,
				my_vectorInt_get(maxNNLoad, i),
				my_vectorDouble_get(maxDistLoad, i));
		if (*maxNNWrite == NNWRITE_SAME_FIRST)
			*maxNNWrite = getMaxNNLength(vq);
		for (int64_t j = 0; j < my_vectorObj_size(vq); ++j) {
			struct QueryTxt *qAdd = my_vectorObj_get(vq, j);
			convertDistanceToVote(qAdd);
			addSearchTime(name2searchTimes, qAdd);
			struct QueryTxt *qBase = my_mapStringObj_get(name2query,
					qAdd->name_file);
			if (qBase == NULL && i == 0) {
				my_mapStringObj_add(name2query, qAdd->name_file, qAdd);
			} else if (qBase == NULL && i > 0) {
				my_log_info("could not locate query '%s'\n", qAdd->name_file);
			} else {
				mergeListsSummingVotes(qBase, qAdd);
			}
		}
	}
	updateSearchTimes(name2query, name2searchTimes, typeSearchTime);
	my_log_info_time("merge complete, %"PRIi64" queries, %"PRIi64" files\n",
			my_mapStringObj_size(name2query), my_vectorString_size(filenames));
	MyVectorObj *list = my_mapStringObj_valuesVector(name2query);
	for (int64_t i = 0; i < my_vectorObj_size(list); ++i) {
		struct QueryTxt *query = my_vectorObj_get(list, i);
		convertVotesToDistance(query);
	}
	my_mapStringObj_release(name2query, false, false);
	return list;
}
/******************/
char *validateFileFormats(MyVectorString *filenames) {
	const char *firstFile = my_vectorString_get(filenames, 0);
	char *fileFormat = my_io_detectFileConfig(firstFile, "PVCD");
	for (int64_t i = 0; i < my_vectorString_size(filenames); ++i) {
		const char *fn = my_vectorString_get(filenames, i);
		char *fmt = my_io_detectFileConfig(fn, "PVCD");
		if (fmt == NULL || fileFormat == NULL)
			my_log_error("invalid file format\n");
		my_assert_equalString("fileformat", fileFormat, fmt);
		MY_FREE(fmt);
	}
	return fileFormat;
}
static void validateLengthI(char *nameList, MyVectorInt *values,
		MyVectorString *filenames, int64_t defValue) {
	if (my_vectorInt_size(values) == 0) {
		while (my_vectorInt_size(values) < my_vectorString_size(filenames))
			my_vectorInt_add(values, defValue);
	} else if (my_vectorInt_size(values) != my_vectorString_size(filenames))
		my_log_error("list %s must contain %"PRIi64" values\n", nameList,
				my_vectorString_size(filenames));
}
static void validateLengthD(char *nameList, MyVectorDouble *values,
		MyVectorString *filenames, double defValue) {
	if (my_vectorDouble_size(values) == 0) {
		while (my_vectorDouble_size(values) < my_vectorString_size(filenames))
			my_vectorDouble_add(values, defValue);
	} else if (my_vectorDouble_size(values) != my_vectorString_size(filenames))
		my_log_error("list %s must contain %"PRIi64" values\n", nameList,
				my_vectorString_size(filenames));
}
#define METHOD_CONCATENATE 1
#define METHOD_DISTSUM 2
#define METHOD_RANK_AGGREGATION 3
#define DEFAULT_DIST_NOT_FOUND "1"

int pvcd_mergeNN(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	const char *typeSearchTime = "SUM";
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s\n", getBinaryName(cmd_params));
		my_log_info(
				"  -filesIn (fileSS | fileSsVector)...       Mandatory. Two or more files.\n");
		my_log_info(
				"  -method (CONCATENATE | DISTSUM | RANK_AGGREGATION )  Mandatory.\n");
		my_log_info(
				"  -typeSearchTime (SUM | MAX | AVG )                     Optional. Default=%s.\n",
				typeSearchTime);
		my_log_info(
				"  -fileOut filenameNew                           Mandatory.\n");
		my_log_info(
				"  -maxNNWrite [num | SAME_FIRST]  Optional. Max NNs to write. SAME_FIRST=number of NNs loaded from the first file. default=unlimited\n");
		my_log_info(
				"  -maxDistWrite dist             Optioanl. Trims the NN lists to distance <= dist.\n");
		my_log_info(
				"  -maxNNLoad n1 n2 n3...         Optional. Max NNs to read. Default to no limit.\n");
		my_log_info(
				"  -maxDistLoad d1 d2 d3...       Optional. Max NNs to read. Default to no limit.\n");
		my_log_info(
				"  -distsNorm n1 n2 n3...      Optional. Applied to distances. As many as files.\n");
		my_log_info(
				"  -distsWeight w1 w2 w3...    Optional. Applied to distances. As many as files, 0=discard a distance.\n");
		my_log_info(
				"  -distNotFound val          Optional. Assigned to any miss. default=%s\n",
				DEFAULT_DIST_NOT_FOUND);
		return pvcd_system_exit_error();
	}
	const char *outFilename = NULL;
	int64_t maxNNWrite = INT64_MAX;
	MyVectorString *filenames = my_vectorString_new();
	MyVectorInt *maxNNLoad = my_vectorInt_new();
	MyVectorDouble *maxDistLoad = my_vectorDouble_new();
	MyVectorDouble *distsNorm = my_vectorDouble_new();
	MyVectorDouble *distsWeight = my_vectorDouble_new();
	double distNotFound = my_parse_fraction(DEFAULT_DIST_NOT_FOUND);
	double maxDistWrite = DBL_MAX;
	int64_t method = 0, currentList = 0;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-fileOut")) {
			outFilename = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-method")) {
			if (isNextParam(cmd_params, "CONCATENATE"))
				method = METHOD_CONCATENATE;
			else if (isNextParam(cmd_params, "DISTSUM"))
				method = METHOD_DISTSUM;
			else if (isNextParam(cmd_params, "RANK_AGGREGATION"))
				method = METHOD_RANK_AGGREGATION;
			else
				my_log_error("unknown method %s\n", nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-typeSearchTime")) {
			typeSearchTime = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-maxNNWrite")) {
			if (isNextParam(cmd_params, "SAME_FIRST"))
				maxNNWrite = NNWRITE_SAME_FIRST;
			else
				maxNNWrite = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-maxDistWrite")) {
			maxDistWrite = nextParamFraction(cmd_params);
		} else if (isNextParam(cmd_params, "-distNotFound")) {
			distNotFound = nextParamFraction(cmd_params);
		} else if (isNextParam(cmd_params, "-filesIn")) {
			currentList = 1;
		} else if (isNextParam(cmd_params, "-maxNNLoad")) {
			currentList = 2;
		} else if (isNextParam(cmd_params, "-maxDistLoad")) {
			currentList = 3;
		} else if (isNextParam(cmd_params, "-distsNorm")) {
			currentList = 4;
		} else if (isNextParam(cmd_params, "-distsWeight")) {
			currentList = 5;
		} else {
			const char *param = nextParam(cmd_params);
			switch (currentList) {
			case 1:
				my_assert_fileExists(param);
				my_vectorStringConst_add(filenames, param);
				break;
			case 2:
				my_vectorInt_add(maxNNLoad, my_parse_int(param));
				break;
			case 3:
				my_vectorDouble_add(maxDistLoad, my_parse_fraction(param));
				break;
			case 4:
				my_vectorDouble_add(distsNorm, my_parse_fraction(param));
				break;
			case 5:
				my_vectorDouble_add(distsWeight, my_parse_fraction(param));
				break;
			default:
				my_log_error("unknown option %s\n", param);
			}
		}
	}
	my_assert_greaterInt("num files", my_vectorString_size(filenames), 0);
	my_assert_notNull("out", outFilename);
	if (method == 0)
		my_log_error("must choose a method\n");
	validateLengthI("maxNNLoad", maxNNLoad, filenames, INT64_MAX);
	validateLengthD("maxDistLoad", maxDistLoad, filenames, DBL_MAX);
	validateLengthD("norms", distsNorm, filenames, 1);
	validateLengthD("distsWeight", distsWeight, filenames, 1);
	char *fileFormat = validateFileFormats(filenames);
	MyVectorObj *result = NULL;
	switch (method) {
	case METHOD_CONCATENATE:
		result = NN_mergeByConcatenation(filenames, outFilename, maxNNLoad,
				maxDistLoad, &maxNNWrite, distsNorm, distsWeight,
				typeSearchTime);
		break;
	case METHOD_DISTSUM:
		result = NN_mergeByDistSum(filenames, outFilename, maxNNLoad,
				maxDistLoad, &maxNNWrite, distsNorm, distsWeight, distNotFound,
				typeSearchTime);
		break;
	case METHOD_RANK_AGGREGATION:
		result = NN_mergeByRankAggregation(filenames, outFilename, maxNNLoad,
				maxDistLoad, &maxNNWrite, typeSearchTime);
		break;
	}
	writeOutfile(outFilename, cmd_params, fileFormat, result, maxNNWrite,
			maxDistWrite);
	return pvcd_system_exit_ok(cmd_params);
}
