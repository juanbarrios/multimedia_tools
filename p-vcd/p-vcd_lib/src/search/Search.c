/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"
#include <metricknn/metricknn_c/metricknn_impl.h>

static MknnDataset *select_dataset(const char *name, MknnDataset *query,
		MknnDataset *reference, MknnDataset *both) {
	if (my_string_equals("Q", name))
		return query;
	else if (my_string_equals("R", name))
		return reference;
	else if (my_string_equals("QR", name))
		return both;
	my_log_error("unknown %s\n", name);
	return NULL;
}
static void ss_create_histogram(CmdParams *cmd_params, const char *argOption) {
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info("     -profile profile_name\n");
		my_log_info("     -numSamples num\n");
		my_log_info("     -numBins num\n");
		my_log_info("     -src (Q|R|QR)\n");
		my_log_info("     -dest (Q|R|QR)\n");
		return;
	}
	const char *profile_name = NULL, *src = NULL, *dest = NULL;
	int64_t numSamples = 0, numBins = 0;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-profile")) {
			profile_name = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-numSamples")) {
			numSamples = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-numBins")) {
			numBins = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-src")) {
			src = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-dest")) {
			dest = nextParam(cmd_params);
		} else {
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
		}
	}
	my_assert_notNull("profile", profile_name);
	my_assert_notNull("src", src);
	my_assert_notNull("dest", dest);
	my_assert_greaterInt("numSamples", numSamples, 0);
	my_assert_greaterInt("numBins", numBins, 0);
	set_logfile(profile_name, cmd_params, "");
	struct SearchProfile *profile = loadProfile(profile_name);
	loadDescriptorsInProfile(profile);
	MknnDataset *query = profile_get_query_globalDescriptors(profile);
	MknnDataset *reference = profile_get_reference_globalDescriptors(profile);
	MknnDataset *two_ds[2] = { query, reference };
	MknnDataset *both = mknn_datasetLoader_Concatenate(2, two_ds, false);
	MknnDistance *distance = profile_get_mdistance(profile);
	MknnDataset *dataset_src = select_dataset(src, query, reference, both);
	MknnDataset *dataset_dst = select_dataset(dest, query, reference, both);
	int64_t max_threads = NUM_CORES;
	double *samples = MY_MALLOC(numSamples, double);
	mknn_sample_distances_multithread(dataset_src, dataset_dst, numSamples,
			distance, max_threads, samples, NULL, NULL);
	struct MyDataStats stats = my_math_computeStats(numSamples, samples);
	char *string_stats = my_math_statsDetail_newString(&stats, "#", "\t", "\n");
	my_log_info("%s", string_stats);
	MknnHistogram *mknn_hist = mknn_histogram_new(numSamples, samples, numBins);
	char *fn1 = my_newString_format("%s/histogram-%s_%s.hist",
			profile->path_profile, src, dest);
	char *fn2 = my_newString_format("%s/histogram-%s_%s.png",
			profile->path_profile, src, dest);
	mknn_histogram_save(mknn_hist, fn1);
	IplImage *img = my_image_newImageHistogram(
			mknn_histogram_getNumBins(mknn_hist),
			mknn_histogram_getBins(mknn_hist));
	my_image_save(img, fn2);
	my_image_release(img);
	mknn_histogram_release(mknn_hist);
	free(fn1);
	free(fn2);
	free(samples);
	unloadDescriptorsInProfile(profile);
	releaseProfile(profile);
#endif
}

static void ss_new_printSearchFiles(struct SearchCollection *c) {
	int64_t i;
	for (i = 0; i < c->numFiles; ++i) {
		struct SearchFile *arc = c->sfiles[i];
		my_log_info("%s\n", arc->name);
	}
}

static char *ss_new_normalizeDbName(const char *name) {
	int64_t n = my_string_indexOf(name, "%");
	if (n < 0)
		return my_io_getAbsolutPath(name);
	char *pre = my_subStringI_fromTo(name, 0, n);
	char *pos = my_subStringI_fromEnd(name, n);
	char *abs = my_io_getAbsolutPath(pre);
	char *newPath = my_newString_concat(abs, pos);
	MY_FREE_MULTI(pre, pos, abs);
	return newPath;
}
static void ss_new_profile(CmdParams *cmd_params, const char *argOption) {
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"   -profile profile_name           Mandatory. New profile to create.\n");
		my_log_info(
				"   -query nameDB                   Mandatory. One or more (will be concatenated in a single collection). 'R' is the reference collection. DBOptions: %s\n",
				getDbLoadOptions());
		my_log_info(
				"   -reference nameDB               Mandatory. One or more (will be concatenated in a single collection). DBOptions: %s\n",
				getDbLoadOptions());
		my_log_info(
				"   -desc nameDescriptor [+]        One or more (will be combined in a multi-object). Same descriptor for Q and R.\n");
		my_log_info(
				"      [-descQ nameDescriptor]      Descriptor only for Q.\n");
		my_log_info(
				"      [-descR nameDescriptor]      Descriptor only for R.\n");
		my_log_info(
				"   -distance ID_DIST,PARAMS        Mandatory. Distance between descriptors.\n");
		my_log_info(
				"   -distanceCustom ID,PARAMS         Optional. Custom distance between segments.\n");
		my_log_info("   [-printSearchFiles]\n");
		return;
	}
	struct LoadSearchProfileParams params = { 0 };
	params.paramQ.dbsNames = my_vectorString_new();
	params.paramR.dbsNames = my_vectorString_new();
	params.paramQ.descriptorsAlias = my_vectorString_new();
	params.paramR.descriptorsAlias = my_vectorString_new();
	bool printSearchFiles = false;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-profile")) {
			params.path_profile = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-query")) {
			if (isNextParam(cmd_params, "R"))
				params.is_Q_equal_R = 1;
			else
				my_vectorString_add(params.paramQ.dbsNames,
						ss_new_normalizeDbName(nextParam(cmd_params)));
		} else if (isNextParam(cmd_params, "-reference")) {
			my_vectorString_add(params.paramR.dbsNames,
					ss_new_normalizeDbName(nextParam(cmd_params)));
		} else if (isNextParam(cmd_params, "-desc")) {
			const char *s = nextParam(cmd_params);
			my_vectorStringConst_add(params.paramQ.descriptorsAlias, s);
			my_vectorStringConst_add(params.paramR.descriptorsAlias, s);
		} else if (isNextParam(cmd_params, "-descQ")) {
			my_vectorStringConst_add(params.paramQ.descriptorsAlias,
					nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-descR")) {
			my_vectorStringConst_add(params.paramR.descriptorsAlias,
					nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-distance")) {
			params.id_dist = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-distanceCustom")) {
			params.id_dist_custom = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-printSearchFiles")) {
			printSearchFiles = true;
		} else {
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
		}
	}
	my_assert_notNull("profile", params.path_profile);
	my_assert_greaterInt("reference",
			my_vectorString_size(params.paramR.dbsNames), 0);
	my_assert_greaterInt("descs",
			my_vectorString_size(params.paramQ.descriptorsAlias)
					+ my_vectorString_size(params.paramR.descriptorsAlias), 0);
	my_assert_greaterInt("descsR",
			my_vectorString_size(params.paramR.descriptorsAlias), 0);
	if (!params.is_Q_equal_R) {
		my_assert_greaterInt("dbsQ",
				my_vectorString_size(params.paramQ.dbsNames), 0);
		my_assert_greaterInt("descsQ",
				my_vectorString_size(params.paramQ.descriptorsAlias), 0);
	}
	if (params.id_dist == NULL && params.id_dist_custom == NULL)
		my_log_error("distance or distanceCustom is needed\n");
	char *filename = my_newString_format("%s/profile.def", params.path_profile);
	my_assert_fileNotExists(filename);
	my_io_createDir(params.path_profile, 0);
	set_logfile(params.path_profile, cmd_params, "");
	struct SearchProfile *profile = loadProfileParams(&params);
	MknnDistance *distance = profile_get_mdistance(profile);
	if (printSearchFiles) {
		my_log_info("QUERY FILES\n");
		ss_new_printSearchFiles(profile->colQuery);
		my_log_info("REFERENCE FILES\n");
		ss_new_printSearchFiles(profile->colReference);
	}
	mknn_distance_release(distance);
	releaseProfile(profile);
	saveProfileParams(filename, &params);
}

void ss_resolve_search(CmdParams *cmd_params, const char *argOption);

int pvcd_similaritySearch(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s -new ...\n", getBinaryName(cmd_params));
		my_log_info("%s -histogram ...\n", getBinaryName(cmd_params));
		my_log_info("%s -ss ...\n", getBinaryName(cmd_params));
		return pvcd_system_exit_error();
	}
	const char *option = nextParam(cmd_params);
	if (my_string_equals(option, "-new")) {
		ss_new_profile(cmd_params, option);
	} else if (my_string_equals(option, "-histogram")) {
		ss_create_histogram(cmd_params, option);
	} else if (my_string_equals(option, "-ss")) {
		ss_resolve_search(cmd_params, option);
	} else {
		my_log_error("unknown parameter %s\n", option);
	}
	return pvcd_system_exit_ok(cmd_params);
}
