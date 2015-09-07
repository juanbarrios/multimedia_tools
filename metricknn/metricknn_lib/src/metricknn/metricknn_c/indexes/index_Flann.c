/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#ifndef NO_FLANN

#include <flann/flann.h>

struct FlannDataType {
	bool is_byte, is_int, is_float, is_double;
	const char *type_name;
	MknnDatatype dt;
	int num_dimensions;
};
static flann_index_t libflann_build_index(void *vectors_dataset,
		int num_vectors_dataset, struct FlannDataType flanntype,
		struct FLANNParameters* flann_params_index) {
	my_log_info_time("building flann index for dataset %i x %i %s\n",
			num_vectors_dataset, flanntype.num_dimensions, flanntype.type_name);
	if (num_vectors_dataset <= 0 || flanntype.num_dimensions <= 0)
		my_log_error("cannot create an index for %i x %i\n",
				num_vectors_dataset, flanntype.num_dimensions);
	float speedup = 0;
	flann_index_t fnn_index = NULL;
	if (flanntype.is_byte)
		fnn_index = flann_build_index_byte((unsigned char*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions, &speedup,
				flann_params_index);
	else if (flanntype.is_int)
		fnn_index = flann_build_index_int((int*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions, &speedup,
				flann_params_index);
	else if (flanntype.is_float)
		fnn_index = flann_build_index_float((float*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions, &speedup,
				flann_params_index);
	else if (flanntype.is_double)
		fnn_index = flann_build_index_double((double*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions, &speedup,
				flann_params_index);
	if (fnn_index == NULL)
		my_log_error("flann returned an error at flann_build_index\n");
	if (speedup != 0)
		my_log_info("flann index speedup %f\n", speedup);
	my_log_info_time("flann index built ok\n");
	return fnn_index;
}
static flann_index_t libflann_load_index(const char *filenameRead,
		void *vectors_dataset, int num_vectors_dataset,
		struct FlannDataType flanntype) {
	char *filename = my_newString_string(filenameRead);
	char *mb = my_newString_diskSpace(my_io_getFilesize(filename));
	my_log_info_time("loading flann index for dataset %i x %i %s (%s)\n",
			num_vectors_dataset, flanntype.num_dimensions, flanntype.type_name,
			mb);
	flann_index_t fnn_index = NULL;
	if (flanntype.is_byte)
		fnn_index = flann_load_index_byte(filename,
				(unsigned char*) vectors_dataset, num_vectors_dataset,
				flanntype.num_dimensions);
	else if (flanntype.is_int)
		fnn_index = flann_load_index_int(filename, (int*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions);
	else if (flanntype.is_float)
		fnn_index = flann_load_index_float(filename, (float*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions);
	else if (flanntype.is_double)
		fnn_index = flann_load_index_double(filename, (double*) vectors_dataset,
				num_vectors_dataset, flanntype.num_dimensions);
	if (fnn_index == NULL)
		my_log_error("flann returned an error at flann_load_index from %s\n",
				filename);
	my_log_info_time("flann index loaded ok\n");
	free(filename);
	free(mb);
	return fnn_index;
}
static void libflann_save_index(flann_index_t fnn_index,
		struct FlannDataType flanntype, const char *filenameWrite) {
	char *filename = my_newString_string(filenameWrite);
	my_log_info_time("saving flann index...\n");
	int st = -1;
	if (flanntype.is_byte)
		st = flann_save_index_byte(fnn_index, filename);
	else if (flanntype.is_int)
		st = flann_save_index_int(fnn_index, filename);
	else if (flanntype.is_float)
		st = flann_save_index_float(fnn_index, filename);
	else if (flanntype.is_double)
		st = flann_save_index_double(fnn_index, filename);
	if (st != 0)
		my_log_error("flann returned an error at flann_save_index to %s\n",
				filename);
	char *mb = my_newString_diskSpace(my_io_getFilesize(filename));
	my_log_info_time("flann index saved ok (%s)\n", mb);
	free(filename);
	free(mb);
}
static void libflann_free_index(flann_index_t fnn_index,
		struct FlannDataType flanntype,
		struct FLANNParameters* flann_params_index) {
	int st = -1;
	if (flanntype.is_byte)
		st = flann_free_index_byte(fnn_index, flann_params_index);
	else if (flanntype.is_int)
		st = flann_free_index_int(fnn_index, flann_params_index);
	else if (flanntype.is_float)
		st = flann_free_index_float(fnn_index, flann_params_index);
	else if (flanntype.is_double)
		st = flann_free_index_double(fnn_index, flann_params_index);
	if (st != 0)
		my_log_error("flann returned an error at flann_free_index\n");
}
struct FlannNNResult {
	int *indices;
	double *distances_d;
	float *distances_f;
};
static struct FlannNNResult libflann_find_nearest_neighbors(
		flann_index_t fnn_index, struct FlannDataType flanntype,
		void *vectors_query, int num_vectors_query, int knn,
		struct FLANNParameters *flann_params_search) {
	int num_cells = knn * num_vectors_query;
	int *indices = MY_MALLOC(num_cells, int);
	double *distances_d = NULL;
	float *distances_f = NULL;
	int st = -1;
	if (flanntype.is_byte) {
		distances_f = MY_MALLOC(num_cells, float);
		st = flann_find_nearest_neighbors_index_byte(fnn_index,
				(unsigned char*) vectors_query, num_vectors_query, indices,
				distances_f, knn, flann_params_search);
	} else if (flanntype.is_int) {
		distances_f = MY_MALLOC(num_cells, float);
		st = flann_find_nearest_neighbors_index_int(fnn_index,
				(int*) vectors_query, num_vectors_query, indices, distances_f,
				knn, flann_params_search);
	} else if (flanntype.is_float) {
		distances_f = MY_MALLOC(num_cells, float);
		st = flann_find_nearest_neighbors_index_float(fnn_index,
				(float*) vectors_query, num_vectors_query, indices, distances_f,
				knn, flann_params_search);
	} else if (flanntype.is_double) {
		distances_d = MY_MALLOC(num_cells, double);
		st = flann_find_nearest_neighbors_index_double(fnn_index,
				(double*) vectors_query, num_vectors_query, indices,
				distances_d, knn, flann_params_search);
	}
	if (st != 0)
		my_log_error("flann returned an error at find_nearest_neighbors\n");
	struct FlannNNResult fnn_result = { 0 };
	fnn_result.indices = indices;
	fnn_result.distances_d = distances_d;
	fnn_result.distances_f = distances_f;
	return fnn_result;
}
static void libflann_set_distance(MknnDistance *distance) {
	const char *id_dist = mknn_distance_getIdPredefinedDistance(distance);
	if (my_string_equals(id_dist, "L1")) {
		flann_set_distance_type(FLANN_DIST_L1, 0);
	} else if (my_string_equals(id_dist, "L2")) {
		flann_set_distance_type(FLANN_DIST_L2, 0);
	} else if (my_string_equals(id_dist, "LP")) {
		MknnDistanceParams *param = mknn_distance_getParameters(distance);
		int order = my_math_round_int(
				mknn_distanceParams_getDouble(param, "order"));
		if (order == 1)
			flann_set_distance_type(FLANN_DIST_L1, 0);
		else if (order == 2)
			flann_set_distance_type(FLANN_DIST_L2, 0);
		else
			flann_set_distance_type(FLANN_DIST_MINKOWSKI, order);
	} else if (my_string_equals(id_dist, "CHI2")) {
		flann_set_distance_type(FLANN_DIST_CHI_SQUARE, 0);
	} else if (my_string_equals(id_dist, "HAMMING")) {
		flann_set_distance_type(FLANN_DIST_HAMMING, 0);
	} else if (my_string_equals(id_dist, "HELLINGER")) {
		flann_set_distance_type(FLANN_DIST_HELLINGER, 0);
	} else {
		my_log_error("flann does not support distance %s\n", id_dist);
	}
}

static struct FLANNParameters getFLANNParameters_index(const char *id_index,
		MknnIndexParams *params_index) {
	struct FLANNParameters params = DEFAULT_FLANN_PARAMETERS;
	params.log_level = FLANN_LOG_INFO;
	if (my_string_equals("FLANN-LINEARSCAN", id_index)) {
		params.algorithm = FLANN_INDEX_LINEAR;
	} else if (my_string_equals("FLANN-KDTREE", id_index)) {
		params.algorithm = FLANN_INDEX_KDTREE;
		/* number of randomized trees to use (for kdtree) */
		params.trees = mknn_indexParams_getInt(params_index, "num_trees");
		if (params.trees <= 0) {
			my_log_info("num_trees must be greater than 0\n");
			mknn_predefIndex_helpPrintIndex(id_index);
		}
	} else if (my_string_equals("FLANN-LSH", id_index)) {
		params.algorithm = FLANN_INDEX_LSH;
		/* The number of hash tables to use */
		params.table_number_ = mknn_indexParams_getInt(params_index,
				"table_number");
		/* The length of the key in the hash tables */
		params.key_size_ = mknn_indexParams_getInt(params_index, "key_size");
		/* Number of levels to use in multi-probe LSH, 0 for standard LSH */
		params.multi_probe_level_ = mknn_indexParams_getInt(params_index,
				"multi_probe_level");
		if (params.table_number_ <= 0 || params.key_size_ <= 0
				|| params.multi_probe_level_ <= 0) {
			my_log_info(
					"table_number, key_size, multi_probe_level must be greater than 0\n");
			mknn_predefIndex_helpPrintIndex(id_index);
		}
	} else if (my_string_equals("FLANN-KMEANS", id_index)) {
		params.algorithm = FLANN_INDEX_KMEANS;
		/* branching factor (for kmeans tree) */
		params.branching = mknn_indexParams_getInt(params_index, "branching");
		/* max iterations to perform in one kmeans clustering (kmeans tree) */
		params.iterations = mknn_indexParams_getInt(params_index, "iterations");
		/* algorithm used for picking the initial cluster centers for kmeans tree */
		params.centers_init = FLANN_CENTERS_GONZALES;
		if (params.branching <= 0 || params.iterations <= 0) {
			my_log_info("branching and iterations must be greater than 0\n");
			mknn_predefIndex_helpPrintIndex(id_index);
		}
	} else if (my_string_equals("FLANN-AUTO", id_index)) {
		params.algorithm = FLANN_INDEX_AUTOTUNED;
		/* precision desired (used for autotuning, -1 otherwise) */
		params.target_precision = mknn_indexParams_getDouble(params_index,
				"target_precision");
		/* build tree time weighting factor */
		params.build_weight = mknn_indexParams_getDouble(params_index,
				"build_weight");
		/* index memory weigthing factor */
		params.memory_weight = mknn_indexParams_getDouble(params_index,
				"memory_weight");
		/* what fraction of the dataset to use for autotuning */
		params.sample_fraction = mknn_indexParams_getDouble(params_index,
				"sample_fraction");
		if (params.target_precision <= 0 || params.build_weight <= 0
				|| params.memory_weight <= 0 || params.sample_fraction <= 0) {
			my_log_info("FLANN-AUTO parameters must be greater than 0\n");
			mknn_predefIndex_helpPrintIndex(id_index);
		}
	} else {
		my_log_error("internal error. Unknown flann index %s\n", id_index);
	}
	return params;
}
static struct FlannDataType getFlannDataType(MknnDataset *dataset) {
	MknnDomain *domain = mknn_dataset_getDomain(dataset);
	if (!mknn_domain_isGeneralDomainVector(domain))
		my_log_error("dataset must be vector domain\n");
	struct FlannDataType flanntype = { 0 };
	flanntype.num_dimensions = mknn_domain_vector_getNumDimensions(domain);
	flanntype.dt = mknn_domain_vector_getDimensionDataType(domain);
	if (mknn_datatype_isUInt8(flanntype.dt)) {
		flanntype.is_byte = true;
		flanntype.type_name = "byte";
		my_assert_equalInt("sizeof(unsigned char)", sizeof(unsigned char), 1);
	} else if (mknn_datatype_isInt32(flanntype.dt)) {
		flanntype.is_int = true;
		flanntype.type_name = "int";
		my_assert_equalInt("sizeof(int)", sizeof(int), 4);
	} else if (mknn_datatype_isFloat(flanntype.dt)) {
		flanntype.is_float = true;
		flanntype.type_name = "float";
		my_assert_equalInt("sizeof(float)", sizeof(float), 4);
	} else if (mknn_datatype_isDouble(flanntype.dt)) {
		flanntype.is_double = true;
		flanntype.type_name = "double";
		my_assert_equalInt("sizeof(double)", sizeof(double), 8);
	} else {
		my_log_error(
				"flann does not support the datatype.\nFlann only supports datatypes: unsigned integer 8 bits, integer 32 bits, float and double.\n");
	}
	return flanntype;
}

struct Flann_Index {
	MknnDataset *search_dataset;
	struct FLANNParameters params_index;
	flann_index_t fnn_index;
	struct FlannDataType flanntype;
	void *vectors_dataset;
	int64_t num_vectors_dataset;
};
static void flann_index_save(void *state_index, const char *id_index,
		MknnIndexParams *params_index, const char *filenameWrite) {
	struct Flann_Index *state = state_index;
	libflann_save_index(state->fnn_index, state->flanntype, filenameWrite);
}
static void flann_index_release(void *state_index) {
	struct Flann_Index *state = state_index;
	libflann_free_index(state->fnn_index, state->flanntype,
			&state->params_index);
	MY_FREE(state);
}
static void flann_index_load(void *state_index, const char *id_index,
		MknnIndexParams *params_index, const char *filename_read) {
	struct Flann_Index *state = state_index;
	state->fnn_index = libflann_load_index(filename_read,
			state->vectors_dataset, state->num_vectors_dataset,
			state->flanntype);
}

static void flann_index_build(void *state_index, const char *id_index,
		MknnIndexParams *params_index) {
	struct Flann_Index *state = state_index;
	state->fnn_index = libflann_build_index(state->vectors_dataset,
			state->num_vectors_dataset, state->flanntype, &state->params_index);
}
static struct MknnIndexInstance flann_index_new(const char *id_index,
		MknnIndexParams *params_index, MknnDataset *search_dataset,
		MknnDistance *distance) {
	struct Flann_Index *state = MY_MALLOC(1, struct Flann_Index);
	state->search_dataset = search_dataset;
	state->flanntype = getFlannDataType(search_dataset);
	libflann_set_distance(distance);
	state->params_index = getFLANNParameters_index(id_index, params_index);
	state->params_index.cores = my_parallel_getNumberOfCores();
	state->vectors_dataset = mknn_dataset_getCompactVectors(search_dataset);
	state->num_vectors_dataset = mknn_dataset_getNumObjects(search_dataset);
	struct MknnIndexInstance newIdx = { 0 };
	newIdx.state_index = state;
	newIdx.func_index_build = flann_index_build;
	newIdx.func_index_load = flann_index_load;
	newIdx.func_index_save = flann_index_save;
	newIdx.func_index_release = flann_index_release;
	return newIdx;
}
/*********************************************************/
struct Flann_Search {
	int64_t knn;
	struct Flann_Index *state_index;
	struct FLANNParameters params_search;
};
static MknnResult *flann_resolver_search(void *state_resolver,
		MknnDataset *query_dataset) {
	struct Flann_Search *state = state_resolver;
	int64_t num_query_objects = mknn_dataset_getNumObjects(query_dataset);
	void *vectors_query = mknn_dataset_getCompactVectors(query_dataset);
	struct FlannDataType qtype = getFlannDataType(query_dataset);
	if (!mknn_datatype_areEqual(qtype.dt, state->state_index->flanntype.dt))
		my_log_error(
				"flann requires identical data types between index and query sets (%s!=%s)\n",
				state->state_index->flanntype.type_name, qtype.type_name);
	struct FlannNNResult fresult = libflann_find_nearest_neighbors(
			state->state_index->fnn_index, qtype, vectors_query,
			num_query_objects, state->knn, &state->params_search);
	MknnResult *result = mknn_result_newEmpty(num_query_objects, state->knn);
	int64_t pos = 0;
	for (int64_t i = 0; i < num_query_objects; ++i) {
		MknnResultQuery *res = mknn_result_getResultQuery(result, i);
		res->num_distance_evaluations = 0;
		res->num_nns = state->knn;
		for (int64_t j = 0; j < state->knn; ++j) {
			res->nn_distance[j] =
					(fresult.distances_d != NULL) ?
							fresult.distances_d[pos] : fresult.distances_f[pos];
			res->nn_position[j] = fresult.indices[pos];
			my_assert_indexRangeInt("NN position", res->nn_position[j],
					state->state_index->num_vectors_dataset);
			pos++;
		}
	}
	MY_FREE_MULTI(fresult.distances_d, fresult.distances_f, fresult.indices);
	return result;
}

static void flann_resolver_release(void *state_resolver) {
	struct Flann_Search *state = state_resolver;
	MY_FREE(state);
}
struct MknnResolverInstance flann_resolver_new(void *state_index,
		const char *id_index, MknnResolverParams *params_resolver) {
	struct Flann_Search *state = MY_MALLOC(1, struct Flann_Search);
	state->knn = mknn_resolverParams_getKnn(params_resolver);
	if (state->knn < 1)
		state->knn = 1;
	state->state_index = state_index;
	state->params_search = state->state_index->params_index;
	int checks = 0;
	if (mknn_resolverParams_getString(params_resolver, "num_checks") != NULL)
		checks = mknn_resolverParams_getInt(params_resolver, "num_checks");
	else if (state->state_index->params_index.algorithm
			== FLANN_INDEX_AUTOTUNED)
		checks = FLANN_CHECKS_AUTOTUNED;
	else if (state->state_index->params_index.algorithm == FLANN_INDEX_LINEAR)
		checks = FLANN_CHECKS_UNLIMITED;
	state->params_search.checks = checks;
	state->params_search.cores = mknn_resolverParams_getMaxThreads(
			params_resolver);
	if (state->params_search.cores < 1)
		state->params_search.cores = my_parallel_getNumberOfCores();
	struct MknnResolverInstance newResolver = { 0 };
	newResolver.state_resolver = state;
	newResolver.func_resolver_search = flann_resolver_search;
	newResolver.func_resolver_release = flann_resolver_release;
	return newResolver;
}
#endif
/* ******************************************************* */
void register_index_flann() {
#ifndef NO_FLANN
	metricknn_register_index("FLANN-LINEARSCAN", NULL, NULL, NULL,
			flann_index_new, flann_resolver_new);
	metricknn_register_index("FLANN-KDTREE", "num_trees=[int]",
			"num_checks=[int]", NULL, flann_index_new, flann_resolver_new);
	metricknn_register_index("FLANN-LSH",
			"table_number=[int],key_size=[int],multi_probe_level=[int]",
			"num_checks=[int]", NULL, flann_index_new, flann_resolver_new);
	metricknn_register_index("FLANN-KMEANS", "branching=[int],iterations=[int]",
			"num_checks=[int]", NULL, flann_index_new, flann_resolver_new);
	metricknn_register_index("FLANN-AUTO",
			"target_precision=[float],build_weight=[float],memory_weight=[float],sample_fraction=[float]",
			"num_checks=[int]", NULL, flann_index_new, flann_resolver_new);
#endif
}
