MetricKnn
=========

MetricKnn is an open source library to address the problem of similarity
search, i.e., to efficiently locate objects that are close to each other
according to some distance function. MetricKnn is comprised of:

  - Command Line, which enables to perform similarity searches through console commands.
  - C API, which contains the implementation for different distance functions and indexes.
  - C++ API, which is a thin C++ wrapper library for the C library.

The intended users are software developers, academics, students and data scientist who need to efficiently compare objects according to some criteria.

Some pre-defined distances are:

 * Minkowski distances, including the L1, L2 and Lmax.
 * Cosine distance.
 * Dynamic Partial Function, which is a non-metric version of Minkowski distances.
 * Earth Movers Distance, which is an interface to the OpenCV implementation.

Some available indexes are:

 * LAESA, which is a set of static pivots.
 * SnakeTable, which uses a set of dynamic pivots.
 * kd-tree, k-means tree, and LSH, which are different multi-dimensional indexes, as implemented by FLANN library.

 
FEATURES
========

  - Similarity Search for User-Defined objects and User-Defined distances.
  - Several pre-defined distances and dataset loaders.
  - Multi-threaded searches.
  - Metric Access Methods index vectors as well as User-Defined objects with User-Defined distance.
  - Easy to compare the performance of different distances and indexes.
  - Read/Write vectors in binary files for fast loading of datasets.

 
COMMAND LINE
============

Invoking the command without options will print a brief description of the options.

	metricknn

The following command produces a file query.txt with 4 random vectors and a file reference.txt
with 10000 random vectors. Vectors are 20-d, each dimension between 0 and 100.

	metricknn
		-query_vectors_random      4     20 0 100 float
		-reference_vectors_random  10000 20 0 100 float
		-query_print_txt           query.txt
		-reference_print_txt       reference.txt

The following command locates the 3-Nearest-Neighbors for each vector in query.txt, according to euclidean distance:

	metricknn
		-query_vectors_file      query.txt      float
        -reference_vectors_file  reference.txt  float
        -distance L2
        -knn 3
        -threads 4

The Command Line documentation can be seen here: http://www.metricknn.org/doc/metricknn_cli/index.html


C++ API
=======

The same search implemented using the C++ API is:

	// distance parameters, euclidean distance
	mknn::DistanceParams *distanceParams = mknn::PredefDistance::L2();
	//index parameters, the metric index LAESA.
	mknn::IndexParams *indexParams =
			mknn::PredefIndex::LinearScan_indexParams();
	//resolver parameters, search the 3 nearest objects using 4 threads
	mknn::ResolverParams *resolverParams =
			mknn::PredefIndex::LinearScan_resolverExactNearestNeighbors(3, 0, 4);
	//load the data objects
	mknn::Dataset *reference_set = mknn::DatasetLoader::ParseVectorFile(
			"reference.txt", mknn::Datatype::FLOATING_POINT_32bits);
	//build the distance
	mknn::Distance *distance = mknn::Distance::newPredefined(distanceParams,
			true);
	//build the index
	mknn::Index *index = mknn::Index::newPredefined(indexParams, true,
			reference_set, true, distance, true);
	//build the search resolver
	mknn::Resolver *resolver = index->newResolver(resolverParams, true);
	//load query objects
	mknn::Dataset *query_set = mknn::DatasetLoader::ParseVectorFile(
			"query.txt", mknn::Datatype::FLOATING_POINT_32bits);
	//perform k-NN search (this method may take a lot of time)
	mknn::Result *result = resolver->search(true, query_set, true);
	//print performance
	std::cout << result->getNumQueries() << " queries, "
			<< result->getTotalSearchTime() << " seconds, "
			<< result->getTotalDistanceEvaluations()
			<< " distance evaluations, "
			<< result->getResolver()->getParameters()->getMaxThreads()
			<< " threads." << std::endl;
	//print result by query object
	for (int i = 0; i < result->getNumQueries(); ++i) {
		mknn::ResultQuery resq = result->getResultQuery(i);
		std::cout << "query=" << i << std::endl;
		for (int j = 0; j < resq.num_nns; ++j) {
			std::cout << j + 1 << "-NN) id=" << resq.nn_position[j]
					<< " distance=" << resq.nn_distance[j] << std::endl;
		}
	}
	//releases result and in-cascade releases query_set and resolver
	delete result;
	//releases index and in-cascade releases reference_set and distance
	delete index;


The C++ API documentation can be seen here: http://www.metricknn.org/doc/metricknn_cpp/index.html


C API
=====

The same search implemented using the C API is:

	// distance parameters, euclidean distance
	MknnDistanceParams *distanceParams = mknn_predefDistance_L2();
	//index parameters, the metric index LAESA.
	MknnIndexParams *indexParams = mknn_predefIndex_LinearScan_indexParams();
	//resolver parameters, search the 3 nearest objects using 4 threads
	MknnResolverParams *resolverParams =
			mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(3, 0, 4);
	//load the data objects
	MknnDataset *reference_set = mknn_datasetLoader_ParseVectorFile(
			"reference.txt", MKNN_DATATYPE_FLOATING_POINT_32bits);
	//build the distance
	MknnDistance *distance = mknn_distance_newPredefined(distanceParams, true);
	//build the index
	MknnIndex *index = mknn_index_newPredefined(indexParams, true,
			reference_set, true, distance, true);
	//build the search resolver
	MknnResolver *resolver = mknn_index_newResolver(index, resolverParams, true);
	//load query objects
	MknnDataset *query_set = mknn_datasetLoader_ParseVectorFile("query.txt",
			MKNN_DATATYPE_FLOATING_POINT_32bits);
	//perform k-NN search (this method may take a lot of time)
	MknnResult *result = mknn_resolver_search(resolver, true, query_set, true);
	//print search performance
	printf("%i queries, %3.1lf seconds, %i distance evaluations.\n",
			(int) mknn_result_getNumQueries(result),
			mknn_result_getTotalSearchTime(result),
			(int) mknn_result_getTotalDistanceEvaluations(result));
	//print search performance
	printf("%i queries, %3.1lf seconds, %i distance evaluations.\n",
			(int) mknn_result_getNumQueries(result),
			mknn_result_getTotalSearchTime(result),
			(int) mknn_result_getTotalDistanceEvaluations(result));
	//print result by query object
	for (int i = 0; i < mknn_result_getNumQueries(result); ++i) {
		MknnResultQuery *resq = mknn_result_getResultQuery(result, i);
		printf("query=%i\n", i);
		for (int j = 0; j < resq->num_nns; ++j) {
			printf("    %i-NN) id=%i distance=%lf\n", j + 1,
					(int) resq->nn_position[j], resq->nn_distance[j]);
		}
	}
	//releases result and in-cascade releases query_set and resolver
	mknn_result_release(result);
	//releases index and in-cascade releases reference_set and distance
	mknn_index_release(index);

The C API documentation can be seen here: http://www.metricknn.org/doc/metricknn/index.html


LICENSE
=======

Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>

MetricKnn is made available under the terms of the BSD 2-Clause License.


WEBSITE
=======

For more info please visit the project's homepage: http://metricknn.org/
