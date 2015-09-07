/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATASET_HPP_
#define MKNN_DATASET_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

class DatasetCustom;

/**
 * Represents a set of objects of any type.
 * Objects in dataset are type @c void*. In order to use some pre-defined distance, the objects
 * in the dataset must have defined a Domain.
 */
class Dataset {
public:
	/**
	 * @name Using the dataset
	 * @{
	 */
	/**
	 * Size of the dataset
	 * @return the number of objects in @p dataset
	 */
	long long getNumObjects();

	/**
	 * Retrieves the object in position @p pos in @p dataset.
	 * @param pos the position of the object to retrieve. It must be a number between 0
	 * and <tt>getNumObjects - 1</tt>.
	 */
	void *getObject(long long pos);

	/**
	 * Adds an object to a dataset. The dataset must be dynamic in order to support this
	 * method (see DatasetLoader::Empty).
	 *
	 * @param object the new object to add at the end of the dataset.
	 */
	void pushObject(void *object);

	/**
	 * Returns the domain assigned to the dataset. Do not modify or free the returned domain.
	 *
	 * @return domain assigned to the dataset or NULL if no domain has been assigned.
	 */
	Domain getDomain();
	/**
	 * The objects in the dataset are stored in a single long array.
	 *
	 * The format of the created data is similar to the input described in DatasetLoader::PointerCompactVectors.
	 *
	 * The dataset must contain vectors (i.e., the domain in the dataset must belong to some constant in mknn::Domain::GENERAL_DOMAIN_VECTOR.
	 *
	 * The type of the returned array (<tt>float*</tt>, <tt>double*</tt>, ...) depends on the domain datatype.
	 *
	 * The returned pointer is cached, thus consecutive calls do not compact again the vectors. The cache is released
	 * together with the dataset. If an object is added to a dynamic dataset
	 * after the call to this method, the cache is released it will not be added.
	 *
	 * @return a pointer to an array with all the vectors one after the other. The vector will be released by
	 * delete (it must not be freed).
	 */
	void *getCompactVectors();

	/**
	 * It saves the objects in a dataset in text format. This method is useful for
	 * reviewing the objects that are inside a dataset.
	 * @param filename_write Filename to write or overwrite.
	 */
	void printObjectsToTextFile(std::string filename_write);

	/**
	 * @}
	 */

	/**
	 * @name Load/Save datasets
	 * @{
	 */
	/**
	 * The dataset is saved to a file.
	 * It may create other files using @p filename_write as prefix.
	 * The created files may be in binary format for fast loading.
	 *
	 * @param filename_write File to create. If the file already exists it is overwritten.
	 */
	void save(std::string filename_write);

	/**
	 * Loads a dataset from a file. The file must have been created by save.
	 *
	 * @param filename_read File to read. If the file does not exists an error is raised.
	 * @return a new dataset (it must be released with delete).
	 */
	static Dataset restore(std::string filename_read);

	/**
	 * @}
	 */

	/**
	 * Default constructor.
	 */
	Dataset();
	/**
	 * Default destructor.
	 */
	virtual ~Dataset();
	/**
	 * Copy constructor.
	 */
	Dataset(const Dataset &other);
	/**
	 * Assignment operator.
	 */
	Dataset &operator=(const Dataset &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class DatasetLoader;
	friend class Distance;
	friend class Index;
	friend class PredefDistance;
	friend class Resolver;
};

}
#endif
