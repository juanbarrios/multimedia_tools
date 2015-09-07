/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PCA_H_
#define MKNN_PCA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

typedef struct MknnPcaAlgorithm MknnPcaAlgorithm;

typedef void (*mknn_pca_func_transformVector)(MknnPcaAlgorithm *pca,
		void *vector_src, void *vector_dst, int64_t dim_vector_dst);

MknnPcaAlgorithm *mknn_pca_new();

void mknn_pca_addDatasetToVectorStats(MknnPcaAlgorithm *pca, MknnDataset *dataset);

void mknn_pca_computeTransformationMatrix(MknnPcaAlgorithm *pca);

void mknn_pca_save(MknnPcaAlgorithm *pca, const char *filename_write,
bool include_debug);

void mknn_pca_restore(MknnPcaAlgorithm *pca, const char *filename_read);

int64_t mknn_pca_getInputDimension(MknnPcaAlgorithm *pca);

mknn_pca_func_transformVector mknn_pca_getTransformVectorFunction(
		MknnDatatype dtype_vector_src, MknnDatatype dtype_vector_dst);

void mknn_pca_transform_dataset(MknnPcaAlgorithm *pca, MknnDataset *dataset_in,
		MknnDataset *dataset_out);

void mknn_pca_release(MknnPcaAlgorithm *pca);

#ifdef __cplusplus
}
#endif

#endif
