/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef METRICKNN_C_H_
#define METRICKNN_C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Main header to be included.
 * @file
 */

/**
 * Represents a broad type of objects, like strings, vectors, etc.
 */
typedef struct {
	int8_t mknn_domain_code;
} MknnGeneralDomain;

/**
 * Represents a constant for a datatype, like integer 8bits, float 32 bits, etc.
 */
typedef struct {
	int8_t mknn_datatype_code;
} MknnDatatype;

/**
 * Represents a type of object, as strings, vectors (with a given dimensionality), etc.
 *
 * See documentation for @ref mknn_domain.h and  @ref mknn_datatype.h.
 */
typedef struct MknnDomain MknnDomain;

/**
 * Represents a set of objects.
 * In order to use some pre-defined distance the dataset must select some domain.
 *
 * See documentation for @ref mknn_dataset.h.
 */
typedef struct MknnDataset MknnDataset;

/**
 * Represents the definition of a distance.
 * Every pre-defined distance supports one domain.
 *
 * See documentation for @ref mknn_distance.h.
 */
typedef struct MknnDistance MknnDistance;

/**
 * Represents the instance of a distance.
 * It evaluates the distance between any two objects of the given domain.
 * Each instance does not support multi-threading, so in order to resolve queries in
 * parallel the MknnDistance must create one instance per thread.
 *
 * See documentation for @ref mknn_distance.h.
 */
typedef struct MknnDistanceEval MknnDistanceEval;

/**
 * Represents the index structure.
 * It needs at least a dataset and a distance.
 *
 * See documentation for @ref mknn_index.h.
 */
typedef struct MknnIndex MknnIndex;

/**
 * Represents the object that performs the similarity search.
 * It needs a query dataset, the index and search parameters.
 *
 * See documentation for @ref mknn_resolver.h.
 */
typedef struct MknnResolver MknnResolver;

/**
 * It contains the result of a similarity search of the query dataset.
 * It summarizes the resources needed to resolve the search.
 * It is a non-opaque structure, thus its fields can be directly accessed.
 *
 * See documentation for @ref mknn_resolver.h.
 */
typedef struct MknnResult MknnResult;

/**
 * It contains the result of a single similarity search for a single query object.
 * It contains the k-NN and distances.
 * It is a non-opaque structure, thus its fields can be directly accessed.
 *
 * See documentation for @ref mknn_resolver.h.
 */
typedef struct MknnResultQuery MknnResultQuery;

/**
 * Stores the parameters for building a distance.
 *
 * See documentation for @ref mknn_params.h.
 */
typedef struct MknnDistanceParams MknnDistanceParams;

/**
 * Stores the parameters for building an index.
 *
 * See documentation for @ref mknn_params.h.
 */
typedef struct MknnIndexParams MknnIndexParams;

/**
 * Stores the parameters for building a search resolver.
 *
 * See documentation for @ref mknn_params.h.
 */
typedef struct MknnResolverParams MknnResolverParams;

/**
 * It converts an object from a given domain to its string representation.
 */
typedef struct MknnPrinter MknnPrinter;

/**
 * It parses an object from its string representation.
 */
typedef struct MknnParser MknnParser;

#include "metricknn_c/mknn_general_domain.h"
#include "metricknn_c/mknn_datatype.h"
#include "metricknn_c/mknn_domain.h"
#include "metricknn_c/mknn_params.h"
#include "metricknn_c/mknn_dataset.h"
#include "metricknn_c/mknn_dataset_loader.h"
#include "metricknn_c/mknn_distance.h"
#include "metricknn_c/mknn_distance_eval.h"
#include "metricknn_c/mknn_index.h"
#include "metricknn_c/mknn_resolver.h"
#include "metricknn_c/mknn_result.h"
#include "metricknn_c/mknn_printer.h"
#include "metricknn_c/mknn_predefined_index.h"
#include "metricknn_c/mknn_predefined_distance.h"
#include "metricknn_c/mknn_kmeans.h"
#include "metricknn_c/mknn_pca.h"

#ifdef __cplusplus
}
#endif

#endif
