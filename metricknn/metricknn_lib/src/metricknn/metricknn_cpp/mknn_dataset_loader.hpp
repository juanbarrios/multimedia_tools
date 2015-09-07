/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATASET_LOADER_HPP_
#define MKNN_DATASET_LOADER_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 * Abstract class that must be inherited to define a custom dataset.
 */
class DatasetCustom {
public:

	/**
	 * Return the current size of the dataset
	 *
	 * @return the number of objects stored in @p data_pointer
	 */
	virtual long long getNumObjects()= 0;
	/**
	 * Returns an object in the dataset.
	 *
	 * @param pos the position of the desired object, between 0 and num_objects-1.
	 * @return the object stored in the position @p pos in @p data_pointer
	 */
	virtual void *getObject(long long pos)= 0;

	/**
	 * Adds an object to a dynamic dataset.
	 * Note that the object must be added to the dataset through Dataset::pushObject method
	 * in order to the index knows that the dataset has changed and must be updated.
	 *
	 * @param object the new object to add at the end of @p data_pointer.
	 */
	virtual void pushObject(void *object)= 0;

	virtual ~DatasetCustom() = 0;

};

/**
 * A dataset returned by DatasetLoader::Concatenate.
 */
class DatasetConcatenate: public Dataset {
public:
	/**
	 * Returns the number of subdatasets.
	 * @return the number of subdatasets.
	 */
	long long getNumSubDatasets();
	/**
	 * Returns one of the subdatasets that produced this dataset.
	 * The dataset must created by DatasetLoader::Concatenate.
	 *
	 * @param num_subdataset from 0 to getNumSubDatasets() - 1
	 * @return a subdataset.
	 */
	Dataset getSubDataset(long long num_subdataset);
	/**
	 * Given the number of an object returns two numbers: the number of the subdataset and the
	 * number of the object in that subdataset which corresponds to the object in the concatenated dataset.
	 * The dataset must created by DatasetLoader::Concatenate.
	 *
	 * @param posObject the number of the object between 0 and #getNumObjects - 1
	 * @param out_numSubdataset returns the number of the dataset. It can be used in DatasetConcatenate::getSubDataset
	 * @param out_posObjectInSubdataset returns the number of the object in the subdataset. It can be used in Dataset::getObject.
	 */
	void getDatasetObject(long long posObject, long long *out_numSubdataset,
			long long *out_posObjectInSubdataset);

};

/**
 * A dataset returned by DatasetLoader::MultiObject.
 */
class DatasetMultiObject: public Dataset {
public:
	/**
	 * Returns the number of subdatasets.
	 * @return the number of subdatasets.
	 */
	long long getNumSubDatasets();
	/**
	 * Returns one of the subdatasets that produced this dataset.
	 * The dataset must created by DatasetLoader::Concatenate.
	 *
	 * @param num_subdataset from 0 to getNumSubDatasets() - 1
	 * @return a subdataset.
	 */
	Dataset getSubDataset(long long num_subdataset);

};

/**
 * Different Loaders
 */
class DatasetLoader {
public:

	/**
	 * Creates a new custom dataset.
	 *
	 * The custom dataset must implement the DatasetCustom abstract class.
	 *
	 * @param custom_dataset the object that stores and is used to obtain objects
	 * @param delete_custom_dataset_on_dataset_release binds the lifetime of @c custom_dataset to the new dataset object
	 * @param domain the domain for the objects returned by @c custom_dataset.
	 * @param delete_domain_on_dataset_release binds the lifetime of @p domain to this dataset.
	 *
	 * @return a new dataset object
	 */
	static Dataset Custom(DatasetCustom *custom_dataset,
			bool delete_custom_dataset_on_dataset_release, Domain domain);

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
	 * @param delete_domain_on_dataset_release binds the lifetime of @p domain to this dataset.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset PointerArray(void **object_array, long long num_objects,
			Domain domain);

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
	 * The value of <tt>vector_size</tt> is determined by
	 * @c vector_num_dimensions and @c vector_dimension_datatype (see mknn::Domain::newVector and mknn::Domain::getVectorSizeInBytes).
	 *
	 * @param vectors_header pointer to the header of the set of vectors.
	 * @param num_vectors number of object to read from @p vectors_header.
	 * @param vector_num_dimensions number of dimensions of the vectors.
	 * @param vector_dimension_datatype datatype for the vector values. See constants in mknn::Datatype.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset PointerCompactVectors(void *vectors_header,
			long long num_vectors, long long vector_num_dimensions,
			const std::string vector_dimension_datatype);

	/**
	 * Creates a new dataset by reading a text file with vectors.
	 *
	 * The format is one vector per line, each dimension separated by tab.
	 *
	 * @param filename the filename to read.
	 * @param datatype the datatype of the objects.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset ParseVectorFile(std::string filename,
			const std::string datatype);

	/**
	 * Creates a new dataset by reading a text file with strings.
	 *
	 * The format is one string per line.
	 * @param filename the filename to read.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset ParseStringsFile(std::string filename);

	/**
	 * Creates a new dataset which is the concatenation of one or more datasets.
	 *
	 * @param subdatasets the array of datasets
	 * @param delete_subdatasets_on_dataset_release all  @p subdatasets[i] must be released (by
	 * calling delete) during delete of the new dataset.
	 * @return a new dataset (it must be released with delete).
	 */
	static DatasetConcatenate Concatenate(
			const std::vector<Dataset> &subdatasets);

	/**
	 * Creates a new dataset which is a subset of a bigger dataset.
	 *
	 * @param superdataset the dataset to extract objects
	 * @param position_start position of the first objects to extract.
	 * @param length number of consecutive objects to extract, starting from @p position_start. it must be greater than zero.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset SubsetSegment(Dataset &superdataset, long long position_start,
			long long length);

	/**
	 * Creates a new dataset which is a subset of a bigger dataset.
	 *
	 * @param superdataset the dataset to extract objects
	 * @param positions each position of the objects to extract from @c superdataset. The positions are copied to an internal array.
	 * @param num_positions length of array @c positions. it must be greater than zero.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset SubsetPositions(Dataset &superdataset, long long *positions,
			long long num_positions);
	/**
	 * Creates a new dataset with random vectors of the given datatype.
	 * Each dimension is bounded in <tt>[0, dimension_max_value)</tt>.
	 *
	 * @param num_objects desired size of the dataset.
	 * @param dimension number of dimensions to generate.
	 * @param dimension_minValueIncluded the minimum value for each dimension (included).
	 * @param dimension_maxValueNotIncluded the maximum value for each dimension (not included).
	 * @param datatype the datatype of the generated vectors. See constants in mknn::Datatype.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset UniformRandomVectors(long long num_objects, long long dimension,
			double dimension_minValueIncluded,
			double dimension_maxValueNotIncluded, const std::string datatype);

	/**
	 * Creates a new dataset where each object is a multi-object. Each multi-object
	 * is created by combining one object of each subdataset. All datasets must contain
	 * the same number of objects.
	 *
	 * @param subdatasets the datasets from which multi-objects will be created.
	 * @param delete_subdatasets_on_dataset_release all  @p subdatasets[i] must be released (by
	 * calling delete) during delete of the new dataset.
	 * @return a new dataset (it must be released with delete).
	 */
	static DatasetMultiObject MultiObject(
			const std::vector<Dataset> &subdatasets);

	/**
	 * Creates a new empty dataset that can dynamically grow as new objects are added.
	 * The new objects are added by Dataset::pushObject.
	 *
	 * @param domain the domain for the objects that will be added to this dataset.
	 * @param delete_domain_on_dataset_release binds the lifetime of @p domain to this dataset.
	 * @return a new empty dataset (it must be released with delete).
	 */
	static Dataset Empty(Domain domain);

};

}
#endif
