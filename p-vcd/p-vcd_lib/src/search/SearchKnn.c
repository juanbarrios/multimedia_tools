/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

struct SearchKnnOptions {
	CmdParams *cmd_params;
	const char *search_name;
	struct SearchProfile *profile;
	bool write_results, write_results_by_query, load_descriptors_by_query;
	int64_t max_threads;
	//process
	MknnResolver *resolver;
	int64_t size_dataset, distances_computed, distances_expected;
};
static FILE *create_file_results(struct SearchKnnOptions *options,
		bool searchByLocalVectors, struct SearchFile *current_query_file) {
	FILE *out;
	if (searchByLocalVectors) {
		char *fn =
				my_newString_format("%s/ssVector,%s%s%s.txt",
						options->profile->path_profile,
						(options->write_results_by_query ?
								current_query_file->name : ""),
						(options->write_results_by_query ? "," : ""),
						options->search_name);
		out = my_io_openFileWrite1Config(fn, "PVCD", "SSVector", 1, 1);
		free(fn);
	} else {
		char *fn =
				my_newString_format("%s/ss,%s%s%s.txt",
						options->profile->path_profile,
						(options->write_results_by_query ?
								current_query_file->name : ""),
						(options->write_results_by_query ? "," : ""),
						options->search_name);
		out = my_io_openFileWrite1Config(fn, "PVCD", "SS", 1, 1);
		free(fn);
	}
	char *stCmd = get_command_line(options->cmd_params);
	fprintf(out, "#%s\n", stCmd);
	free(stCmd);
	return out;
}
static char *toString_listNns(struct MknnResultQuery *resQuery,
		struct SearchCollection *colReference) {
	if (resQuery->num_nns == 0)
		return my_newString_string("\t");
	MyStringBuffer *sb = my_stringbuf_new();
	int64_t j;
	for (j = 0; j < resQuery->num_nns; ++j) {
		int64_t pos = resQuery->nn_position[j];
		struct SearchSegment *ssegNn = colReference->allSegments[pos];
		char *st1 = toString_SearchSegmentAndFile(ssegNn);
		char *st2 = my_newString_double(resQuery->nn_distance[j]);
		char *st3 = my_newString_format("\t%s\t%s", st1, st2);
		my_stringbuf_appendString(sb, st3);
		MY_FREE_MULTI(st1, st2, st3);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
static void print_results_header(FILE *out, struct SearchFile *queryFile,
		double search_time, struct SearchCollection *colReference) {
	char *name = toString_SearchFile(queryFile);
	char *numSegments = my_newString_int(queryFile->numSegments);
	char *numSeconds = my_newString_doubleDec(search_time, 3);
	fprintf(out, "query=%s\tsegments=%s\tsearch_time=%s\n", name, numSegments,
			numSeconds);
	MY_FREE_MULTI(name, numSegments, numSeconds);
}
static void print_results_list(FILE *out, struct SearchFile *queryFile,
		MknnResult *result, int64_t offset_result,
		struct SearchCollection *colReference) {
	int64_t i;
	for (i = 0; i < queryFile->numSegments; ++i) {
		struct SearchSegment *query = queryFile->ssegments[i];
		MknnResultQuery *resSegment = mknn_result_getResultQuery(result,
				i + offset_result);
		char *stHead =
				(queryFile->numSegments > 1) ?
						toString_SearchSegmentNoFile(query) :
						my_newString_string("");
		char * stNns = toString_listNns(resSegment, colReference);
		fprintf(out, "%s%s\n", stHead, stNns);
		MY_FREE_MULTI(stHead, stNns);
	}
}
static bool testAllFilesOneSegment(struct SearchCollection *col) {
	int64_t i;
	for (i = 0; i < col->numFiles; ++i) {
		struct SearchFile *file = col->sfiles[i];
		if (file->numSegments > 1)
			return false;
	}
	return true;
}
#if 0
struct UnderDistance {
	MknnDistance *underdist;
};

static double distance_segments_simple(void *object1, void *object2,
		double current_max, int64_t num_thread, void *state_dist) {
	struct UnderDistance *state = state_dist;
	struct SearchSegment *s1 = object1;
	struct SearchSegment *s2 = object2;
	return mknn_distanceEval_eval(s1->descriptor, s2->descriptor, current_max,
			num_thread, state->underdist);
}
static MknnDistance *get_distance2(struct SearchProfile *profile, MknnDataset *dataset) {
	MknnDistance *under_distance = mknn_distance_newPredefined(profile->dist_name,
			dataset);
	struct UnderDistance *state = MY_MALLOC(1, struct UnderDistance);
	state->underdist = under_distance;
	return mknn_distance_newCustom(state, distance_segments_simple);
}
static void *getSegmentDescriptor2(void *data_pointer, int64_t pos) {
	struct SearchSegment **segments = data_pointer;
	return segments[pos];
}
#endif

static void search_allQueries(struct SearchKnnOptions *options) {
	struct SearchCollection *colQuery = options->profile->colQuery;
	MknnDataset *query_dataset = get_dataset_global_descriptors(colQuery,
			colQuery->allSegments, colQuery->totalSegments);
	MknnResult *result = mknn_resolver_search(options->resolver, false,
			query_dataset, true);
	if (options->write_results) {
		my_assert_equalInt("num_query_objects",
				mknn_result_getNumQueries(result), colQuery->numFiles);
		double avgTime = mknn_result_getTotalSearchTime(result)
				/ colQuery->numFiles;
		FILE *out = create_file_results(options, false, NULL);
		int64_t i;
		for (i = 0; i < colQuery->numFiles; ++i) {
			struct SearchFile *sfileQ = colQuery->sfiles[i];
			my_assert_equalInt("numSegments", sfileQ->numSegments, 1);
			print_results_header(out, sfileQ, avgTime,
					options->profile->colReference);
			print_results_list(out, sfileQ, result, i,
					options->profile->colReference);
			fflush(out);
		}
		fclose(out);
	}
	options->distances_computed += mknn_result_getTotalDistanceEvaluations(
			result);
	options->distances_expected += mknn_dataset_getNumObjects(query_dataset)
			* options->size_dataset;
	mknn_result_release(result);
}
static void search_queriesByFile(struct SearchKnnOptions *options) {
	struct SearchCollection *colQuery = options->profile->colQuery;
	MyProgress *lt = my_progress_new("ss", colQuery->totalSegments,
			options->max_threads);
	int64_t i;
	FILE *out = NULL;
	for (i = 0; i < colQuery->numFiles; ++i) {
		struct SearchFile *sfileQ = colQuery->sfiles[i];
		if (options->load_descriptors_by_query)
			loadDescriptorsInSearchFile(sfileQ);
		MknnDataset *query_dataset = get_dataset_global_descriptors(colQuery,
				sfileQ->ssegments, sfileQ->numSegments);
		MknnResult *result = mknn_resolver_search(options->resolver, false,
				query_dataset, true);
		if (options->write_results) {
			if (out == NULL)
				out = create_file_results(options, false, sfileQ);
			print_results_header(out, sfileQ,
					mknn_result_getTotalSearchTime(result),
					options->profile->colReference);
			print_results_list(out, sfileQ, result, 0,
					options->profile->colReference);
			fflush(out);
			if (options->write_results_by_query) {
				fclose(out);
				out = NULL;
			}
		}
		options->distances_computed += mknn_result_getTotalDistanceEvaluations(
				result);
		options->distances_expected += mknn_dataset_getNumObjects(query_dataset)
				* options->size_dataset;
		mknn_result_release(result);
		my_progress_addN(lt, sfileQ->numSegments);
		if (options->load_descriptors_by_query)
			unloadDescriptorsInSearchFile(sfileQ);
	}
	if (out != NULL)
		fclose(out);
	my_progress_release(lt);
}

static char *toString_localVector(struct SearchSegment *sseg, int64_t idVector) {
	struct MyLocalKeypoint keypoint = my_localDescriptors_getKeypoint(
			sseg->descriptor, idVector);
	char *st1 = my_newString_doubleDec(keypoint.x, 1);
	char *st2 = my_newString_doubleDec(keypoint.y, 1);
	char *st3 = my_newString_doubleDec(keypoint.radius, 3);
	char *st4 = my_newString_doubleDec(keypoint.angle, 1);
	char *st = my_newString_format("%s,%s,%s,%s", st1, st2, st3, st4);
	MY_FREE_MULTI(st1, st2, st3, st4);
	return st;
}
static char *toString_listVectorsNNs(struct MknnResultQuery *resQuery,
		struct SearchCollection *colReference, MknnDataset *reference_dataset) {
	if (resQuery->num_nns == 0)
		return my_newString_string("\t");
	MyStringBuffer *sb = my_stringbuf_new();
	int64_t j;
	for (j = 0; j < resQuery->num_nns; ++j) {
		int64_t pos = resQuery->nn_position[j];
		int64_t numSegNN = -1, numVecNN = -1;
		mknn_dataset_concatenate_getDatasetObject(reference_dataset, pos,
				&numSegNN, &numVecNN);
		struct SearchSegment *ssegNN = colReference->allSegments[numSegNN];
		char *st1 = toString_SearchSegmentAndFile(ssegNN);
		char *st2 = toString_localVector(ssegNN, numVecNN);
		char *st3 = my_newString_double(resQuery->nn_distance[j]);
		char *st4 = my_newString_format("\t%s|%s\t%s", st1, st2, st3);
		my_stringbuf_appendString(sb, st4);
		MY_FREE_MULTI(st1, st2, st3, st4);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
static void print_results_vectorsSegment(FILE *out, struct SearchSegment *ssegQ,
		int64_t numVectorsQ, MknnResult *result,
		struct SearchCollection *colReference) {
	char *name = toString_SearchSegmentAndFile(ssegQ);
	char *numSegments = my_newString_int(numVectorsQ);
	char *numSeconds = my_newString_doubleDec(
			mknn_result_getTotalSearchTime(result), 3);
	fprintf(out, "query=%s\tvectors=%s\tsearch_time=%s\n", name, numSegments,
			numSeconds);
	MY_FREE_MULTI(name, numSegments, numSeconds);
	MknnDataset *reference_dataset = mknn_index_getSearchDataset(
			mknn_resolver_getIndex(mknn_result_getResolver(result)));
	for (int64_t k = 0; k < numVectorsQ; ++k) {
		MknnResultQuery *resVector = mknn_result_getResultQuery(result, k);
		char *stHead = toString_localVector(ssegQ, k);
		char *stNns = toString_listVectorsNNs(resVector, colReference,
				reference_dataset);
		fprintf(out, "%s%s\n", stHead, stNns);
		MY_FREE_MULTI(stHead, stNns);
	}
	fflush(out);
}
static void search_byLocalVectors(struct SearchKnnOptions *options) {
	struct SearchCollection *colQuery = options->profile->colQuery;
	MyProgress *lt = NULL;
	if (options->load_descriptors_by_query) {
		lt = my_progress_new("ssSegments", colQuery->totalSegments,
				options->max_threads);
	} else {
		int64_t totalVectors = getNumLocalVectors(colQuery->allSegments,
				colQuery->totalSegments);
		lt = my_progress_new("ssVectors", totalVectors, options->max_threads);
	}
	FILE *out = NULL;
	for (int64_t a = 0; a < colQuery->numFiles; ++a) {
		struct SearchFile *sfileQ = colQuery->sfiles[a];
		if (options->load_descriptors_by_query)
			loadDescriptorsInSearchFile(sfileQ);
		for (int64_t b = 0; b < sfileQ->numSegments; ++b) {
			struct SearchSegment *ssegQ = sfileQ->ssegments[b];
			MknnDataset *query_dataset = get_dataset_local_descriptors(colQuery,
					&ssegQ, 1);
			if (mknn_dataset_getNumObjects(query_dataset) == 0) {
				char *name = toString_SearchSegmentAndFile(ssegQ);
				my_log_info("query %s does not have vectors\n", name);
				free(name);
			}
			int64_t numVectorsQ = mknn_dataset_getNumObjects(query_dataset);
			MknnResult *result = mknn_resolver_search(options->resolver, false,
					query_dataset, true);
			my_assert_equalInt("num_query_objects",
					mknn_result_getNumQueries(result), numVectorsQ);
			if (options->write_results) {
				if (out == NULL)
					out = create_file_results(options, true, sfileQ);
				print_results_vectorsSegment(out, ssegQ, numVectorsQ, result,
						options->profile->colReference);
			}
			options->distances_computed +=
					mknn_result_getTotalDistanceEvaluations(result);
			options->distances_expected += numVectorsQ * options->size_dataset;
			mknn_result_release(result);
			if (options->load_descriptors_by_query) {
				my_progress_add1(lt);
			} else {
				my_progress_addN(lt, numVectorsQ);
			}
		}
		if (options->write_results_by_query && out != NULL) {
			fclose(out);
			out = NULL;
		}
		if (options->load_descriptors_by_query)
			unloadDescriptorsInSearchFile(sfileQ);
	}
	if (out != NULL)
		fclose(out);
	my_progress_release(lt);
}
static MknnIndex *create_load_index(MknnDataset *search_dataset,
		MknnDistance *distance, const char *index_build_options,
		const char *load_index_path, const char *load_index_name,
		const char *save_index_path, const char *save_index_name,
		struct SearchKnnOptions *options) {
	if (load_index_path != NULL || load_index_name != NULL) {
		char *fname = NULL;
		if (load_index_name != NULL)
			fname = my_newString_format("%s/%s", options->profile->path_profile,
					load_index_name);
		else
			fname = my_newString_string(load_index_path);
		if (my_io_existsFile(fname)) {
			MknnIndex *index = mknn_index_restore(fname, search_dataset, true,
					distance, true, NULL, true);
			free(fname);
			return index;
		} else {
			my_log_info("can't load index in file %s\n", fname);
			free(fname);
		}
	}
	if (index_build_options != NULL) {
		my_log_info("creating index %s\n", index_build_options);
		MknnIndex *index = mknn_index_newPredefined(
				mknn_indexParams_newParseString(index_build_options), true,
				search_dataset, true, distance, true);
		if (save_index_path != NULL || save_index_name != NULL) {
			char *fname = NULL;
			if (save_index_name != NULL)
				fname = my_newString_format("%s/%s",
						options->profile->path_profile, save_index_name);
			else
				fname = my_newString_string(save_index_path);
			mknn_index_save(index, fname);
			free(fname);
		}
		return index;
	}
	my_log_error("can't load index\n");
	return NULL;
}
void ss_resolve_search(CmdParams *cmd_params, const char *argOption) {
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info("   -profile profile_name\n");
		my_log_info(
				"   -searchName txt                 Optional. unique name (default=auto)\n");
		my_log_info("   -searchByLocalVectors           Optional.\n");
		my_log_info(
				"   -load_index_path filename       Optional. path to the index file to load\n");
		my_log_info(
				"   -load_index_name filename       Optional. name inside the profile of the index file to load\n");
		my_log_info(
				"   -index INDEX_CODE,PARAMS        Index to create. by default: linearscan. -load_index_path -load_index_name override this value.\n");
		my_log_info(
				"   -save_index_path filename       Optional. Saves the created index to file. Requires -index.\n");
		my_log_info(
				"   -save_index_name filename       Optional. Saves the created index to a file inside profile. Requires -index.\n");
		my_log_info(
				"   -searchOptions PARAMS           Optional. depends on index.\n");
		my_log_info(
				"   -knn num                        Optional. Default=1. -1=maximum\n");
		my_log_info(
				"   -range dist_val                 Optional. Default=DBL_MAX\n");
		//my_log_info("   -rangeAlpha 0.xx                Optional. Default=DBL_MAX\n");
		my_log_info(
				"   -noWriteResults                 Optional. avoid printing results\n");
		my_log_info(
				"   -writeResultsByQuery            Optional. one output file by query file.\n");
		my_log_info(
				"   -loadDescriptorsByQuery         Optional. load descriptors before each search\n");
		//log_info("  [-searchSpace opt]                Optional. %s\n",	getSearchSpaceHelp());
		//log_info(
		//	"  [-reorder (Q|R) (RANDOM|PERMUT_file|NAMES_file)]+  change ordering of segments in Q and R\n");
		//log_info("  [-gt filename]\n");
		return;
	}
	const char *profile_name = NULL, *index_build_options = "LINEARSCAN",
			*search_options =
			NULL, *search_name = NULL;
	const char *load_index_path = NULL, *load_index_name = NULL;
	const char *save_index_path = NULL, *save_index_name = NULL;
	bool write_results = true, searchByLocalVectors = false,
			write_results_by_query =
			false, load_descriptors_by_query = false;
	int64_t search_knn = 1;
	double search_range = DBL_MAX;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-profile")) {
			profile_name = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-index")) {
			index_build_options = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-load_index_path")) {
			load_index_path = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-load_index_name")) {
			load_index_name = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-save_index_path")) {
			save_index_path = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-save_index_name")) {
			save_index_name = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-knn")) {
			search_knn = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-range")) {
			search_range = nextParamDouble(cmd_params);
		} else if (isNextParam(cmd_params, "-searchOptions")) {
			search_options = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-searchName")) {
			search_name = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-searchByLocalVectors")) {
			searchByLocalVectors = true;
		} else if (isNextParam(cmd_params, "-noWriteResults")) {
			write_results = false;
		} else if (isNextParam(cmd_params, "-writeResultsByQuery")) {
			write_results_by_query = true;
		} else if (isNextParam(cmd_params, "-loadDescriptorsByQuery")) {
			load_descriptors_by_query = true;
			/*
			 } else if (isNextParam(cmd_params, "-searchSpace")) {
			 searchSpace = newSearchSpace(nextParam(cmd_params));
			 } else if (isNextParam(cmd_params, "-reorder")) {
			 if (isNextParam(cmd_params, "Q"))
			 reorderQ = nextParam(cmd_params);
			 else if (isNextParam(cmd_params, "R"))
			 reorderR = nextParam(cmd_params);
			 else
			 log_error("invalid reorder %s (Q or R)\n",
			 nextParam(cmd_params));
			 } else if (isNextParam(cmd_params, "-gt")) {
			 fileGroundTruth = nextParam(cmd_params);
			 */
		} else {
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
		}
	}
	my_assert_notNull("profile", profile_name);
	if (search_name == NULL) {
		MyStringBuffer *sb = my_stringbuf_new();
		if (searchByLocalVectors)
			my_stringbuf_appendString(sb, "vectors");
		else
			my_stringbuf_appendString(sb, "segments");
		my_stringbuf_appendString(sb, "-knn_");
		my_stringbuf_appendInt(sb, search_knn);
		if (search_range != DBL_MAX) {
			my_stringbuf_appendString(sb, "-range_");
			my_stringbuf_appendDouble(sb, search_range);
		}
		search_name = my_stringbuf_releaseReturnBuffer(sb);
	}
	set_logfile(profile_name, cmd_params, "");
	struct SearchProfile *profile = loadProfile(profile_name);
	loadDescriptorsInCollection(profile->colReference);
	my_log_info_time("starting search %s\n", search_name);
	MyTimer *timerGlobal = my_timer_new();
	struct SearchKnnOptions options = { 0 };
	options.cmd_params = cmd_params;
	options.search_name = search_name;
	options.profile = profile;
	options.write_results = write_results;
	options.write_results_by_query = write_results_by_query;
	options.load_descriptors_by_query = load_descriptors_by_query;
	options.max_threads = NUM_CORES;
	MknnDataset *search_dataset = NULL;
	if (searchByLocalVectors) {
		search_dataset = get_dataset_local_descriptors(profile->colReference,
				profile->colReference->allSegments,
				profile->colReference->totalSegments);
	} else {
		search_dataset = get_dataset_global_descriptors(profile->colReference,
				profile->colReference->allSegments,
				profile->colReference->totalSegments);
	}
	options.size_dataset = mknn_dataset_getNumObjects(search_dataset);
	MknnDistance *distance = profile_get_mdistance(profile);
	MknnIndex *index = create_load_index(search_dataset, distance,
			index_build_options, load_index_path, load_index_name,
			save_index_path, save_index_name, &options);
	MknnResolverParams *params_resolver = mknn_resolverParams_newParseString(
			search_knn, search_range, options.max_threads, search_options);
	options.resolver = mknn_index_newResolver(index, params_resolver, true);
	if (!options.load_descriptors_by_query
			&& profile->colQuery != profile->colReference)
		loadDescriptorsInCollection(profile->colQuery);
	if (searchByLocalVectors) {
		search_byLocalVectors(&options);
	} else if (testAllFilesOneSegment(profile->colQuery)) {
		search_allQueries(&options);
	} else {
		search_queriesByFile(&options);
	}
	if (!options.load_descriptors_by_query
			&& profile->colQuery != profile->colReference)
		unloadDescriptorsInCollection(profile->colQuery);
	my_log_info_time("SECONDS\t%1.1lf\n", my_timer_getSeconds(timerGlobal));
	if (options.distances_computed > 0) {
		my_log_info_time("DISTANCES\t%"PRIi64"\t%4.1lf%%\n",
				options.distances_computed,
				options.distances_computed * 100.0
						/ options.distances_expected);
	}
	my_timer_release(timerGlobal);
	mknn_resolver_release(options.resolver);
	mknn_index_release(index);
	unloadDescriptorsInCollection(profile->colReference);
	releaseProfile(profile);
}

#if 0
struct GroundTruthFrames *gtf = NULL;
if (fileGroundTruth != NULL)
gtf = newGroundTruthFrames(fileGroundTruth, profile->colQuery,
		profile->colReference);
if (reorderQ != NULL)
reorderSearchFiles(profile->colQuery, reorderQ);
if (reorderR != NULL)
reorderSearchObjects(profile->colReference, reorderR);
if (rangeAlpha != DBL_MAX) {
	HistogramaDistancias *hist = newHistogramaDistanciasFile(
			profile->path_profile, "histogram-Q_R.hist", 1);
	range = getDistanciaThreshold(hist, rangeAlpha);
	mknn_histogram_release(hist);
	char *s1 = my_newString_double(rangeAlpha);
	char *s2 = my_newString_double(range);
	my_log_info("range alpha=%s => distance range %s\n", s1, s2);
	MY_FREE_MULTI(s1, s2);
}
#endif

#if 0
static void bus_resolverQuerySS_thread(int64_t numQueryKf,
		struct Results_SS *results) {
	struct Params_SS_Search *pSearch = results->pSearch;
	struct SearchSegment *query = results->queries[numQueryKf];
	double range = pSearch->range;
	if (pSearch->gtf != NULL) {
		Array_obj *corrects = pSearch->gtf->frames[query->seq_obj_id];
		if (corrects == NULL || getLengthArray_obj(corrects) == 0)
		return;
		range = pSearch->rangesSearchGt[query->seq_obj_id];
	}
	if (pSearch->gtf != NULL) {
		char *fn = my_newString_concat("ssGt,", pSearch->searchID, ".txt");
		pSearch->out = my_io_openFileWrite2Config(pSearch->profile->path_profile, fn,
				"SS_GT", 1, 0);
		MY_FREE(fn);
	}
	if (pSearch->gtf != NULL) {
		pSearch->rangesSearchGt = compute_ranges_gt(gtf, profile, &numQueries);
		pSearch->ranksGt = newHistogramaDistanciasParm(numQueries + 1, 10000);
		agregarDistanciaHistograma(pSearch->ranksGt,
				profile->colReference->totalSegments);
	}
	if (pSearch->gtf != NULL) {
		char *fn = my_newString_concat("ssGt,", pSearch->searchID, ".hist");
		FILE *out = my_io_openFileWrite2Config(pSearch->profile->path_profile, fn,
				"HIST", 1, 0);
		MY_FREE(fn);
		char *stCmd = get_command_line(cmd_params);
		fprintf(out, "#%s\n", stCmd);
		MY_FREE(stCmd);
		mknn_histogram_saveBins(pSearch->ranksGt, out);
		fclose(out);
	}

}
static char *priv_printCorrectNns(int64_t numNNs, struct SearchSegment **nn_reference,
		double *nn_dist, struct SearchSegment *query, struct Params_SS_Search *pSearch) {
	Array_obj *corrects = pSearch->gtf->frames[query->seq_obj_id];
	if (corrects == NULL || getLengthArray_obj(corrects) == 0)
	return my_newString_string("\t");
	int64_t j;
	for (j = 0; j < numNNs; ++j) {
		struct SearchSegment *nn = nn_reference[j];
		int64_t k;
		for (k = 0; k < getLengthArray_obj(corrects); ++k) {
			struct SearchSegment *kf = getObj(corrects, k);
			if (kf == nn) {
				int64_t rank = j + 1;
				agregarDistanciaHistograma(pSearch->ranksGt, rank);
				double dist = nn_dist[j];
				char *st1 = toString_SearchSegmentAndFile(nn);
				char *st2 = my_newString_double(dist);
				char *stOut = my_newString_format("\t%s\t%"PRIi64"\t%s", st1, rank, st2);
				MY_FREE_MULTI(st1, st2);
				return stOut;
			}
		}
	}
	pSearch->contNotFound++;
	int64_t k;
	MyStringBuffer *sb = my_stringbuf_new();
	for (k = 0; k < getLengthArray_obj(corrects); ++k) {
		struct SearchSegment *kf = getObj(corrects, k);
		char *st1 = toString_SearchSegmentAndFile(kf);
		my_stringbuf_appendString(sb, (k == 0) ? "\t" : "|");
		my_stringbuf_appendString(sb, st1);
		MY_FREE(st1);
	}
	my_stringbuf_appendString(sb, "\t-1\t-1");
	char *stOut = my_stringbuf_getCurrentBuffer(sb);
	my_stringbuf_releaseNoBuffer(sb);
	return stOut;
}
static double *compute_ranges_gt(struct GroundTruthFrames *gtf,
		struct SearchProfile *profile, int64_t *out_numQueries) {
	struct SearchCollection *colQuery = profile->colQuery;
	Distance *fd = profile->fd;
	double *ranges = MY_MALLOC(colQuery->totalSegments, double);
	int64_t i, contQueries = 0;
	for (i = 0; i < colQuery->totalSegments; ++i) {
		Array_obj *corrects = gtf->frames[i];
		if (corrects == NULL || getLengthArray_obj(corrects) == 0)
		continue;
		contQueries++;
		struct SearchSegment *q = colQuery->allSegments[i];
		double minDist = DBL_MAX;
		int64_t j;
		for (j = 0; j < getLengthArray_obj(corrects); ++j) {
			struct SearchSegment *kf = getObj(corrects, j);
			double dist = DIST(fd, q, kf);
			if (dist < minDist)
			minDist = dist;
		}
		ranges[i] = (minDist < DBL_MAX) ? minDist * 1.001 : minDist;
	}
	*out_numQueries = contQueries;
	return ranges;
}
#endif

