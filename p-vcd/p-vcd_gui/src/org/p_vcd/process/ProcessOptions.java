/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

public class ProcessOptions {
	final OneDescriptorOptions descriptorQuery;
	final OneDescriptorOptions descriptorRef;
	final SearchOptions searchOptions;
	final LocalizationOptions localizationOptions;

	public ProcessOptions(OneDescriptorOptions descriptorQuery,
			OneDescriptorOptions descriptorRef, SearchOptions searchOptions,
			LocalizationOptions localizationOptions) {
		this.descriptorQuery = descriptorQuery;
		this.descriptorRef = descriptorRef;
		this.searchOptions = searchOptions;
		this.localizationOptions = localizationOptions;
	}

	static class OneDescriptorOptions {
		final String seg, segAlias, des, desAlias;

		private OneDescriptorOptions(String seg, String segAlias, String des,
				String desAlias) {
			this.seg = seg;
			this.segAlias = segAlias;
			this.des = des;
			this.desAlias = desAlias;
		}

	}

	static class SearchOptions {

		final String distance;
		final Integer knnSearch;

		final String indexBuildOptions, indexSearchOptions, indexFilename;

		final boolean isSearchByLocal;
		final Integer knnSearchAfterMerge;

		private SearchOptions(String distance, Integer knnSearch,
				String indexBuildOptions, String indexSearchOptions,
				String indexFilename, boolean isSearchByLocal,
				Integer knnSearchAfterMerge) {
			this.distance = distance;
			this.knnSearch = knnSearch;
			this.indexBuildOptions = indexBuildOptions;
			this.indexSearchOptions = indexSearchOptions;
			this.indexFilename = indexFilename;
			this.isSearchByLocal = isSearchByLocal;
			this.knnSearchAfterMerge = knnSearchAfterMerge;
		}

	}

	static class LocalizationOptions {
		final Integer maxDetections;
		final double minLengthSec, missCost, rankWeight;

		private LocalizationOptions(Integer maxDetections, double minLengthSec,
				double missCost, double rankWeight) {
			this.maxDetections = maxDetections;
			this.minLengthSec = minLengthSec;
			this.missCost = missCost;
			this.rankWeight = rankWeight;
		}

		Object[] getOptions() {
			return new Object[] { "-maxDetections", maxDetections,
					"-minLength", minLengthSec + "s", "-missCost", missCost,
					"-rankWeight", rankWeight };
		}

	}

	private static final OneDescriptorOptions GLOBAL_EHD = new OneDescriptorOptions(
			"SEGCTE_1/3", "033seg", "AVG_1U_EHD_4x4_8x8_5_K10_8F",
			"EH10P_033seg");

	private static final OneDescriptorOptions LOCAL_SIFT = new OneDescriptorOptions(
			"SEGCTE_1/2", "05seg", "CV_SIFT_SIFT_UCHAR,BV900x150", "siftH150_05seg");

	private static final SearchOptions SCAN_GLOBAL = new SearchOptions("L1", 3,
			"LINEARSCAN", null, null, false, null);

	private static final SearchOptions KDTREE_GLOBAL = new SearchOptions("L1",
			3, "FLANN-KDTREE,num_trees=5", "num_checks=100", "kdtree-5-l1",
			false, null);

	private static final SearchOptions LAESA_GLOBAL = new SearchOptions("L1",
			3, "LAESA,num_pivots=5,sets_eval=8",
			"method=APPROX,approximation=0.1", "laesa-5-l1", false, null);

	private static final SearchOptions SCAN_LOCAL = new SearchOptions(
			"LOWE,0.8_L2", 3, "LINEARSCAN", null, null, false, null);

	private static final SearchOptions KDTREE_LOCAL = new SearchOptions("L1",
			10, "FLANN-KDTREE,num_trees=2", "num_checks=100", "kdtree-2-l1",
			true, 3);

	private static final LocalizationOptions DEFAULT_LOCALIZATION = new LocalizationOptions(
			10, 5, -0.1, 0.99);

	public static final ProcessOptions getDefaultGlobalOptions() {
		return new ProcessOptions(GLOBAL_EHD, GLOBAL_EHD, KDTREE_GLOBAL,
				DEFAULT_LOCALIZATION);
	}

	public static final ProcessOptions getDefaultLocalOptions() {
		return new ProcessOptions(LOCAL_SIFT, LOCAL_SIFT, KDTREE_LOCAL,
				DEFAULT_LOCALIZATION);
	}

}
