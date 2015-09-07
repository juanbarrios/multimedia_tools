/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#define NUM_PREVS 5
#define AVERAGES_DATATYPE MY_DATATYPE_FLOAT64
#define AVERAGES_C_TYPE double

struct Kmeans_Stats {
	struct MknnKmeansStatsClustering *stats_clustering;
	struct MknnKmeansStatsCluster *stats_clusters;
};
static void computeClusteringStats(MknnKmeansAlgorithm *kmeans);
static void releaseClusteringStats(struct Kmeans_Stats *stats);

struct Kmeans_CentroidData {
	int64_t num_elements;
	int64_t num_elements_prevs[NUM_PREVS];
	int64_t cont_lastMovedIn;
	int64_t cont_lastMovedOut;
	double dist_lastAdjust;
};
struct EndParameters {
	int64_t maxIteration;
	double maxSecondsProcess;
	double fractionMinMovedVectors;
	double fractionMinMovedCentroids;
	int64_t numMinMovedVectors;
	int64_t numMinMovedCentroids;
};
struct SubsetRun {
	double fractionSubsetSize;
	struct EndParameters end;
};
struct MknnKmeansAlgorithm {
	//input
	MknnDataset *dataset;
	MknnDistance *distance;
	bool release_distance;
	int64_t num_vectors, num_dimensions;
	//parameters
	int64_t num_centroids;
	struct EndParameters end;
	int64_t max_threads;
	char *filenameAutoSaveState;
	MknnDatatype default_centroids_datatype;
	double secondsToPrintInfo;
	double secondsAutoSaveState;
	char *paramsIndex, *paramsResolver;
	MyVectorObj *subsetsRuns;
	//
	int64_t savedPriorNumIterations;
	double savedPriorSeconds;
	//
	int64_t contIterationsSameAssignations;
	int64_t *assignations;
	bool release_assignations;
	MknnDataset *centroids;
	bool release_centroids;
	//
	MknnDistance *distance_euclideanSquared;
	MknnDistanceEval **distEvals_euclideanSquared_vector2centroid;
	MknnDistanceEval **distEvals_vector2centroid;
	MknnDistanceEval **distEvals_centroid2centroid;
	my_function_copy_vector func_copy_centroid2centroid;
	my_function_copy_vector func_copy_vector2average;
	my_function_copy_vector func_copy_average2centroid;
	//
	struct Kmeans_CentroidData *centroids_data;
	double dist_maxAdjust;
	mknn_kmeans_function_callback iteration_callback_function;
	void *iteration_callback_state_pointer;
	struct Kmeans_Stats *stats;
//
//double *dist_1NN_estimated, *dist_2NN_estimated;
//int64_t contAhorros, contEvaluados;
//double numSeconds_prev;
//int64_t numIterations_prev;
};

static MknnKmeansAlgorithm *getDefaultKmeans() {
	struct MknnKmeansAlgorithm *kmeans = MY_MALLOC(1,
			struct MknnKmeansAlgorithm);
	kmeans->secondsToPrintInfo = 60;
	kmeans->max_threads = my_parallel_getNumberOfCores();
	kmeans->default_centroids_datatype = MKNN_DATATYPE_FLOATING_POINT_32bits;
	kmeans->paramsIndex = my_newString_string("LINEARSCAN");
	return kmeans;
}

MknnKmeansAlgorithm *mknn_kmeans_new() {
	MknnKmeansAlgorithm *kmeans = getDefaultKmeans();
	/*
	 dist_1NN_estimated = MY_MALLOC_NOINIT(numVectors,
	 double);
	 dist_2NN_estimated = MY_MALLOC_NOINIT(numVectors,
	 double);
	 for (int64_t k = 0; k < numVectors; ++k) {
	 assignation[k] = -1;
	 dist_1NN_estimated[k] = dist_2NN_estimated[k] =
	 DBL_MAX;
	 }
	 */
	return kmeans;
}
void mknn_kmeans_setDataset(MknnKmeansAlgorithm *kmeans, MknnDataset *dataset) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	if (!mknn_domain_isGeneralDomainVector(domain))
		my_log_error("clustering is supported only for vectors\n");
	kmeans->dataset = dataset;
	kmeans->num_vectors = mknn_dataset_getNumObjects(dataset);
	kmeans->num_dimensions = mknn_domain_vector_getNumDimensions(domain);
}
void mknn_kmeans_setDistance(MknnKmeansAlgorithm *kmeans,
		MknnDistance *distance) {
	kmeans->distance = distance;
}
void mknn_kmeans_setNumCentroids(MknnKmeansAlgorithm *kmeans,
		int64_t num_centroids) {
	kmeans->num_centroids = num_centroids;
}

static void initDatasetCentroids(MknnKmeansAlgorithm *kmeans) {
	my_assert_notNull("centroids", kmeans->centroids);
	MknnDomain *domain = mknn_dataset_getDomain(kmeans->centroids);
	if (!mknn_domain_isGeneralDomainVector(domain))
		my_log_error("datasets must contain vectors\n");
	my_assert_equalInt("dimension", mknn_domain_vector_getNumDimensions(domain),
			kmeans->num_dimensions);
	if (kmeans->num_centroids == 0)
		kmeans->num_centroids = mknn_dataset_getNumObjects(kmeans->centroids);
	else
		my_assert_equalInt("num_centroids",
				mknn_dataset_getNumObjects(kmeans->centroids),
				kmeans->num_centroids);
	kmeans->centroids_data = MY_MALLOC(kmeans->num_centroids,
			struct Kmeans_CentroidData);
	if (kmeans->assignations == NULL) {
		kmeans->assignations = MY_MALLOC_NOINIT(kmeans->num_vectors, int64_t);
		for (int64_t k = 0; k < kmeans->num_vectors; ++k)
			kmeans->assignations[k] = -1;
		kmeans->release_assignations = true;
	}
}
static void initDistances(MknnKmeansAlgorithm *kmeans) {
	my_assert_notNull("dataset", kmeans->dataset);
	my_assert_notNull("distance", kmeans->distance);
	my_assert_notNull("centroids", kmeans->centroids);
	my_assert_greaterInt("num_centroids", kmeans->num_centroids, 0);
	my_assert_greaterInt("num_dimensions", kmeans->num_dimensions, 0);
	kmeans->distEvals_centroid2centroid = mknn_distance_createDistEvalArray(
			kmeans->distance, kmeans->max_threads,
			mknn_dataset_getDomain(kmeans->centroids),
			mknn_dataset_getDomain(kmeans->centroids));
	kmeans->distEvals_vector2centroid = mknn_distance_createDistEvalArray(
			kmeans->distance, kmeans->max_threads,
			mknn_dataset_getDomain(kmeans->dataset),
			mknn_dataset_getDomain(kmeans->centroids));
	kmeans->distance_euclideanSquared = mknn_distance_newPredefined(
			mknn_predefDistance_L2squared(), true);
	kmeans->distEvals_euclideanSquared_vector2centroid =
			mknn_distance_createDistEvalArray(kmeans->distance_euclideanSquared,
					kmeans->max_threads,
					mknn_dataset_getDomain(kmeans->dataset),
					mknn_dataset_getDomain(kmeans->centroids));
	MyDatatype vectors_datatype = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(
					mknn_dataset_getDomain(kmeans->dataset)));
	MyDatatype centroids_datatype = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(
					mknn_dataset_getDomain(kmeans->centroids)));
	kmeans->func_copy_centroid2centroid = my_datatype_getFunctionCopyVector(
			centroids_datatype, centroids_datatype);
	kmeans->func_copy_vector2average = my_datatype_getFunctionCopyVector(
			vectors_datatype, AVERAGES_DATATYPE);
	kmeans->func_copy_average2centroid = my_datatype_getFunctionCopyVector(
	AVERAGES_DATATYPE, centroids_datatype);
}
static void assignVectors(MknnKmeansAlgorithm *kmeans) {
	for (int64_t i = 0; i < kmeans->num_centroids; ++i) {
		kmeans->centroids_data[i].cont_lastMovedIn =
				kmeans->centroids_data[i].cont_lastMovedOut = 0;
	}
	MknnIndexParams *indexParams = mknn_indexParams_newParseString(
			kmeans->paramsIndex);
	MknnResolverParams *resolverParams = mknn_resolverParams_newParseString(1,
			0, kmeans->max_threads, kmeans->paramsResolver);
	MknnIndex *index = mknn_index_newPredefined(indexParams,
	true, kmeans->centroids, false, kmeans->distance, false);
	MknnResolver *resolver = mknn_index_newResolver(index, resolverParams,
	true);
	MknnResult *result = mknn_resolver_search(resolver, true, kmeans->dataset,
	false);
//int64_t contAhorros = 0, contEvaluados = 0;
	for (int64_t i = 0; i < kmeans->num_vectors; ++i) {
		/*
		 if (asignacion_act >= 0) {
		 double new_1nn = dist_1NN_estimated[i]
		 + kmeans->centroids_data[asignacion_act]->dist_lastMovement;
		 double new_2nn = dist_2NN_estimated[i]
		 - kmeans->dist_maxAjuste;
		 if (new_1nn <= new_2nn) {
		 dist_1NN_estimated[i] = new_1nn;
		 dist_2NN_estimated[i] = new_2nn;
		 contAhorros++;
		 continue;
		 }
		 }*/
		MknnResultQuery *rq = mknn_result_getResultQuery(result, i);
		int64_t assignation_new = (rq->num_nns > 0) ? rq->nn_position[0] : -1;
		int64_t assignation_old = kmeans->assignations[i];
		/*
		 double dist_1nn = DBL_MAX, dist_2nn = DBL_MAX;
		 dist_1NN_estimated[i] = dist_1nn;
		 dist_2NN_estimated[i] = dist_2nn;
		 */
		if (assignation_old != assignation_new) {
			if (assignation_old >= 0)
				kmeans->centroids_data[assignation_old].cont_lastMovedOut++;
			kmeans->centroids_data[assignation_new].cont_lastMovedIn++;
			kmeans->assignations[i] = assignation_new;
		}
	}
	mknn_result_release(result);
	mknn_index_release(index);
}
static void adjustCentroids_thread(int64_t currentProcess, void* state,
		int64_t current_thread) {
	MknnKmeansAlgorithm *kmeans = state;
	int64_t idCentroid = currentProcess;
	struct Kmeans_CentroidData *c = kmeans->centroids_data + idCentroid;
	//update the last sizes
	for (int64_t i = NUM_PREVS - 1; i > 0; --i)
		c->num_elements_prevs[i] = c->num_elements_prevs[i - 1];
	c->num_elements_prevs[0] = c->num_elements;
	if (c->cont_lastMovedIn == 0 && c->cont_lastMovedOut == 0) {
		c->dist_lastAdjust = 0;
		return;
	}
	c->num_elements = 0;
	c->dist_lastAdjust = 0;
	int64_t dimensions = kmeans->num_dimensions;
	AVERAGES_C_TYPE *vector_copied = MY_MALLOC(dimensions, AVERAGES_C_TYPE);
	AVERAGES_C_TYPE *vector_average = MY_MALLOC(dimensions, AVERAGES_C_TYPE);
	for (int64_t j = 0; j < kmeans->num_vectors; ++j) {
		if (kmeans->assignations[j] != idCentroid)
			continue;
		void *vector_orig = mknn_dataset_getObject(kmeans->dataset, j);
		kmeans->func_copy_vector2average(vector_orig, vector_copied,
				dimensions);
		c->num_elements++;
		for (int64_t i = 0; i < dimensions; ++i)
			vector_average[i] += (vector_copied[i] - vector_average[i])
					/ c->num_elements;
	}
	if (c->num_elements > 0) {
		void *new_centroid = mknn_domain_vector_createNewEmptyVectors(
				mknn_dataset_getDomain(kmeans->centroids), 1);
		kmeans->func_copy_average2centroid(vector_average, new_centroid,
				dimensions);
		void *old_centroid = mknn_dataset_getObject(kmeans->centroids,
				idCentroid);
		c->dist_lastAdjust = mknn_distanceEval_eval(
				kmeans->distEvals_centroid2centroid[current_thread],
				old_centroid, new_centroid);
		kmeans->func_copy_centroid2centroid(new_centroid, old_centroid,
				dimensions);
		free(new_centroid);
	}
	free(vector_copied);
	free(vector_average);
}
static void adjustCentroids(MknnKmeansAlgorithm *kmeans) {
	my_parallel_incremental(kmeans->num_centroids, kmeans,
			adjustCentroids_thread,
			NULL, kmeans->max_threads);
	double max_d = 0;
	for (int64_t j = 0; j < kmeans->num_centroids; ++j) {
		double val = kmeans->centroids_data[j].dist_lastAdjust;
		if (val > max_d)
			max_d = val;
	}
	kmeans->dist_maxAdjust = max_d;
}
static void saveStatsClustering(MknnKmeansAlgorithm *kmeans,
		const char *filenameState) {
	if (kmeans->stats == NULL)
		computeClusteringStats(kmeans);
	struct Kmeans_Stats *stats = kmeans->stats;
	FILE *out = my_io_openFileWrite1(filenameState);
	fprintf(out, "clustering statistics\n");
	char *st_a = my_newString_double(
			stats->stats_clustering->sum_squared_error);
	char *st_b = my_newString_double(
			stats->stats_clustering->average_squared_error);
	fprintf(out, "sum_SSE\t%s\naverage_SSE\t%s\n", st_a, st_b);
	MY_FREE_MULTI(st_a, st_b);
	fprintf(out, "\ndistances statistics for each centroid\n");
	fprintf(out,
			"id\telements\tsum_SSE\taverage_SSE\tdist_minimum\tdist_maximum\tdist_average\tdist_variance\tdist_std_dev\tdist_skewness\tdist_kurtosis\n");
	for (int64_t j = 0; j < kmeans->num_centroids; ++j) {
		struct MknnKmeansStatsCluster *c = stats->stats_clusters + j;
		char *st1 = my_newString_double(c->sum_squared_error);
		char *st2 = my_newString_double(c->average_squared_error);
		char *st3 = my_newString_double(c->distances_minimum);
		char *st4 = my_newString_double(c->distances_maximum);
		char *st5 = my_newString_double(c->distances_average);
		char *st6 = my_newString_double(c->distances_variance);
		char *st7 = my_newString_double(c->distances_std_dev);
		char *st8 = my_newString_double(c->distances_skewness);
		char *st9 = my_newString_double(c->distances_kurtosis);
		fprintf(out,
				"%"PRIi64"\t%"PRIi64"\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", j,
				c->num_assignations, st1, st2, st3, st4, st5, st6, st7, st8,
				st9);
		MY_FREE_MULTI(st1, st2, st3, st4, st5, st6, st7, st8, st9);
	}
	fclose(out);
}
static void saveProcessState(MknnKmeansAlgorithm *kmeans,
		const char *filename_write, int64_t numIteration, double secondsProcess,
		int64_t contMovedVectors, int64_t contMovedCenters) {
	char *fnamekmeans =
			my_string_endsWith_ignorecase(filename_write, ".kmeans") ?
					my_newString_string(filename_write) :
					my_newString_format("%s.kmeans", filename_write);
	FILE *out = my_io_openFileWrite1Config(fnamekmeans, "MetricKnn",
			"KmeansState", 1, 1);
	fprintf(out, "kmeans.centroids.num_centroids=%"PRIi64"\n",
			kmeans->num_centroids);
	fprintf(out, "kmeans.distance=%s\n",
			mknn_distanceParams_toString(
					mknn_distance_getParameters(kmeans->distance)));
	fprintf(out, "kmeans.dataset.num_vectors=%"PRIi64"\n", kmeans->num_vectors);
	fprintf(out, "kmeans.dataset.num_dimensions=%"PRIi64"\n",
			kmeans->num_dimensions);
	MknnDomain *centroids_domain = mknn_dataset_getDomain(kmeans->centroids);
	char *st_domain = mknn_domain_toString(centroids_domain);
	fprintf(out, "kmeans.centroids.domain=%s\n", st_domain);
	free(st_domain);
	if (secondsProcess + kmeans->savedPriorSeconds > 0)
		fprintf(out, "kmeans.total_runs.timeSeconds=%1.1lf\n",
				secondsProcess + kmeans->savedPriorSeconds);
	if (numIteration + kmeans->savedPriorNumIterations > 0)
		fprintf(out, "kmeans.total_runs.numIterations=%"PRIi64"\n",
				numIteration + kmeans->savedPriorNumIterations);
	if (secondsProcess > 0)
		fprintf(out, "kmeans.last_run.timeSeconds=%1.1lf\n", secondsProcess);
	if (numIteration > 0)
		fprintf(out, "kmeans.last_run.numIterations=%"PRIi64"\n", numIteration);
	if (contMovedCenters > 0)
		fprintf(out, "kmeans.last_run.contMovedCenters=%"PRIi64"\n",
				contMovedCenters);
	if (contMovedVectors > 0)
		fprintf(out, "kmeans.last_run.contMovedVectors=%"PRIi64"\n",
				contMovedVectors);
	if (kmeans->dist_maxAdjust > 0) {
		char *st = my_newString_double(kmeans->dist_maxAdjust);
		fprintf(out, "kmeans.last_run.dist_maxAdjust=%s\n", st);
		free(st);
	}
	fprintf(out, "--\n\n");
	int64_t length_in_bytes = mknn_domain_vector_getVectorLengthInBytes(
			centroids_domain);
	for (int64_t i = 0; i < mknn_dataset_getNumObjects(kmeans->centroids);
			++i) {
		void *object = mknn_dataset_getObject(kmeans->centroids, i);
		int64_t wrote = fwrite(object, sizeof(char), length_in_bytes, out);
		my_assert_equalInt("fwrite", wrote, length_in_bytes);
	}
	for (int64_t i = 0; i < kmeans->num_centroids; ++i)
		fwrite(&kmeans->centroids_data[i].num_elements, sizeof(int64_t), 1,
				out);
	fwrite(kmeans->assignations, sizeof(int64_t), kmeans->num_vectors, out);
	fclose(out);
	if (true) {
		char *filenameCentroids2 = my_newString_format("%s.centroids.txt",
				fnamekmeans);
		FILE *out = my_io_openFileWrite1(filenameCentroids2);
		mknn_dataset_printObjectsText(kmeans->centroids, out);
		fclose(out);
		free(filenameCentroids2);
		char *filenameStats = my_newString_format("%s.stats.txt", fnamekmeans);
		saveStatsClustering(kmeans, filenameStats);
		free(filenameStats);
	}
	free(fnamekmeans);
}

static void loadStatsClustering(MknnKmeansAlgorithm *kmeans,
		const char *filename_read) {
	FILE *input = my_io_openFileRead1(filename_read, true);
	MyLineReader *reader = my_lreader_config_open_params(input, 1, "MetricKnn",
			"KmeansState", 1, 1);
	int64_t num_vectors = 0, num_dimensions = 0, num_centroids = 0;
	char *distance_name = NULL, *cdomain_name = NULL;
	for (;;) {
		const char *line = my_lreader_readLine(reader);
		if (line == NULL || my_string_equals(line, "--"))
			break;
		int64_t pos = my_string_indexOf(line, "=");
		if (my_string_startsWith_ignorecase(line,
				"kmeans.centroids.num_centroids=")) {
			num_centroids = my_parse_int(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line, "kmeans.distance=")) {
			distance_name = my_newString_string(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"kmeans.dataset.num_vectors=")) {
			num_vectors = my_parse_int(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"kmeans.dataset.num_dimensions=")) {
			num_dimensions = my_parse_int(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"kmeans.centroids.domain=")) {
			cdomain_name = my_newString_string(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"kmeans.total_runs.timeSeconds=")) {
			kmeans->savedPriorSeconds = my_parse_double(line + pos + 1);
		} else if (my_string_startsWith_ignorecase(line,
				"kmeans.total_runs.numIterations=")) {
			kmeans->savedPriorNumIterations = my_parse_int(line + pos + 1);
		}
	}
	my_lreader_close(reader, false);
	if (num_centroids != 0) {
		if (kmeans->num_centroids == 0)
			kmeans->num_centroids = num_centroids;
		else if (kmeans->num_centroids != num_centroids)
			my_log_error(
					"invalid format %s (num_centroids %"PRIi64" != %"PRIi64")\n",
					filename_read, kmeans->num_centroids, num_centroids);
	}
	if (num_dimensions != 0) {
		if (kmeans->num_dimensions == 0)
			kmeans->num_dimensions = num_dimensions;
		else if (kmeans->num_dimensions != num_dimensions)
			my_log_error(
					"invalid format %s (num_dimensions %"PRIi64" != %"PRIi64")\n",
					filename_read, kmeans->num_dimensions, num_dimensions);
	}
	if (distance_name != NULL) {
		MknnDistance *distance = mknn_distance_newPredefined(
				mknn_distanceParams_newParseString(distance_name), true);
		if (kmeans->distance != NULL)
			my_log_info_time("kmeans: warning, restoring distance to %s\n",
					distance_name);
		kmeans->distance = distance;
		kmeans->release_distance = true;
		free(distance_name);
	}
	if (cdomain_name == NULL)
		my_log_error("invalid file format %s\n", filename_read);
	MknnDomain *centroids_domain = mknn_domain_newParseString(cdomain_name);
	int64_t expected_size = num_centroids
			* mknn_domain_vector_getVectorLengthInBytes(centroids_domain);
	void *data_bytes = MY_MALLOC_NOINIT(expected_size, char);
	my_io_readBytesFile(input, data_bytes, expected_size, false);
	MknnDataset *centroids = mknn_datasetLoader_PointerCompactVectors_alt(
			data_bytes, true, num_centroids, centroids_domain, true);
	if (kmeans->centroids != NULL)
		my_log_info_time(
				"kmeans: warning, restoring centroids to state in %s\n",
				filename_read);
	kmeans->centroids = centroids;
	kmeans->release_centroids = true;
	free(cdomain_name);
	initDatasetCentroids(kmeans);
	if (num_vectors != kmeans->num_vectors) {
		my_log_info(
				"warning: saved num_vectors does not match current dataset size. Assigations will not be loaded.\n");
	} else {
		int64_t expected_size_2 = kmeans->num_centroids * sizeof(int64_t);
		int64_t *data2 = MY_MALLOC_NOINIT(kmeans->num_centroids, int64_t);
		my_io_readBytesFile(input, data2, expected_size_2, false);
		for (int64_t i = 0; i < kmeans->num_centroids; ++i)
			kmeans->centroids_data[i].num_elements = data2[i];
		free(data2);
		int64_t expected_size_3 = kmeans->num_vectors * sizeof(int64_t);
		int64_t *data3 = MY_MALLOC_NOINIT(kmeans->num_vectors, int64_t);
		my_io_readBytesFile(input, data3, expected_size_3, true);
		for (int64_t i = 0; i < kmeans->num_vectors; ++i)
			kmeans->assignations[i] = data3[i];
		free(data3);
	}
	fclose(input);
}

static void printStartLog(MknnKmeansAlgorithm *kmeans) {
	MyStringBuffer *sbLog = my_stringbuf_new();
	if (kmeans->end.maxIteration > 0) {
		my_stringbuf_appendString(sbLog, " maxIteration=");
		my_stringbuf_appendInt(sbLog, kmeans->end.maxIteration);
	}
	if (kmeans->end.maxSecondsProcess > 0) {
		my_stringbuf_appendString(sbLog, " maxSecondsProcess=");
		my_stringbuf_appendInt(sbLog, kmeans->end.maxSecondsProcess);
	}
	if (kmeans->end.numMinMovedVectors > 0) {
		my_stringbuf_appendString(sbLog, " minMovedVectors=");
		my_stringbuf_appendInt(sbLog, kmeans->end.numMinMovedVectors);
	}
	if (kmeans->end.numMinMovedCentroids > 0) {
		my_stringbuf_appendString(sbLog, " minMovedCentroids=");
		my_stringbuf_appendInt(sbLog, kmeans->end.numMinMovedCentroids);
	}
	if (kmeans->max_threads > 1) {
		my_stringbuf_appendString(sbLog, " max_threads=");
		my_stringbuf_appendInt(sbLog, kmeans->max_threads);
	}
	my_log_info_time(
			"kmeans: computing %"PRIi64" centroids for %"PRIi64" vectors %"PRIi64"-d distance=%s%s\n",
			kmeans->num_centroids, kmeans->num_vectors, kmeans->num_dimensions,
			mknn_distanceParams_toString(
					mknn_distance_getParameters(kmeans->distance)),
			my_stringbuf_getCurrentBuffer(sbLog));
	my_stringbuf_release(sbLog);
}
static MknnDataset *createEmptyDataset(int64_t num_vectors,
		int64_t num_dimensions, MknnDatatype datatype) {
	MknnDomain *domain = mknn_domain_newVector(num_dimensions, datatype);
	void *all_vectors = mknn_domain_vector_createNewEmptyVectors(domain,
			num_vectors);
	return mknn_datasetLoader_PointerCompactVectors_alt(all_vectors, true,
			num_vectors, domain,
			true);
}
static void selectCentroids(MknnKmeansAlgorithm *kmeans, bool useSSS) {
	if (kmeans->centroids != NULL)
		my_log_info("warning: a set of centroids was already defined\n");
	my_assert_greaterInt("num_dimensions", kmeans->num_dimensions, 0);
	my_assert_greaterInt("num_centroids", kmeans->num_centroids, 0);
	my_log_info_time(
			"kmeans: selecting %"PRIi64" centroids between %"PRIi64" vectors\n",
			kmeans->num_centroids, kmeans->num_vectors);
	//select
	int64_t *sample = MY_MALLOC(kmeans->num_centroids, int64_t);
	if (useSSS) {
		mknn_laesa_select_pivots_sss(kmeans->dataset, kmeans->distance,
				kmeans->num_centroids, 1, kmeans->max_threads, sample);
	} else {
		int64_t full_size = mknn_dataset_getNumObjects(kmeans->dataset);
		my_random_intList_noRepetitions(0, full_size, sample,
				kmeans->num_centroids);
	}
	//copy
	kmeans->centroids = createEmptyDataset(kmeans->num_centroids,
			kmeans->num_dimensions, kmeans->default_centroids_datatype);
	MyDatatype dtype1 = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(
					mknn_dataset_getDomain(kmeans->dataset)));
	MyDatatype dtype2 = mknn_datatype_convertMknn2My(
			mknn_domain_vector_getDimensionDataType(
					mknn_dataset_getDomain(kmeans->centroids)));
	my_function_copy_vector func_copy_vector2centroid =
			my_datatype_getFunctionCopyVector(dtype1, dtype2);
	for (int64_t i = 0; i < kmeans->num_centroids; ++i) {
		void *vector_orig = mknn_dataset_getObject(kmeans->dataset, sample[i]);
		void *centroid_dst = mknn_dataset_getObject(kmeans->centroids, i);
		func_copy_vector2centroid(vector_orig, centroid_dst,
				kmeans->num_dimensions);
	}
	free(sample);
	my_log_info_time("kmeans: reordering %"PRIi64" centroids\n",
			kmeans->num_centroids);
	kmeans->centroids = mknn_datasetLoader_reorderNearestNeighbor(
			kmeans->centroids, kmeans->distance, -1, true);
	kmeans->release_centroids = true;
	initDatasetCentroids(kmeans);
}
void mknn_kmeans_initCentroidsRandom(MknnKmeansAlgorithm *kmeans) {
	selectCentroids(kmeans, false);
}
void mknn_kmeans_initCentroidsSSS(MknnKmeansAlgorithm *kmeans) {
	selectCentroids(kmeans, true);
}
static bool testFinalized(MknnKmeansAlgorithm *kmeans, int64_t numIteration,
		double secondsProcess, int64_t contMovedVectors,
		int64_t contMovedCenters) {
	if (contMovedVectors == 0) {
		my_log_info_time("kmeans: finalized ok (%"PRIi64" iterations).\n",
				numIteration);
		return true;
	} else if (kmeans->end.maxIteration > 0
			&& numIteration >= kmeans->end.maxIteration) {
		my_log_info_time("kmeans: early stop due to %"PRIi64" iterations.\n",
				numIteration);
		return true;
	} else if (kmeans->end.maxSecondsProcess > 0
			&& secondsProcess >= kmeans->end.maxSecondsProcess) {
		my_log_info_time(
				"kmeans: early stop due to %1.1lf seconds (%"PRIi64" iterations).\n",
				secondsProcess, numIteration);
		return true;
	} else if (kmeans->end.numMinMovedVectors > 0
			&& contMovedVectors <= kmeans->end.numMinMovedVectors) {
		my_log_info_time(
				"kmeans: early stop due to %"PRIi64" vectors moved (%"PRIi64" iterations).\n",
				contMovedVectors, numIteration);
		return true;
	} else if (kmeans->end.numMinMovedCentroids > 0
			&& contMovedCenters <= kmeans->end.numMinMovedCentroids) {
		my_log_info_time(
				"kmeans: early stop due to %"PRIi64" centroids moved (%"PRIi64" iterations).\n",
				contMovedCenters, numIteration);
		return true;
	}
//test if the same assignation happened before
//this is an approximation of comparing every old assignment
	bool allEqual = false;
	for (int64_t i = NUM_PREVS - 1; i > 0; --i) {
		allEqual = true;
		for (int64_t j = 0; j < kmeans->num_centroids; ++j) {
			struct Kmeans_CentroidData *c = kmeans->centroids_data + j;
			if (c->num_elements != c->num_elements_prevs[i]) {
				allEqual = false;
				break;
			}
		}
		if (allEqual)
			break;
	}
	if (allEqual) {
		kmeans->contIterationsSameAssignations++;
		if (kmeans->contIterationsSameAssignations < 3) {
			my_log_info_time(
					"kmeans: the current distribution is identical a previous one, maybe the algorithm is looping.\n");
		} else {
			my_log_info_time(
					"kmeans: early stop due to %"PRIi64" times with same distribution (%"PRIi64" iterations).\n",
					kmeans->contIterationsSameAssignations, numIteration);
			return true;
		}
	}
	return false;
}
static void printIterationProgress(MknnKmeansAlgorithm *kmeans,
		int64_t numIteration, double seconds, int64_t contMovedVectors,
		int64_t contMovedCenters) {
	double rate = (seconds == 0) ? 0 : numIteration / seconds;
	char *rateStr;
	if (rate > 100)
		rateStr = my_newString_format("(%1.0lf iter/sec)", rate);
	else if (rate > 10)
		rateStr = my_newString_format("(%1.1lf iter/sec)", rate);
	else if (rate > 1)
		rateStr = my_newString_format("(%1.2lf iter/sec)", rate);
	else if (rate > 0)
		rateStr = my_newString_format("(%1.1lf sec/iter)", 1 / rate);
	else
		rateStr = my_newString_string("");
	char *globalIteration;
	if (kmeans->savedPriorNumIterations > 0)
		globalIteration = my_newString_format(" (total %"PRIi64")",
				numIteration + kmeans->savedPriorNumIterations);
	else
		globalIteration = my_newString_string("");
	char *st0 = my_newString_hhmmss(seconds);
	char *st1 = my_newString_doubleDec(
			(100.0 * contMovedVectors) / kmeans->num_vectors, 1);
	char *st2 = my_newString_doubleDec(
			(100.0 * contMovedCenters) / kmeans->num_centroids, 1);
	my_log_info_time(
			"kmeans at iteration %"PRIi64"%s, time %s %s, %"PRIi64" moved vectors (%s%%), %"PRIi64" moved centroids (%s%%)\n",
			numIteration, globalIteration, st0, rateStr, contMovedVectors, st1,
			contMovedCenters, st2);
	MY_FREE_MULTI(rateStr, globalIteration, st0, st1, st2);
}
static void performKmeansOnSubset(MknnKmeansAlgorithm *kmeans_full,
		struct SubsetRun *subrun) {
	int64_t sampleSize = my_math_getFractionSize(subrun->fractionSubsetSize,
			kmeans_full->num_vectors);
	MknnDataset *dataset_sample = mknn_datasetLoader_SubsetRandomSample(
			kmeans_full->dataset, sampleSize, false);
	MknnKmeansAlgorithm *subkmeans = mknn_kmeans_new();
	mknn_kmeans_setDataset(subkmeans, dataset_sample);
	mknn_kmeans_setDistance(subkmeans, kmeans_full->distance);
	mknn_kmeans_setNumCentroids(subkmeans, kmeans_full->num_centroids);
	mknn_kmeans_setInitialCentroids(subkmeans, kmeans_full->centroids, false);
	mknn_kmeans_setMaxThreads(subkmeans, kmeans_full->max_threads);
	mknn_kmeans_setTermitationCriteria(subkmeans, subrun->end.maxIteration,
			subrun->end.maxSecondsProcess, subrun->end.fractionMinMovedVectors,
			subrun->end.fractionMinMovedCentroids);
	mknn_kmeans_setParametersMknnIndex(subkmeans, kmeans_full->paramsIndex);
	mknn_kmeans_setParametersMknnResolver(subkmeans,
			kmeans_full->paramsResolver);
	if (kmeans_full->filenameAutoSaveState != NULL)
		mknn_kmeans_setAutoSaveState(subkmeans,
				kmeans_full->filenameAutoSaveState,
				kmeans_full->secondsAutoSaveState);
	subkmeans->secondsToPrintInfo = 20;
	mknn_kmeans_perform(subkmeans);
	mknn_kmeans_release(subkmeans);
	mknn_dataset_release(dataset_sample);
}
static void mknn_kmeans_runSubsets(MknnKmeansAlgorithm *kmeans) {
	my_log_info_time("kmeans: RUNNNING ON %"PRIi64" SUBSETS.\n",
			my_vectorObj_size(kmeans->subsetsRuns));
	for (int64_t i = 0; i < my_vectorObj_size(kmeans->subsetsRuns); ++i) {
		struct SubsetRun *subrun = my_vectorObj_get(kmeans->subsetsRuns, i);
		char *st = my_newString_double(subrun->fractionSubsetSize);
		my_log_info_time("kmeans: SUBSET %"PRIi64"/%"PRIi64" (size=%s).\n",
				i + 1, my_vectorObj_size(kmeans->subsetsRuns), st);
		free(st);
		performKmeansOnSubset(kmeans, subrun);
	}
	my_log_info_time("kmeans: RUNS ON SUBSETS FINALIZED OK\n");
}
void mknn_kmeans_perform(MknnKmeansAlgorithm *kmeans) {
	if (mknn_dataset_getNumObjects(kmeans->dataset) == 0)
		return;
	if (kmeans->centroids == NULL)
		mknn_kmeans_initCentroidsSSS(kmeans);
	initDistances(kmeans);
	if (kmeans->subsetsRuns != NULL) {
		mknn_kmeans_runSubsets(kmeans);
	}
	printStartLog(kmeans);
	MyTimer *timerProcess = my_timer_new();
	MyTimer *timerSaveState = my_timer_new();
	MyTimer *timerPrintInfo = my_timer_new();
	int64_t numIteration = 0;
	if (kmeans->iteration_callback_function != NULL) {
		kmeans->iteration_callback_function(kmeans, numIteration, false,
				kmeans->iteration_callback_state_pointer);
	}
	for (;;) {
		assignVectors(kmeans);
		adjustCentroids(kmeans);
		int64_t contMovedCenters = 0, contMovedVectors = 0;
		for (int64_t j = 0; j < kmeans->num_centroids; ++j) {
			struct Kmeans_CentroidData *c = kmeans->centroids_data + j;
			if (c->cont_lastMovedIn > 0 || c->cont_lastMovedOut > 0) {
				contMovedCenters++;
				contMovedVectors += c->cont_lastMovedIn;
			}
		}
		numIteration++;
		double secondsProcess = my_timer_getSeconds(timerProcess);
		bool finalized = testFinalized(kmeans, numIteration, secondsProcess,
				contMovedVectors, contMovedCenters);
		if (!finalized
				&& my_timer_getSeconds(timerPrintInfo)
						> kmeans->secondsToPrintInfo) {
			printIterationProgress(kmeans, numIteration, secondsProcess,
					contMovedVectors, contMovedCenters);
			my_timer_updateToNow(timerPrintInfo);
		}
		if (kmeans->filenameAutoSaveState != NULL
				&& (finalized
						|| my_timer_getSeconds(timerSaveState)
								> kmeans->secondsAutoSaveState)) {
			if (kmeans->stats != NULL) {
				releaseClusteringStats(kmeans->stats);
				kmeans->stats = NULL;
			}
			saveProcessState(kmeans, kmeans->filenameAutoSaveState,
					numIteration, secondsProcess, contMovedVectors,
					contMovedCenters);
			if (kmeans->stats != NULL) {
				releaseClusteringStats(kmeans->stats);
				kmeans->stats = NULL;
			}
			my_timer_updateToNow(timerSaveState);
		}
		if (kmeans->iteration_callback_function != NULL) {
			kmeans->iteration_callback_function(kmeans, numIteration, finalized,
					kmeans->iteration_callback_state_pointer);
		}
		if (finalized)
			break;
	}
	my_timer_release(timerProcess);
	my_timer_release(timerSaveState);
	my_timer_release(timerPrintInfo);
}

MknnDataset *mknn_kmeans_getDataset(MknnKmeansAlgorithm *kmeans) {
	return kmeans->dataset;
}
MknnDistance *mknn_kmeans_getDistance(MknnKmeansAlgorithm *kmeans) {
	return kmeans->distance;
}
int64_t mknn_kmeans_getNumCentroids(MknnKmeansAlgorithm *kmeans) {
	return kmeans->num_centroids;
}
void mknn_kmeans_setMaxThreads(MknnKmeansAlgorithm *kmeans, int64_t max_threads) {
	kmeans->max_threads = max_threads;
}
void mknn_kmeans_setTermitationCriteria(MknnKmeansAlgorithm *kmeans,
		int64_t maxIteration, double maxSecondsProcess,
		double pctOrNumberMinMovedVectors, double pctOrNumberMinMovedCentroids) {
	kmeans->end.maxIteration = maxIteration;
	kmeans->end.maxSecondsProcess = maxSecondsProcess;
	kmeans->end.fractionMinMovedVectors = pctOrNumberMinMovedVectors;
	kmeans->end.fractionMinMovedCentroids = pctOrNumberMinMovedCentroids;
	kmeans->end.numMinMovedVectors = my_math_getFractionSize(
			pctOrNumberMinMovedVectors, kmeans->num_vectors);
	kmeans->end.numMinMovedCentroids = my_math_getFractionSize(
			pctOrNumberMinMovedCentroids, kmeans->num_centroids);
}
void mknn_kmeans_addSubsetRun(MknnKmeansAlgorithm *kmeans,
		double pctOrNumberSampleSize, int64_t terminationMaxIteration,
		double terminationMaxSecondsProcess,
		double terminationPctOrNumberMinMovedVectors,
		double terminationPctOrNumberMinMovedCentroids) {
	if (kmeans->subsetsRuns == NULL)
		kmeans->subsetsRuns = my_vectorObj_new();
	struct SubsetRun *subrun = MY_MALLOC(1, struct SubsetRun);
	subrun->fractionSubsetSize = pctOrNumberSampleSize;
	subrun->end.maxIteration = terminationMaxIteration;
	subrun->end.maxSecondsProcess = terminationMaxSecondsProcess;
	subrun->end.fractionMinMovedVectors = terminationPctOrNumberMinMovedVectors;
	subrun->end.fractionMinMovedCentroids =
			terminationPctOrNumberMinMovedCentroids;
	my_vectorObj_add(kmeans->subsetsRuns, subrun);
}
void mknn_kmeans_addDefaultSubsetRuns(MknnKmeansAlgorithm *kmeans) {
	MyVectorDouble *sample_sizes = my_vectorDouble_new();
	int64_t curr = kmeans->num_centroids * 10;
	for (;;) {
		int64_t next = curr * 3;
		if (next <= kmeans->num_vectors)
			my_vectorDouble_add(sample_sizes, curr);
		else
			break;
		curr = next;
	}
	double fractionMinMovedVectors = 0.001;
	double fractionMinMovedCentroids = 0.01;
	for (int64_t i = 0; i < my_vectorDouble_size(sample_sizes); ++i) {
		double sample_size = my_vectorDouble_get(sample_sizes, i);
		mknn_kmeans_addSubsetRun(kmeans, sample_size, kmeans->end.maxIteration,
				kmeans->end.maxSecondsProcess, fractionMinMovedVectors,
				fractionMinMovedCentroids);
	}
	my_vectorDouble_release(sample_sizes);
}
void mknn_kmeans_setDefaultCentroidsDatatype(MknnKmeansAlgorithm *kmeans,
		MknnDatatype datatype) {
	kmeans->default_centroids_datatype = datatype;
}
MknnDatatype mknn_kmeans_getDefaultCentroidsDatatype(
		MknnKmeansAlgorithm *kmeans) {
	return kmeans->default_centroids_datatype;
}
void mknn_kmeans_setInitialCentroids(MknnKmeansAlgorithm *kmeans,
		MknnDataset *centroids_dataset, bool release_centroids_on_release) {
	if (kmeans->centroids != NULL)
		my_log_info_time(
				"kmeans: warning, a set of centroids was already defined\n");
	kmeans->centroids = centroids_dataset;
	kmeans->release_centroids = release_centroids_on_release;
	initDatasetCentroids(kmeans);
}
void mknn_kmeans_setParametersMknnIndex(MknnKmeansAlgorithm *kmeans,
		const char *parameters_mknn_index) {
	MY_FREE(kmeans->paramsIndex);
	kmeans->paramsIndex = my_newString_string(parameters_mknn_index);
}
void mknn_kmeans_setParametersMknnResolver(MknnKmeansAlgorithm *kmeans,
		const char *parameters_mknn_resolver) {
	MY_FREE(kmeans->paramsResolver);
	kmeans->paramsResolver = my_newString_string(parameters_mknn_resolver);
}
void mknn_kmeans_loadState(MknnKmeansAlgorithm *kmeans,
		const char *filenameSavedState) {
	my_log_info_time("kmeans: restoring state from %s\n", filenameSavedState);
	loadStatsClustering(kmeans, filenameSavedState);
}
void mknn_kmeans_setAutoSaveState(MknnKmeansAlgorithm *kmeans,
		const char *filenameOutput, double secondsAutoSave) {
	kmeans->filenameAutoSaveState = my_newString_string(filenameOutput);
	kmeans->secondsAutoSaveState = secondsAutoSave;
}
void mknn_kmeans_saveState(MknnKmeansAlgorithm *kmeans,
		const char *filenameOutput) {
	saveProcessState(kmeans, filenameOutput, 0, 0, 0, 0);
}
void mknn_kmeans_setIterationCallBackFunction(MknnKmeansAlgorithm *kmeans,
		mknn_kmeans_function_callback function, void *state_pointer) {
	kmeans->iteration_callback_function = function;
	kmeans->iteration_callback_state_pointer = state_pointer;
}
MknnDataset *mknn_kmeans_getCentroids(MknnKmeansAlgorithm *kmeans,
bool dont_release_centroids_on_kmeans_release) {
	kmeans->release_centroids = !dont_release_centroids_on_kmeans_release;
	return kmeans->centroids;
}
int64_t *mknn_kmeans_getAssignations(MknnKmeansAlgorithm *kmeans,
bool dont_release_assignations_on_kmeans_release) {
	kmeans->release_assignations = !dont_release_assignations_on_kmeans_release;
	return kmeans->assignations;
}
MknnDataset *mknn_kmeans_getAssignationsDataset(MknnKmeansAlgorithm *kmeans,
		int64_t id_cluster) {
	int64_t num_elems = kmeans->centroids_data[id_cluster].num_elements;
	int64_t *positions = MY_MALLOC(num_elems, int64_t);
	int64_t cont = 0;
	for (int64_t j = 0; j < kmeans->num_vectors; ++j) {
		if (kmeans->assignations[j] != id_cluster)
			continue;
		positions[cont++] = j;
	}
	my_assert_equalInt("num_assignations", cont, num_elems);
	MknnDataset *subset = mknn_datasetLoader_SubsetPositions(kmeans->dataset,
			positions, num_elems, false);
	free(positions);
	return subset;
}
static void computeClusteringStats_thread(int64_t currentProcess, void* state,
		int64_t current_thread) {
	MknnKmeansAlgorithm *kmeans = state;
	struct Kmeans_Stats *stats = kmeans->stats;
	int64_t id_cluster = currentProcess;
	struct MknnKmeansStatsCluster *c = stats->stats_clusters + id_cluster;
	void *centroid = mknn_dataset_getObject(kmeans->centroids, id_cluster);
	struct MyDataStatsCompute *dsc = my_math_computeStats_new(1);
	my_math_computeStats_addSample funcAddSample =
			my_math_computeStats_getAddSampleFunction(MY_DATATYPE_FLOAT64);
	c->sum_squared_error = c->average_squared_error = c->num_assignations = 0;
	for (int64_t j = 0; j < kmeans->num_vectors; ++j) {
		if (kmeans->assignations[j] != id_cluster)
			continue;
		void *vector_orig = mknn_dataset_getObject(kmeans->dataset, j);
		double dist_to_centroid = mknn_distanceEval_eval(
				kmeans->distEvals_vector2centroid[current_thread], vector_orig,
				centroid);
		funcAddSample(dsc, &dist_to_centroid);
		double squared_error =
				mknn_distanceEval_eval(
						kmeans->distEvals_euclideanSquared_vector2centroid[current_thread],
						vector_orig, centroid);
		c->num_assignations++;
		c->sum_squared_error += squared_error;
		c->average_squared_error += (squared_error - c->average_squared_error)
				/ c->num_assignations;
	}
	struct MyDataStats ds = { 0 };
	my_math_computeStats_getStats(dsc, 0, &ds);
	my_math_computeStats_release(dsc);
	c->distances_minimum = ds.minimum;
	c->distances_maximum = ds.maximum;
	c->distances_average = ds.average;
	c->distances_variance = ds.variance;
	c->distances_std_dev = ds.std_dev;
	c->distances_skewness = ds.skewness;
	c->distances_kurtosis = ds.kurtosis;
}
static void computeClusteringStats(MknnKmeansAlgorithm *kmeans) {
	if (kmeans->stats != NULL)
		return;
	struct Kmeans_Stats *stats = MY_MALLOC(1, struct Kmeans_Stats);
	kmeans->stats = stats;
	stats->stats_clusters = MY_MALLOC(kmeans->num_centroids,
			struct MknnKmeansStatsCluster);
	my_parallel_incremental(kmeans->num_centroids, kmeans,
			computeClusteringStats_thread, NULL, kmeans->max_threads);
	stats->stats_clustering = MY_MALLOC(1, struct MknnKmeansStatsClustering);
	stats->stats_clustering->sum_squared_error =
			stats->stats_clustering->average_squared_error = 0;
	for (int64_t i = 0; i < kmeans->num_centroids; ++i) {
		stats->stats_clustering->sum_squared_error +=
				stats->stats_clusters[i].sum_squared_error;
		stats->stats_clustering->average_squared_error +=
				(stats->stats_clusters[i].sum_squared_error
						- stats->stats_clustering->average_squared_error)
						/ (i + 1);
	}
}
static void releaseClusteringStats(struct Kmeans_Stats *stats) {
	if (stats == NULL)
		return;
	free(stats->stats_clustering);
	free(stats->stats_clusters);
	free(stats);
}
struct MknnKmeansStatsCluster *mknn_kmeans_getStatsCluster(
		MknnKmeansAlgorithm *kmeans, int64_t id_cluster) {
	if (kmeans->stats == NULL)
		computeClusteringStats(kmeans);
	return kmeans->stats->stats_clusters + id_cluster;
}

struct MknnKmeansStatsClustering *mknn_kmeans_getStatsClustering(
		MknnKmeansAlgorithm *kmeans) {
	if (kmeans->stats == NULL)
		computeClusteringStats(kmeans);
	return kmeans->stats->stats_clustering;
}

void mknn_kmeans_release(MknnKmeansAlgorithm *kmeans) {
	if (kmeans->release_assignations && kmeans->assignations != NULL)
		free(kmeans->assignations);
	if (kmeans->release_centroids && kmeans->centroids != NULL)
		mknn_dataset_release(kmeans->centroids);
	if (kmeans->release_distance && kmeans->distance != NULL)
		mknn_distance_release(kmeans->distance);
	MY_FREE(kmeans->paramsResolver);
	MY_FREE(kmeans->paramsIndex);
	MY_FREE(kmeans->centroids_data);
	MY_FREE(kmeans->filenameAutoSaveState);
	mknn_distanceEval_releaseArray(
			kmeans->distEvals_euclideanSquared_vector2centroid,
			kmeans->max_threads);
	mknn_distanceEval_releaseArray(kmeans->distEvals_vector2centroid,
			kmeans->max_threads);
	mknn_distanceEval_releaseArray(kmeans->distEvals_centroid2centroid,
			kmeans->max_threads);
	mknn_distance_release(kmeans->distance_euclideanSquared);
//MY_FREE_MULTI(kmeans->dist_1NN_estimated, kmeans->dist_2NN_estimated);
	free(kmeans);
}
