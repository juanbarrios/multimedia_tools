/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"
#include <stdio.h>

struct MknnIndexDef {
	const char *id_index;
	const char *help_build;
	const char *help_search;
	mknn_function_printHelp func_printHelp;
	mknn_function_index_new func_index_new;
	mknn_function_resolver_new func_resolver_new;
};
struct MknnIndex {
	MknnIndexParams *parameters;
	MknnDataset *search_dataset;
	MknnDistance *distance;
	bool free_search_dataset_on_index_release, free_distance_on_index_release,
			free_parameters_on_index_release;
	struct MknnIndexDef *def;
	struct MknnIndexInstance instance;
};

static MyVectorObj *index_defs = NULL;

static MyVectorObj *getIndexDefinitions() {
	if (index_defs == NULL) {
		index_defs = my_vectorObj_new();
		mknn_register_default_indexes();
	}
	return index_defs;
}
static struct MknnIndexDef *getIndexDef(const char *id_index, bool fail) {
	MyVectorObj *defs = getIndexDefinitions();
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		struct MknnIndexDef *def = my_vectorObj_get(defs, i);
		if (my_string_equals_ignorecase(id_index, def->id_index))
			return def;
	}
	if (fail) {
		my_log_error("can't find index %s\n", id_index);
		mknn_predefIndex_helpListIndexes();
		my_log_error("\n");
	}
	return NULL;
}
void metricknn_register_index(const char *id_index, const char *help_build,
		const char *help_search, mknn_function_printHelp func_printHelp,
		mknn_function_index_new func_index_new,
		mknn_function_resolver_new func_resolver_new) {
	struct MknnIndexDef *defPrev = getIndexDef(id_index, false);
	if (defPrev != NULL)
		my_log_error("duplicated index %s\n", id_index);
	struct MknnIndexDef *def = MY_MALLOC(1, struct MknnIndexDef);
	def->id_index = id_index;
	def->help_build = help_build;
	def->help_search = help_search;
	def->func_printHelp = func_printHelp;
	def->func_index_new = func_index_new;
	def->func_resolver_new = func_resolver_new;
	MyVectorObj *defs = getIndexDefinitions();
	my_vectorObj_add(defs, def);
}
bool mknn_predefIndex_testIndexId(const char *id_index) {
	MknnIndexParams *param = mknn_indexParams_newParseString(id_index);
	const char *def_id_index = mknn_indexParams_getIndexId(param);
	struct MknnIndexDef *def = getIndexDef(def_id_index, false);
	mknn_indexParams_release(param);
	return (def != NULL);
}
static void printIndexHelp(struct MknnIndexDef *def) {
	my_log_info("Help build:\n    %s%s%s\n", def->id_index,
			(def->help_build != NULL) ? "," : "",
			(def->help_build != NULL) ? def->help_build : "");
	if (def->help_search != NULL)
		my_log_info("Help search:\n    %s\n",
				(def->help_search != NULL) ? def->help_search : "");
	if (def->func_printHelp != NULL)
		def->func_printHelp(def->id_index);
}

void mknn_predefIndex_helpPrintIndex(const char *id_index) {
	struct MknnIndexDef *def = getIndexDef(id_index, true);
	printIndexHelp(def);
	my_log_error("\n");
}
void mknn_predefIndex_helpListIndexes() {
	my_log_info("Available indexes:\n");
	MyVectorObj *defs = getIndexDefinitions();
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		struct MknnIndexDef *def = my_vectorObj_get(defs, i);
		my_log_info("    %s\n", def->id_index);
	}
}

static MknnIndex *private_new_index(MknnDataset *search_dataset,
bool free_search_dataset_on_index_release, MknnDistance *distance,
bool free_distance_on_index_release, MknnIndexParams *parameters,
bool free_parameters_on_index_release) {
	const char *id_index = mknn_indexParams_getIndexId(parameters);
	if (id_index == NULL || my_string_equals_ignorecase(id_index, "HELP")) {
		mknn_predefIndex_helpListIndexes();
		my_log_error("\n");
	}
	struct MknnIndex *index = MY_MALLOC(1, struct MknnIndex);
	index->def = getIndexDef(id_index, true);
	index->parameters = parameters;
	index->search_dataset = search_dataset;
	index->distance = distance;
	index->free_search_dataset_on_index_release =
			free_search_dataset_on_index_release;
	index->free_distance_on_index_release = free_distance_on_index_release;
	index->free_parameters_on_index_release = free_parameters_on_index_release;
	index->instance = index->def->func_index_new(index->def->id_index,
			parameters, search_dataset, distance);
	return index;
}

MknnIndex *mknn_index_newPredefined(MknnIndexParams *parameters,
bool free_parameters_on_index_release, MknnDataset *search_dataset,
bool free_search_dataset_on_index_release, MknnDistance *distance,
bool free_distance_on_index_release) {
	MknnIndex *index = private_new_index(search_dataset,
			free_search_dataset_on_index_release, distance,
			free_distance_on_index_release, parameters,
			free_parameters_on_index_release);
	if (index->instance.func_index_build != NULL)
		index->instance.func_index_build(index->instance.state_index,
				index->def->id_index, parameters);
	return index;
}
const char *mknn_index_getIdPredefinedIndex(MknnIndex *index) {
	return index->def->id_index;
}
MknnIndexParams *mknn_index_getParameters(MknnIndex *index) {
	return index->parameters;
}
MknnDataset *mknn_index_getSearchDataset(MknnIndex *index) {
	return index->search_dataset;
}
MknnDistance *mknn_index_getDistance(MknnIndex *index) {
	return index->distance;
}

void mknn_index_save(MknnIndex *index, const char *filename_write) {
	if (index == NULL)
		return;
	FILE *out = my_io_openFileWrite1(filename_write);
	fprintf(out, "%s\n", mknn_indexParams_toString(index->parameters));
	fclose(out);
	if (index->instance.func_index_save != NULL) {
		char *filename = my_newString_format("%s.%s", filename_write,
				index->def->id_index);
		index->instance.func_index_save(index->instance.state_index,
				index->def->id_index, index->parameters, filename);
		free(filename);
	}
}
MknnIndex *mknn_index_restore(const char *filename_read,
		MknnDataset *search_dataset, bool free_search_dataset_on_index_release,
		MknnDistance *distance, bool free_distance_on_index_release,
		MknnIndexParams *more_parameters,
		bool free_parameters_on_index_release) {
	MknnIndexParams *parameters = more_parameters;
	if (parameters == NULL) {
		parameters = mknn_indexParams_newEmpty();
		free_parameters_on_index_release = true;
	}
	MyVectorString *lines = my_io_loadLinesFile(filename_read);
	mknn_indexParams_parseString(parameters, my_vectorString_get(lines, 0));
	my_vectorString_release(lines, true);
	MknnIndex *index = private_new_index(search_dataset,
			free_search_dataset_on_index_release, distance,
			free_distance_on_index_release, parameters,
			free_parameters_on_index_release);
	if (index->instance.func_index_load != NULL) {
		char *filename = my_newString_format("%s.%s", filename_read,
				index->def->id_index);
		index->instance.func_index_load(index->instance.state_index,
				index->def->id_index, parameters, filename);
		free(filename);
	} else if (index->instance.func_index_build != NULL) {
		index->instance.func_index_build(index->instance.state_index,
				index->def->id_index, parameters);
	}
	return index;
}

void mknn_index_release(MknnIndex *index) {
	if (index->instance.func_index_release != NULL
			&& index->instance.state_index != NULL)
		index->instance.func_index_release(index->instance.state_index);
	if (index->free_search_dataset_on_index_release)
		mknn_dataset_release(index->search_dataset);
	if (index->free_distance_on_index_release)
		mknn_distance_release(index->distance);
	if (index->free_parameters_on_index_release)
		mknn_indexParams_release(index->parameters);
	free(index);
}

MknnResolver *mknn_index_newResolver(MknnIndex *index,
		MknnResolverParams *parameters_resolver,
		bool free_parameters_on_resolver_release) {
	if (parameters_resolver == NULL) {
		parameters_resolver = mknn_resolverParams_newEmpty();
		free_parameters_on_resolver_release = true;
	}
	struct MknnResolverInstance resolverInstance =
			index->def->func_resolver_new(index->instance.state_index,
					index->def->id_index, parameters_resolver);
	return mknn_resolver_newInternal(resolverInstance, index,
			parameters_resolver, free_parameters_on_resolver_release);
}
