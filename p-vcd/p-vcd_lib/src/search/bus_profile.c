/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

struct SearchProfile *loadProfileParams(struct LoadSearchProfileParams *params) {
	struct SearchProfile *profile = MY_MALLOC(1, struct SearchProfile);
	profile->colReference = loadSearchCollection(&params->paramR);
	if (params->is_Q_equal_R) {
		profile->colQuery = profile->colReference;
	} else {
		profile->colQuery = loadSearchCollection(&params->paramQ);
	}
	my_assert_equalInt("numModalities", profile->colQuery->numModalities,
			profile->colReference->numModalities);
	int64_t i;
	for (i = 0; i < profile->colQuery->numModalities; ++i) {
		DescriptorType tdq = profile->colQuery->modalities[i];
		DescriptorType tdr = profile->colReference->modalities[i];
		assertEqualDescriptorType(tdq, tdr);
	}
	profile->path_profile = my_newString_string(params->path_profile);
	profile->id_dist = my_newString_string(params->id_dist);
	profile->id_dist_custom = my_newString_string(params->id_dist_custom);
	return profile;
}
static MyVectorString *readArr(MyMapStringObj *prop, char *name) {
	MyVectorString *arr = my_vectorString_new();
	char *st = my_newString_format("%s_length", name);
	int64_t i, max = my_parse_int(my_mapStringObj_get(prop, st));
	MY_FREE(st);
	for (i = 0; i < max; ++i) {
		char *tmp = my_newString_format("%s_%"PRIi64"", name, i);
		my_vectorString_add(arr, my_mapStringObj_get(prop, tmp));
		free(tmp);
	}
	return arr;
}
struct SearchProfile *loadProfile(const char *profile_name) {
	MyTimer *timer = my_timer_new();
	char *filename = my_newString_format("%s/profile.def", profile_name);
	MyMapStringObj *prop = my_io_loadProperties(filename, 0, "PVCD",
			"struct SearchProfile", 1, 2);
	if (prop == NULL)
		my_log_error("could not find profile '%s' (%s does not exist)\n",
				profile_name, filename);
	char *absFilename = my_io_getAbsolutPath(filename);
	char *profile_path = my_io_getDirname(absFilename);
	MY_FREE_MULTI(filename, absFilename);
	struct LoadSearchProfileParams params = { 0 };
	params.paramR.dbsNames = readArr(prop, "db_R");
	params.paramR.descriptorsAlias = readArr(prop, "descriptor_R");
	params.is_Q_equal_R =
			my_string_equals("true",
					my_mapStringObj_get(prop, "is_Q_equal_R")) ? 1 : 0;
	if (!params.is_Q_equal_R) {
		params.paramQ.dbsNames = readArr(prop, "db_Q");
		params.paramQ.descriptorsAlias = readArr(prop, "descriptor_Q");
	}
	params.id_dist = my_mapStringObj_get(prop, "id_dist");
	params.id_dist_custom = my_mapStringObj_get(prop, "id_dist_custom");
	params.path_profile = profile_path;
	struct SearchProfile *profile = loadProfileParams(&params);
	my_vectorString_release(params.paramR.dbsNames, false);
	my_vectorString_release(params.paramR.descriptorsAlias, false);
	if (!params.is_Q_equal_R) {
		my_vectorString_release(params.paramQ.dbsNames, false);
		my_vectorString_release(params.paramQ.descriptorsAlias, false);
	}
	my_mapStringObj_release(prop, 1, 1);
	double ss = my_timer_getSeconds(timer);
	if (ss > 1)
		my_log_info("profile %s loaded in %1.1lf seconds\n", profile_name, ss);
	my_timer_release(timer);
	return profile;
}
struct SearchProfile *loadProfileFromFile(const char *filename_in_profile) {
	my_assert_fileExists(filename_in_profile);
	if (my_string_indexOf(filename_in_profile, "/") < 0)
		return loadProfile(".");
	char *path_profile = my_subStringC_startLast(filename_in_profile, '/');
	struct SearchProfile *profile = loadProfile(path_profile);
	MY_FREE(path_profile);
	return profile;
}
void releaseProfile(struct SearchProfile *profile) {
	if (profile == NULL)
		return;
	releaseCollection(profile->colReference);
	if (profile->colQuery != profile->colReference)
		releaseCollection(profile->colQuery);
	MY_FREE(profile->path_profile);
	MY_FREE(profile);
}
static void printNameVal(FILE *out, const char *name, const char *val) {
	if (val != NULL)
		fprintf(out, "%s=%s\n", name, val);
}
static void printArr(FILE *out, const char *name, MyVectorString *vals) {
	fprintf(out, "%s_length=%"PRIi64"\n", name, my_vectorString_size(vals));
	int64_t i;
	for (i = 0; i < my_vectorString_size(vals); ++i)
		fprintf(out, "%s_%"PRIi64"=%s\n", name, i,
				my_vectorString_get(vals, i));
}
void saveProfileParams(const char *filename,
		struct LoadSearchProfileParams *params) {
	FILE *out = my_io_openFileWrite1Config(filename, "PVCD",
			"struct SearchProfile", 1, 2);
	printArr(out, "db_R", params->paramR.dbsNames);
	printArr(out, "descriptor_R", params->paramR.descriptorsAlias);
	if (params->is_Q_equal_R) {
		printNameVal(out, "is_Q_equal_R", "true");
	} else {
		printArr(out, "db_Q", params->paramQ.dbsNames);
		printArr(out, "descriptor_Q", params->paramQ.descriptorsAlias);
	}
	printNameVal(out, "id_dist", params->id_dist);
	printNameVal(out, "id_dist_custom", params->id_dist_custom);
	fclose(out);
}
void loadDescriptorsInProfile(struct SearchProfile *profile) {
	loadDescriptorsInCollection(profile->colReference);
	if (profile->colQuery != profile->colReference)
		loadDescriptorsInCollection(profile->colQuery);
}
void unloadDescriptorsInProfile(struct SearchProfile *profile) {
	unloadDescriptorsInCollection(profile->colReference);
	if (profile->colQuery != profile->colReference)
		unloadDescriptorsInCollection(profile->colQuery);
}
