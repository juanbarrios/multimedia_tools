/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATASET_H_
#define MKNN_DATASET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * MknnDataset represents a set of objects of any type.
 * Objects in dataset are by default type @c void*. In order to use some pre-defined distance, the objects
 * in the dataset must have defined a MknnDomain.
 *
 * @file
 */

/**
 * Size of the dataset
 * @param dataset
 * @return the number of objects in @p dataset
 */
int64_t mknn_dataset_getNumObjects(MknnDataset *dataset);

/**
 * Retrieves the object in position @p pos in @p dataset.
 * @param dataset
 * @param pos the position of the object to retrieve. It must be a number between 0
 * and <tt>#mknn_dataset_getNumObjects - 1</tt>.
 */
void *mknn_dataset_getObject(MknnDataset *dataset, int64_t pos);

/**
 * Adds an object to a dataset. The dataset must be dynamic in order to support this
 * method (see #mknn_datasetLoader_Empty).
 *
 * @param dataset a dataset that supports adding objects.
 * @param object the new object to add at the end of @p dataset.
 */
void mknn_dataset_pushObject(MknnDataset *dataset, void *object);

/**
 * Returns the domain assigned to the dataset. Do not modify or free the returned domain.
 *
 * @param dataset
 * @return domain assigned to the dataset or NULL if no domain has been assigned.
 */
MknnDomain *mknn_dataset_getDomain(MknnDataset *dataset);

/**
 * Returns a new dataset with a copy of the each element in dataset
 * @param dataset the dataset to copy
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_dataset_clone(MknnDataset *dataset);

/**
 * The objects in the dataset are stored in a single long array.
 *
 * The format of the created data is similar to the input from #mknn_datasetLoader_PointerCompactVectors.
 *
 * The dataset must contain vectors (i.e., the domain in the dataset must belong to #MKNN_GENERAL_DOMAIN_VECTOR).
 *
 * The type of the returned array (<tt>float*</tt>, <tt>double*</tt>, ...) depends on the domain datatype.
 *
 * The returned pointer is cached and released with the dataset, therefore it will not be updated in dynamic datasets.
 *
 * @param dataset
 * @return a pointer to an array with all the vectors one after the other. The vector will be released by
 * #mknn_dataset_release (it must not be freed).
 */
void *mknn_dataset_getCompactVectors(MknnDataset *dataset);

/**
 * @param dataset
 * @param free_domain_on_dataset_release
 */
void mknn_dataset_set_free_domain_on_dataset_release(MknnDataset *dataset,
bool free_domain_on_dataset_release);

/**
 * @param dataset
 * @return free_domain_on_dataset_release
 */
bool mknn_dataset_get_free_domain_on_dataset_release(MknnDataset *dataset);

/**
 * Releases the dataset.
 * Additionally it may release other objects that were commended to be released
 * (as in #mknn_datasetLoader_PointerArray).
 *
 * @param dataset the dataset to be released.
 */
void mknn_dataset_release(MknnDataset *dataset);

/**
 * The dataset is saved to a file.
 * It may create other files using @p filename_write as prefix.
 * The created files may be in binary format for fast loading.
 *
 * @param filename_write File to create. If the file already exists it is overwritten.
 * @param dataset the dataset to save
 */
void mknn_dataset_save(MknnDataset *dataset, const char *filename_write);

/**
 * Loads a dataset from a file. The file must have been created by #mknn_dataset_save.
 *
 * @param filename_read File to read. If the file does not exists an error is raised.
 * @return a new dataset (it must be released with #mknn_dataset_release).
 */
MknnDataset *mknn_dataset_restore(const char *filename_read);

/**
 * It prints the objects in the dataset in binary format, i.e., using fwrite to write memory addresses.
 * @param dataset
 * @param filename_write File to create. If the file already exists it is overwritten.
 */
void mknn_dataset_printObjectsRawFile(MknnDataset *dataset, const char *filename_write);

/**
 * It prints the objects in the dataset in text format, i.e., converting them to string and using fprintf.
 * If the objects are vectors, the generated file can be parsed by #mknn_datasetLoader_ParseVectorFile.
 * @param dataset
 * @param filename_write File to create. If the file already exists it is overwritten.
 */
void mknn_dataset_printObjectsTextFile(MknnDataset *dataset, const char *filename_write);

/**
 * @name Concatenate dataset
 * @{
 */
/**
 * Returns the number of subdatasets that produced this dataset.
 * @param concatenate_dataset a dataset created by #mknn_datasetLoader_Concatenate.
 * @return the number of datasets
 */
int64_t mknn_dataset_concatenate_getNumSubDatasets(
		MknnDataset *concatenate_dataset);
/**
 * Returns one of the subdatasets that produced this dataset.
 * @param concatenate_dataset a dataset created by #mknn_datasetLoader_Concatenate.
 * @param num_subdataset
 * @return a subdataset.
 */
MknnDataset *mknn_dataset_concatenate_getSubDataset(
		MknnDataset *concatenate_dataset, int64_t num_subdataset);
/**
 * Given the number of an object returns two numbers: the number of the subdataset and the
 * number of the object in that subdataset which corresponds to the object in the concatenated dataset.
 *
 * @param concatenate_dataset a dataset created by #mknn_datasetLoader_Concatenate.
 * @param posObject the number of the object between 0 and #mknn_dataset_getNumObjects - 1
 * @param out_numSubdataset returns the number of the dataset. It can be used in #mknn_dataset_concatenate_getSubDataset
 * @param out_posObjectInSubdataset returns the number of the object in the subdataset. It can be used in #mknn_dataset_getObject.
 */
void mknn_dataset_concatenate_getDatasetObject(MknnDataset *concatenate_dataset,
		int64_t posObject, int64_t *out_numSubdataset,
		int64_t *out_posObjectInSubdataset);
/**
 * @}
 *
 */
/**
 * @name MultiObject dataset
 * @{
 */
/**
 * Returns the number of subdatasets that produced this dataset.
 * @param multiobject_dataset a dataset created by #mknn_datasetLoader_MultiObject.
 * @return the number of datasets
 */
int64_t mknn_dataset_multiobject_getNumSubDatasets(
		MknnDataset *multiobject_dataset);
/**
 * Returns one of the subdatasets that produced this dataset.
 * @param multiobject_dataset a dataset created by #mknn_datasetLoader_MultiObject.
 * @param num_subdataset
 * @return a subdataset.
 */
MknnDataset *mknn_dataset_multiobject_getSubDataset(
		MknnDataset *multiobject_dataset, int64_t num_subdataset);
/**
 * @}
 *
 */
/**
 * @name Custom dataset
 * @{
 */
/**
 * returns the pointer to the object used during the creation of the dataset
 * @param custom_dataset a dataset created by #mknn_datasetLoader_Custom.
 * @returns the pointer given in #mknn_datasetLoader_Custom
 */
void *mknn_dataset_custom_getDataPointer(MknnDataset *custom_dataset);
/**
 * @}
 *
 */

#ifdef __cplusplus
}
#endif

#endif
