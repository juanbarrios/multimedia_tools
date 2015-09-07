/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_INDEX_H_
#define MKNN_INDEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * A MknnIndex represents the index structure.
 *
 * In order to create an index, at least two parameters must be provided:
 *
 * @li a search dataset (#MknnDataset), where the nearest neighbors will be searched in.
 * @li a distance (#MknnDistance), the dissimilarity measure to be used to compare objects.
 *
 *  MetricKnn provides a set of pre-defined indexes identified by a
 *  unique distance identifier. Some valid IDs are:
 *
 *  @li @c LINEARSCAN The naive sequential scan of the search dataset.
 *  @li @c LAESA The LAESA metric access method. http://dx.doi.org/10.1016/0167-8655(94)90095-7
 *  @li @c FLANN-KDTREE The kd-tree, implemented by FLANN library.
 *  @li @c FLANN-KMEANS The k-means tree, implemented by FLANN library.
 *
 * The multidimensional indexes are implemented by the FLANN library. http://www.cs.ubc.ca/research/flann/
 *
 * In order to simplify the creation of distances, there are function that returns
 * the parameters for building the different indexes with #mknn_index_newPredefined.
 *
 * @note
 * The complete list of pre-defined indexes provided by MetricKnn can be printed by
 * calling #mknn_predefIndex_helpListIndexes. The parameters supported by each
 * index can be printed by calling #mknn_predefIndex_helpPrintIndex.
 *
 *  @file
 */

/**
 * Instantiates a new index identified by the given parameters
 *
 * @param parameters the definition of the index.
 * @param free_parameters_on_index_release binds the lifetime of @p parameters to the new index.
 * @param search_dataset the data to be indexed. Any search on this index will return some object from @p search_data.
 * @param free_search_dataset_on_index_release binds the lifetime of @p search_dataset to the new index.
 * @param distance a distance.
 * @param free_distance_on_index_release binds the lifetime of @p distance to the new index.
 * @return a index (must be released)
 */
MknnIndex *mknn_index_newPredefined(MknnIndexParams *parameters,
bool free_parameters_on_index_release, MknnDataset *search_dataset,
bool free_search_dataset_on_index_release, MknnDistance *distance,
bool free_distance_on_index_release);

/**
 * Returns the parameters used to create or load the index.
 * @param index the index
 * @return the parameters used to create or load the index.
 */
MknnIndexParams *mknn_index_getParameters(MknnIndex *index);

/**
 * Returns the dataset used to create or load the index.
 * @param index the index
 * @return the dataset used to create or load the index.
 */
MknnDataset *mknn_index_getSearchDataset(MknnIndex *index);
/**
 * Returns the distance used to create or load the index.
 * @param index the index
 * @return the distance used to create or load the index.
 */
MknnDistance *mknn_index_getDistance(MknnIndex *index);
/**
 * returns the id of the predefined index.
 * @param index the index
 * @return the id of the predefined index.
 */
const char *mknn_index_getIdPredefinedIndex(MknnIndex *index);
/**
 * Saves the built index to a file.
 * It may create other files using @p filename_write as prefix.
 * The created files may be in binary format for fast loading.
 *
 * @remark The search_dataset and the distance are not saved.
 *
 * @param filename_write File to create. If the file already exists it is overwritten.
 * @param index the index to save
 */
void mknn_index_save(MknnIndex *index, const char *filename_write);

/**
 * Loads an index from a file.
 * The file must have been created by #mknn_index_save.
 *
 * @remark @p search_dataset and @p distance must be the same that were
 * used to create and save the index.
 *
 * @param filename_read File to read. If the file does not exists an error is raised.
 * @param search_dataset The search dataset that was used to build the index.
 * @param free_search_dataset_on_index_release binds the lifetime of @p search_dataset to the new index.
 * @param distance The distance  that was used to build the index.
 * @param free_distance_on_index_release binds the lifetime of @p distance to the new index.
 * @param more_parameters any parameter that may override the stored parameters or new parameters that
 * could not be saved.
 * @param free_parameters_on_index_release binds the lifetime of @p more_parameters to the new index.
 * @return a new index (it must be released with #mknn_index_release).
 */
MknnIndex *mknn_index_restore(const char *filename_read,
		MknnDataset *search_dataset, bool free_search_dataset_on_index_release,
		MknnDistance *distance, bool free_distance_on_index_release,
		MknnIndexParams *more_parameters,
		bool free_parameters_on_index_release);

/**
 * Configures a new similarity search using the given index.
 *
 * @param index the index that will resolve the search.
 * @param parameters_resolver parameters to be given to the index in order to resolve the search.
 * @param free_parameters_on_resolver_release true if @p parameters_resolver must be released during #mknn_resolver_release.
 * @return an object configured to resolve a similarity search.
 */
MknnResolver *mknn_index_newResolver(MknnIndex *index,
		MknnResolverParams *parameters_resolver,
		bool free_parameters_on_resolver_release);

/**
 * Releases the index.
 *
 * @param index the index to be released.
 */
void mknn_index_release(MknnIndex *index);

#ifdef __cplusplus
}
#endif

#endif
