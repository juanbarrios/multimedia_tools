/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_KMEANS_H_
#define MKNN_KMEANS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

typedef struct MknnKmeansAlgorithm MknnKmeansAlgorithm;

typedef void (*mknn_kmeans_function_callback)(MknnKmeansAlgorithm *kmeans,
		int64_t num_iteration, bool is_last_iteration, void *state_pointer);

/**
 * An empty object that can run kmeans algorithm.
 * Next methods should be mknn_kmeans_setDataset, mknn_kmeans_setDistance, mknn_kmeans_setNumCentroids or mknn_kmeans_loadState.
 * @return
 */
MknnKmeansAlgorithm *mknn_kmeans_new();

/**
 *
 * @param kmeans
 * @param dataset The set of vectors to compute the clustering.
 */
void mknn_kmeans_setDataset(MknnKmeansAlgorithm *kmeans, MknnDataset *dataset);
/**
 *
 * @param kmeans
 * @param distance the distance to perform the clustering. It can also be loaded from a previously saved state.
 */
void mknn_kmeans_setDistance(MknnKmeansAlgorithm *kmeans,
		MknnDistance *distance);

/**
 *
 * @param kmeans
 * @param num_centroids the number of centroids to compute. It can also be loaded from a previously saved state.
 */
void mknn_kmeans_setNumCentroids(MknnKmeansAlgorithm *kmeans,
		int64_t num_centroids);

MknnDataset *mknn_kmeans_getDataset(MknnKmeansAlgorithm *kmeans);
MknnDistance *mknn_kmeans_getDistance(MknnKmeansAlgorithm *kmeans);
int64_t mknn_kmeans_getNumCentroids(MknnKmeansAlgorithm *kmeans);

/**
 *
 * @param kmeans
 * @param max_threads maximum number o threads to use. By default it is the number
 * of cores reported by the OS.
 */
void mknn_kmeans_setMaxThreads(MknnKmeansAlgorithm *kmeans,
		int64_t max_threads);

void mknn_kmeans_setTermitationCriteria(MknnKmeansAlgorithm *kmeans,
		int64_t maxIteration, double maxSecondsProcess,
		double pctOrNumberMinMovedVectors, double pctOrNumberMinMovedCentroids);

void mknn_kmeans_addSubsetRun(MknnKmeansAlgorithm *kmeans,
		double pctOrNumberSampleSize, int64_t terminationMaxIteration,
		double terminationMaxSecondsProcess,
		double terminationPctOrNumberMinMovedVectors,
		double terminationPctOrNumberMinMovedCentroids);
void mknn_kmeans_addDefaultSubsetRuns(MknnKmeansAlgorithm *kmeans);

void mknn_kmeans_setDefaultCentroidsDatatype(MknnKmeansAlgorithm *kmeans,
		MknnDatatype datatype);
MknnDatatype mknn_kmeans_getDefaultCentroidsDatatype(
		MknnKmeansAlgorithm *kmeans);

void mknn_kmeans_initCentroidsRandom(MknnKmeansAlgorithm *kmeans);
void mknn_kmeans_initCentroidsSSS(MknnKmeansAlgorithm *kmeans);
void mknn_kmeans_setInitialCentroids(MknnKmeansAlgorithm *kmeans,
		MknnDataset *centroids_dataset, bool release_centroids_on_release);

void mknn_kmeans_setParametersMknnIndex(MknnKmeansAlgorithm *kmeans,
		const char *parameters_mknn_index);
void mknn_kmeans_setParametersMknnResolver(MknnKmeansAlgorithm *kmeans,
		const char *parameters_mknn_resolver);

void mknn_kmeans_selectRandomCentroids(MknnKmeansAlgorithm *kmeans);
void mknn_kmeans_loadState(MknnKmeansAlgorithm *kmeans,
		const char *filenameSavedState);
void mknn_kmeans_setAutoSaveState(MknnKmeansAlgorithm *kmeans,
		const char *filenameOutput, double secondsAutoSave);
void mknn_kmeans_saveState(MknnKmeansAlgorithm *kmeans,
		const char *filenameOutput);
void mknn_kmeans_setIterationCallBackFunction(MknnKmeansAlgorithm *kmeans,
		mknn_kmeans_function_callback function, void *state_pointer);

void mknn_kmeans_perform(MknnKmeansAlgorithm *kmeans);

MknnDataset *mknn_kmeans_getCentroids(MknnKmeansAlgorithm *kmeans,
bool dont_release_centroids_on_kmeans_release);

int64_t *mknn_kmeans_getAssignations(MknnKmeansAlgorithm *kmeans,
bool dont_release_assignations_on_kmeans_release);

MknnDataset *mknn_kmeans_getAssignationsDataset(MknnKmeansAlgorithm *kmeans,
		int64_t id_cluster);

struct MknnKmeansStatsCluster {
	int64_t num_assignations;
	double sum_squared_error, average_squared_error;
	double distances_minimum, distances_maximum;
	double distances_average, distances_variance, distances_std_dev,
			distances_skewness, distances_kurtosis;
	double *averages_by_dimension;
	double *variances_by_dimension;
};
struct MknnKmeansStatsClustering {
	double sum_squared_error;
	double average_squared_error;
};

struct MknnKmeansStatsCluster *mknn_kmeans_getStatsCluster(
		MknnKmeansAlgorithm *kmeans, int64_t id_cluster);

struct MknnKmeansStatsClustering *mknn_kmeans_getStatsClustering(
		MknnKmeansAlgorithm *kmeans);

void mknn_kmeans_release(MknnKmeansAlgorithm *kmeans);

#ifdef __cplusplus
}
#endif

#endif
