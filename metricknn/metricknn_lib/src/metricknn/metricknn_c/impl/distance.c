/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnDistanceDef {
	MknnGeneralDomain general_domain;
	const char *id_dist, *help_line;
	mknn_function_printHelp func_printHelp;
	mknn_function_distance_new func_distance_new;
};
struct MknnDistance {
	bool is_predef, is_custom, is_factory;
	struct {
		struct MknnDistanceDef *def;
		struct MknnDistanceInstance instance;
		MknnDistanceParams *parameters;
		bool free_parameters_on_release;
	} predef;
	struct {
		mknn_function_distanceEval_createState func_createState;
		mknn_function_distanceEval_eval func_eval;
		mknn_function_distanceEval_releaseState func_releaseState;
	} custom;
	struct {
		void *state_factory;
		mknn_function_distanceFactory_createDistEval func_factory;
		mknn_function_distanceFactory_releaseFactory func_release;
	} factory;
};

static MyVectorObj *dist_defs = NULL;

static MyVectorObj *getDistanceDefinitions() {
	if (dist_defs == NULL) {
		dist_defs = my_vectorObj_new();
		mknn_register_default_distances();
	}
	return dist_defs;
}
static struct MknnDistanceDef *getDistanceDef(const char *id_dist, bool fail) {
	MyVectorObj *defs = getDistanceDefinitions();
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		struct MknnDistanceDef *def = my_vectorObj_get(defs, i);
		if (my_string_equals_ignorecase(id_dist, def->id_dist))
			return def;
	}
	if (fail) {
		my_log_error("can't find distance %s\n", id_dist);
		mknn_predefDistance_helpListDistances();
		my_log_error("\n");
	}
	return NULL;
}

static void help_printDistancesDomain(MknnGeneralDomain general_domain);

void mknn_register_distance(MknnGeneralDomain general_domain,
		const char *id_dist, const char *help_line,
		mknn_function_printHelp func_printHelp,
		mknn_function_distance_new func_distance_new) {
	struct MknnDistanceDef *defPrev = getDistanceDef(id_dist, false);
	if (defPrev != NULL)
		my_log_error("duplicated distance %s\n", id_dist);
	struct MknnDistanceDef *def = MY_MALLOC(1, struct MknnDistanceDef);
	def->id_dist = id_dist;
	def->help_line = help_line;
	def->func_printHelp = func_printHelp;
	def->general_domain = general_domain;
	def->func_distance_new = func_distance_new;
	MyVectorObj *defs = getDistanceDefinitions();
	my_vectorObj_add(defs, def);
}
static void help_printDistancesDomain(MknnGeneralDomain general_domain) {
	MyVectorObj *defs = getDistanceDefinitions();
	bool first = true;
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		struct MknnDistanceDef *def = my_vectorObj_get(defs, i);
		if (!mknn_generalDomain_areEqual(general_domain, def->general_domain))
			continue;
		if (first)
			my_log_info("\nAvailable distances for general domain %s:\n",
					mknn_generalDomain_toString(general_domain));
		first = false;
		my_log_info("    %s%s%s\n", def->id_dist,
				(def->help_line != NULL) ? "," : "",
				(def->help_line != NULL) ? def->help_line : "");
	}
}
void mknn_predefDistance_helpListDistances() {
	help_printDistancesDomain(MKNN_GENERAL_DOMAIN_STRING);
	help_printDistancesDomain(MKNN_GENERAL_DOMAIN_VECTOR);
	help_printDistancesDomain(MKNN_GENERAL_DOMAIN_MULTIOBJECT);
}
static void printDistanceHelp(struct MknnDistanceDef *def) {
	my_log_info(" Help for distance \"%s\" general domain %s:\n", def->id_dist,
			mknn_generalDomain_toString(def->general_domain));
	if (def->help_line != NULL) {
		my_log_info("      %s,%s\n", def->id_dist, def->help_line);
	}
	if (def->func_printHelp != NULL) {
		my_log_info("\n Detailed help:\n");
		def->func_printHelp(def->id_dist);
	}
}
void mknn_predefDistance_helpPrintDistance(const char *id_dist) {
	struct MknnDistanceDef *def = getDistanceDef(id_dist, true);
	printDistanceHelp(def);
	my_log_error("\n");
}
bool mknn_predefDistance_testDistanceId(const char *id_dist) {
	MknnDistanceParams *param = mknn_distanceParams_newParseString(id_dist);
	const char *def_id_dist = mknn_distanceParams_getDistanceId(param);
	struct MknnDistanceDef *def = getDistanceDef(def_id_dist, false);
	mknn_distanceParams_release(param);
	return (def != NULL);
}
static MknnDistance *private_new_distance_predef(MknnDistanceParams *parameters,
bool free_parameters_on_release) {
	const char *id_dist = mknn_distanceParams_getDistanceId(parameters);
	if (id_dist == NULL || my_string_equals_ignorecase(id_dist, "HELP")) {
		mknn_predefDistance_helpListDistances();
		my_log_error("\n");
	}
	MknnDistance *distance = MY_MALLOC(1, MknnDistance);
	distance->is_predef = true;
	distance->predef.def = getDistanceDef(id_dist, true);
	distance->predef.parameters = parameters;
	distance->predef.free_parameters_on_release = free_parameters_on_release;
	distance->predef.instance = distance->predef.def->func_distance_new(
			distance->predef.def->id_dist, distance->predef.parameters);
	return distance;
}
MknnDistance *mknn_distance_newPredefined(MknnDistanceParams *parameters,
bool free_parameters_on_distance_release) {
	MknnDistance *distance = private_new_distance_predef(parameters,
			free_parameters_on_distance_release);
	if (distance->predef.instance.func_distance_build != NULL) {
		distance->predef.instance.func_distance_build(
				distance->predef.instance.state_distance,
				distance->predef.def->id_dist, parameters);
	}
	return distance;
}
MknnDistance *mknn_distance_newCustom(
		mknn_function_distanceEval_createState func_createState,
		mknn_function_distanceEval_eval func_eval,
		mknn_function_distanceEval_releaseState func_releaseState) {
	MknnDistance *distance = MY_MALLOC(1, MknnDistance);
	distance->is_custom = true;
	distance->custom.func_createState = func_createState;
	distance->custom.func_eval = func_eval;
	distance->custom.func_releaseState = func_releaseState;
	return distance;
}
MknnDistance *mknn_distance_newCustomFactory(void *state_factory,
		mknn_function_distanceFactory_createDistEval func_factory,
		mknn_function_distanceFactory_releaseFactory func_release) {
	MknnDistance *distance = MY_MALLOC(1, MknnDistance);
	distance->is_factory = true;
	distance->factory.state_factory = state_factory;
	distance->factory.func_factory = func_factory;
	distance->factory.func_release = func_release;
	return distance;
}
void mknn_distance_save(MknnDistance *distance, const char *filename_write) {
	if (distance == NULL || !distance->is_predef)
		return;
	FILE *out = my_io_openFileWrite1(filename_write);
	fprintf(out, "%s\n",
			mknn_distanceParams_toString(distance->predef.parameters));
	fclose(out);
	if (distance->predef.instance.func_distance_save != NULL) {
		char *filename = my_newString_format("%s.%s", filename_write,
				distance->predef.def->id_dist);
		distance->predef.instance.func_distance_save(
				distance->predef.instance.state_distance,
				distance->predef.def->id_dist, distance->predef.parameters,
				filename);
		free(filename);
	}
}
MknnDistance *mknn_distance_restore(const char *filename_read,
		MknnDistanceParams *more_parameters,
		bool free_parameters_on_distance_release) {
	MknnDistanceParams *parameters = more_parameters;
	if (parameters == NULL) {
		parameters = mknn_distanceParams_newEmpty();
		free_parameters_on_distance_release = true;
	}
	MyVectorString *lines = my_io_loadLinesFile(filename_read);
	mknn_distanceParams_parseString(parameters, my_vectorString_get(lines, 0));
	my_vectorString_release(lines, true);
	MknnDistance *distance = private_new_distance_predef(parameters,
			free_parameters_on_distance_release);
	if (distance->predef.instance.func_distance_load != NULL) {
		char *filename = my_newString_format("%s.%s", filename_read,
				distance->predef.def->id_dist);
		distance->predef.instance.func_distance_load(
				distance->predef.instance.state_distance,
				distance->predef.def->id_dist, parameters, filename);
		free(filename);
	} else if (distance->predef.instance.func_distance_build != NULL) {
		distance->predef.instance.func_distance_build(
				distance->predef.instance.state_distance,
				distance->predef.def->id_dist, parameters);
	}
	return distance;
}
void mknn_distance_release(MknnDistance *distance) {
	if (distance == NULL)
		return;
	if (distance->is_predef) {
		if (distance->predef.instance.func_distance_release != NULL)
			distance->predef.instance.func_distance_release(
					distance->predef.instance.state_distance);
		if (distance->predef.free_parameters_on_release)
			mknn_distanceParams_release(distance->predef.parameters);
	} else if (distance->is_factory) {
		if (distance->factory.func_release != NULL)
			distance->factory.func_release(distance->factory.state_factory);
	}
	free(distance);
}
const char *mknn_distance_getIdPredefinedDistance(MknnDistance *distance) {
	if (distance == NULL || !distance->is_predef)
		return NULL;
	return distance->predef.def->id_dist;
}
MknnDistanceParams *mknn_distance_getParameters(MknnDistance *distance) {
	if (distance == NULL || !distance->is_predef)
		return NULL;
	return distance->predef.parameters;
}
/* ************************************************************************** */
struct MknnDistanceEval {
	MknnDistance *distance;
	MknnDomain *domain_left;
	MknnDomain *domain_right;
	void *state_distEval;
	mknn_function_distanceEval_eval func_eval;
	mknn_function_distanceEval_releaseState func_releaseState;
};

static void validate_domain(MknnDistance *distance, MknnDomain *domain) {
	MknnGeneralDomain gd = mknn_domain_getGeneralDomain(domain);
	if (!mknn_generalDomain_areEqual(gd, distance->predef.def->general_domain))
		my_log_error("distance %s does not support domain %s\n",
				distance->predef.def->id_dist, mknn_generalDomain_toString(gd));
}
MknnDistanceEval *mknn_distance_newDistanceEval(MknnDistance *distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	if (distance->is_predef) {
		validate_domain(distance, domain_left);
		validate_domain(distance, domain_right);
	}
	MknnDistanceEval *distance_eval = MY_MALLOC(1, MknnDistanceEval);
	distance_eval->distance = distance;
	distance_eval->domain_left = domain_left;
	distance_eval->domain_right = domain_right;
	if (distance->is_predef) {
		struct MknnDistEvalInstance instance =
				distance->predef.instance.func_distanceEval_new(
						distance->predef.instance.state_distance,
						distance_eval->domain_left,
						distance_eval->domain_right);
		distance_eval->state_distEval = instance.state_distEval;
		distance_eval->func_eval = instance.func_distanceEval_eval;
		distance_eval->func_releaseState = instance.func_distanceEval_release;
	} else if (distance->is_factory) {
		distance->factory.func_factory(distance->factory.state_factory,
				distance_eval->domain_left, distance_eval->domain_right,
				&distance_eval->state_distEval, &distance_eval->func_eval,
				&distance_eval->func_releaseState);
	} else if (distance->is_custom) {
		if (distance->custom.func_createState != NULL)
			distance_eval->state_distEval = distance->custom.func_createState(
					distance_eval->domain_left, distance_eval->domain_right);
		distance_eval->func_eval = distance->custom.func_eval;
		distance_eval->func_releaseState = distance->custom.func_releaseState;
	} else {
		my_log_error("could not create a new distance\n");
	}
	if (distance_eval->func_eval == NULL)
		my_log_error("error at instantiating a distance\n");
	return distance_eval;
}

double mknn_distanceEval_evalTh(MknnDistanceEval *distance_eval,
		void *object_left, void *object_right, double current_threshold) {
	return distance_eval->func_eval(distance_eval->state_distEval, object_left,
			object_right, current_threshold);
}
double mknn_distanceEval_eval(MknnDistanceEval *distance_eval,
		void *object_left, void *object_right) {
	return distance_eval->func_eval(distance_eval->state_distEval, object_left,
			object_right, DBL_MAX);
}
MknnDistance *mknn_distanceEval_getDistance(MknnDistanceEval *distance_eval) {
	return distance_eval->distance;
}
MknnDomain *mknn_distanceEval_getDomainLeft(MknnDistanceEval *distance_eval) {
	return distance_eval->domain_left;
}
MknnDomain *mknn_distanceEval_getDomainRight(MknnDistanceEval *distance_eval) {
	return distance_eval->domain_right;
}
void mknn_distanceEval_release(MknnDistanceEval *distance_eval) {
	if (distance_eval == NULL)
		return;
	if (distance_eval->func_releaseState != NULL)
		distance_eval->func_releaseState(distance_eval->state_distEval);
	free(distance_eval);
}

MknnDistanceEval **mknn_distance_createDistEvalArray(MknnDistance *distance,
		int64_t num_instances, MknnDomain *domain_left,
		MknnDomain *domain_right) {
	MknnDistanceEval **dist_evals = MY_MALLOC(num_instances,
			MknnDistanceEval *);
	for (int64_t i = 0; i < num_instances; ++i) {
		dist_evals[i] = mknn_distance_newDistanceEval(distance, domain_left,
				domain_right);
	}
	return dist_evals;
}
void mknn_distanceEval_releaseArray(MknnDistanceEval **dist_evals,
		int64_t num_instances) {
	if (dist_evals == NULL)
		return;
	for (int64_t i = 0; i < num_instances; ++i)
		mknn_distanceEval_release(dist_evals[i]);
	free(dist_evals);
}
