/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATASET_LOADER_H_
#define MKNN_DATASET_LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * There are different methods for creating or loading datasets.
 *
 * @file
 */

/**
 * @name Pointer to functions used to define custom datasets
 * @{
 */
/**
 * Function parameter for #mknn_datasetLoader_Custom.
 * Return the current size of the dataset
 *
 * @param data_pointer the object where the data is located.
 * @return the number of objects stored in @p data_pointer
 */
typedef int64_t (*mknn_function_dataset_getNumObjects)(void *data_pointer);
/**
 * Function parameter for #mknn_datasetLoader_Custom.
 * Returns an object in the dataset.
 *
 * @param data_pointer the object where the data is located.
 * @param pos the position of the desired object, between 0 and num_objects-1.
 * @return the object stored in the position @p pos in @p data_pointer
 */
typedef void *(*mknn_function_dataset_getObject)(void *data_pointer,
		int64_t pos);

/**
 * Function parameter for #mknn_datasetLoader_Custom.
 * Adds an object to a dynamic dataset.
 *
 * @param data_pointer the object where the data is located.
 * @param object the new object to add at the end of @p data_pointer.
 */
typedef void (*mknn_function_dataset_pushObject)(void *data_pointer,
		void *object);

/**
 * Function parameter for #mknn_datasetLoader_Custom.
 * Releases the storage of the dataset.
 *
 * @param data_pointer the object to release.
 */
typedef void (*mknn_function_dataset_releaseDataPointer)(void *data_pointer);

/**
 * Function parameter for #mknn_dataset_pushObject
 * Releases the storage of the object.
 *
 * @param object_pointer the object to release.
 */
typedef void (*mknn_function_dataset_releaseObject)(void *object_pointer);

/**
 * @}
 */

/**
 * Creates a new custom dataset.
 *
 * The data is stored by pointer @p data_pointer and @c func_get_object is invoked
 * in order to get all the objects.
 *
 * @param data_pointer a pointer to an object with data.
 * @param func_getNumObjects current size of the dataset stored in @p data_pointer.
 * @param func_getObject the function to invoke for retrieving the object in any
 * position between @c 0 and <tt>num_objects - 1</tt>.
 * @param func_pushObject the function to invoke for adding an object to the dataset. NULL if
 * the dataset is static.
 * @param func_releaseDataPointer the function to be invoked by #mknn_dataset_release for
 * releasing @c data_pointer (or NULL if it is not needed).
 * @param domain the domain for all the objects in the dataset.
 * @param free_domain_on_dataset_release flag to @p domain be released (by
 * calling #mknn_domain_release) during #mknn_dataset_release of the new dataset.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_Custom(void *data_pointer,
		mknn_function_dataset_getNumObjects func_getNumObjects,
		mknn_function_dataset_getObject func_getObject,
		mknn_function_dataset_pushObject func_pushObject,
		mknn_function_dataset_releaseDataPointer func_releaseDataPointer,
		MknnDomain *domain, bool free_domain_on_dataset_release);

/**
 * Creates a new dataset from an array of objects.
 *
 * The objects are read from @c object_array in the following order:
 *
 * @li first object: <tt>object_array[0]</tt>
 * @li second object: <tt>object_array[1]</tt>
 * @li ...
 * @li last object: <tt>object_array[num_objects - 1]</tt>
 *
 * @param object_array pointer to an array of objects.
 * @param num_objects number of object to read from the array.
 * @param domain the domain for the objects in the array.
 * @param free_each_object_on_dataset_release releases each <tt>object_array[i]</tt> during  #mknn_dataset_release.
 * @param free_object_array_on_dataset_release  releases <tt>object_array</tt> during  #mknn_dataset_release.
 * @param free_domain_on_dataset_release flag to @p domain be released (by
 * calling #mknn_domain_release) during #mknn_dataset_release of the new dataset.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_PointerArray(void **object_array,
		int64_t num_objects, MknnDomain *domain,
		bool free_each_object_on_dataset_release,
		bool free_object_array_on_dataset_release,
		bool free_domain_on_dataset_release);

/**
 * Creates a new dataset from a data array.
 *
 * The objects are read from @c vectors_header in the following order:
 *
 * @li first object: <tt>vectors_header</tt>
 * @li second object: <tt>vectors_header + vector_size</tt>
 * @li ...
 * @li last object: <tt>vectors_header + (num_objects - 1) * vector_size</tt>.
 *
 * The value of <tt>vector_size</tt> is determined by #mknn_domain_vector_getVectorLengthInBytes.
 *
 * @param vectors_header pointer to the header of the set of vectors.
 * @param free_vectors_header_on_dataset_release  releases <tt>vectors_header</tt> during  #mknn_dataset_release.
 * @param num_vectors number of object to read from @p vectors_header.
 * @param vector_dimensions number of dimensions of the vectors.
 * @param vector_dimension_datatype datatype for the vector values.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_PointerCompactVectors(void *vectors_header,
bool free_vectors_header_on_dataset_release, int64_t num_vectors,
		int64_t vector_dimensions, MknnDatatype vector_dimension_datatype);

/**
 * Creates a new dataset from a data array.
 *
 * The objects are read from @c vectors_header in the following order:
 *
 * @li first object: <tt>vectors_header</tt>
 * @li second object: <tt>vectors_header + vector_size</tt>
 * @li ...
 * @li last object: <tt>vectors_header + (num_objects - 1) * vector_size</tt>.
 *
 * The value of <tt>vector_size</tt> is determined by #mknn_domain_vector_getVectorLengthInBytes.
 *
 * @param vectors_header pointer to the header of the set of vectors.
 * @param free_vectors_header_on_dataset_release  releases <tt>vectors_header</tt> during  #mknn_dataset_release.
 * @param num_vectors number of object to read from @p vectors_header.
 * @param domain the domain for the vectors in @p vectors_header.
 * @param free_domain_on_dataset_release flag to @p domain be released (by
 * calling #mknn_domain_release) during #mknn_dataset_release of the new dataset.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_PointerCompactVectors_alt(void *vectors_header,
bool free_vectors_header_on_dataset_release, int64_t num_vectors,
		MknnDomain *domain, bool free_domain_on_dataset_release);
/**
 * Creates a new dataset by reading a text file with vectors.
 *
 * The format is one vector per line, each dimension separated by tab.
 *
 * @param filename the filename to read.
 * @param datatype the datatype of the objects.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_ParseVectorFile(const char *filename,
		MknnDatatype datatype);

/**
 * Creates a new dataset by reading a text file with strings.
 *
 * The format is one string per line.
 * @param filename the filename to read.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_ParseStringsFile(const char *filename);

/**
 * Creates a new dataset which is the concatenation of one or more datasets.
 *
 * @param num_subdatasets the number of datasets to concatenate
 * @param subdatasets the array of datasets
 * @param free_subdatasets_on_dataset_release all  @p subdatasets[i] must be released (by
 * calling #mknn_dataset_release) during #mknn_dataset_release of the new dataset.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_Concatenate(int64_t num_subdatasets,
		MknnDataset **subdatasets, bool free_subdatasets_on_dataset_release);

/**
 * Creates a new dataset which is a subset of a bigger dataset.
 *
 * @param superdataset the dataset to extract objects
 * @param position_start position of the first objects to extract.
 * @param length number of consecutive objects to extract, starting from @p position_start. it must be greater than zero.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_SubsetSegment(MknnDataset *superdataset,
		int64_t position_start, int64_t length,
		bool free_superdataset_on_release);

/**
 * Creates a new dataset which is a subset of a bigger dataset.
 *
 * @param superdataset the dataset to extract objects
 * @param positions each position of the objects to extract from @c superdataset. The positions are copied to an internal array.
 * @param num_positions length of array @c positions. it must be greater than zero.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_SubsetPositions(MknnDataset *superdataset,
		int64_t *positions, int64_t num_positions,
		bool free_superdataset_on_release);

/**
 * Creates a new dataset as a random sample of superdataset.
 * @param superdataset
 * @param free_superdataset_on_release
 * @param sample_size_or_fraction if @>= 1 is number of elements to sample from superdataset,
 * if between 0 and 1 (exclusive) is the fraction of superdataset elements to sample.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_SubsetRandomSample(MknnDataset *superdataset,
		double sample_size_or_fraction, bool free_superdataset_on_release);

/**
 * Creates a new dataset with random vectors of the given datatype.
 * Each dimension is bounded in <tt>[0, dimension_max_value)</tt>.
 *
 * @param num_objects desired size of the dataset.
 * @param dimension number of dimensions to generate.
 * @param dimension_minValueIncluded the minimum value for each dimension (included).
 * @param dimension_maxValueNotIncluded the maximum value for each dimension (not included).
 * @param vectors_dimension_datatype the datatype of the generated vectors.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_UniformRandomVectors(int64_t num_objects,
		int64_t dimension, double dimension_minValueIncluded,
		double dimension_maxValueNotIncluded,
		MknnDatatype vectors_dimension_datatype);

/**
 * Creates a new dataset where each object is a multi-object. Each multi-object
 * is created by combining one object of each subdataset. All datasets must contain
 * the same number of objects.
 *
 * @param num_subdatasets number of subdatasets to combine, i.e., the size of the multi-object
 * to be created.
 * @param subdatasets the datasets from which multi-objects will be created.
 * @param free_subdatasets_on_dataset_release all  @p subdatasets[i] must be released (by
 * calling #mknn_dataset_release) during #mknn_dataset_release of the new dataset.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_MultiObject(int64_t num_subdatasets,
		MknnDataset **subdatasets, bool free_subdatasets_on_dataset_release);

/**
 * Creates a new empty dataset that can dynamically grow as new objects are added.
 * The new objects are added by #mknn_dataset_pushObject.
 * @param domain the domain for all the objects to be added to the dataset.
 * @param free_domain_on_dataset_release flag to @p domain be released (by
 * calling #mknn_domain_release) during #mknn_dataset_release of the new dataset.
 * @return a new empty dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_Empty(MknnDomain *domain,
bool free_domain_on_dataset_release);

/**
 * Creates a new dataset as a random permutation of superdataset.
 * @param superdataset
 * @param free_superdataset_on_release
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_reorderRandomPermutation(
		MknnDataset *superdataset,
		bool free_superdataset_on_release);

/**
 * Creates a new dataset which is a permutation of superdataset, where the first position
 * is given by start_position and following are the consecutive nearest neighbors of the previous one.
 * @param superdataset
 * @param distance
 * @param start_position the seed object, -1 means the object closer to zero (valid only for vectors).
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_datasetLoader_reorderNearestNeighbor(
		MknnDataset *superdataset, MknnDistance *distance,
		int64_t start_position,
		bool free_superdataset_on_release);

#ifdef __cplusplus
}
#endif

#endif
