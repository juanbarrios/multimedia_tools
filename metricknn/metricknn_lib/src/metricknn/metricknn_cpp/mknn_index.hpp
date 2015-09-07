/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_INDEX_HPP_
#define MKNN_INDEX_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

class Resolver;

/**
 * A Index represents the index structure.
 *
 * In order to create an index, at least two parameters must be provided:
 *
 * @li a search Dataset, where the nearest neighbors will be searched in.
 * @li a Distance, the dissimilarity measure to be used to compare objects.
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
 * In order to simplify the creation of distances, there are defined
 * constructors for the different distances which internally invoke Index::newPredefined.
 *
 * @note
 * The complete list of pre-defined indexes provided by MetricKnn can be printed by
 * calling PredefIndex::helpListIndexes. The parameters supported by each
 * index can be printed by calling PredefIndex::helpPrintIndex.
 *
 */
class Index {
public:

	/**
	 * Instantiates a new index identified by the given parameters
	 *
	 * @param parameters the definition of the index.
	 * @param delete_parameters_on_index_release binds the lifetime of @p parameters to the new index.
	 * @param search_dataset the data to be indexed. Any search on this index will return some object from @p search_data.
	 * @param delete_search_dataset_on_index_release binds the lifetime of @p search_dataset to the new index.
	 * @param distance a distance.
	 * @param delete_distance_on_index_release binds the lifetime of @p distance to the new index.
	 * @return a index (must be released)
	 */
	static Index newPredefined(IndexParams &parameters, Dataset &search_dataset,
			Distance &distance);

	/**
	 * Returns the parameters used to create or load the index.
	 * @return the parameters used to create or load the index.
	 */
	IndexParams &getParameters();
	/**
	 * Returns the dataset used to create or load the index.
	 * @return the dataset used to create or load the index.
	 */
	Dataset &getSearchDataset();
	/**
	 * Returns the distance used to create or load the index.
	 * @return the distance used to create or load the index.
	 */
	Distance &getDistance();

	/**
	 * returns the id of the predefined index.
	 * @return the id of the predefined index.
	 */
	std::string getIdPredefinedIndex();

	/**
	 * Saves the built index to a file.
	 * It may create other files using @p filename_write as prefix.
	 * The created files may be in binary format for fast loading.
	 *
	 * @remark The search_dataset is not saved and the distance factory are not saved.
	 *
	 * @param filename_write File to create. If the file already exists it is overwritten.
	 */
	void save(std::string filename_write);

	/**
	 * Loads an index from a file.
	 * The file must have been created by save.
	 *
	 * @remark @p search_dataset and @p dist_factory must be the same that were
	 * used to create and save the index.
	 *
	 * @param filename_read File to read. If the file does not exists an error is raised.
	 * @param search_dataset The search dataset that was used to build the index.
	 * @param delete_search_dataset_on_index_release binds the lifetime of @p search_dataset to the new index.
	 * @param distance The distance  that was used to build the index.
	 * @param delete_distance_on_index_release binds the lifetime of @p distance to the new index.
	 * @param more_parameters any parameter that may override the stored parameters or new parameters that
	 * could not be saved.
	 * @param delete_parameters_on_index_release binds the lifetime of @p more_parameters to the new index.
	 * @return a new index (it must be released with delete).
	 */
	static Index restore(std::string filename_read, Dataset &search_dataset,
			Distance &distance, IndexParams &more_parameters);

	/**
	 * Configures a new similarity search using the given index.
	 *
	 * @param parameters_resolver parameters to be given to the index in order to resolve the search.
	 * @param delete_parameters_on_resolver_release  binds the lifetime of @p parameters_resolver to the new resolver.
	 * @return an object configured to resolve a similarity search.
	 */
	Resolver newResolver(ResolverParams &parameters_resolver);

	/**
	 * Default constructor.
	 */
	Index();
	/**
	 * Default destructor.
	 */
	virtual ~Index();
	/**
	 * Copy constructor.
	 */
	Index(const Index &other);
	/**
	 * Assignment operator.
	 */
	Index &operator=(const Index &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;
};

}

#endif
