/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

MknnDataset *descriptors2MknnDataset(void **descriptors,
		int64_t num_descriptors, DescriptorType td) {
	if (td.dtype == DTYPE_LOCAL_VECTORS) {
		MknnDataset *subsets[num_descriptors];
		for (int64_t k = 0; k < num_descriptors; ++k) {
			MyLocalDescriptors *ldes = descriptors[k];
			subsets[k] = my_localDescriptors_createMknnDataset_vectors(ldes,
			false);
		}
		return mknn_datasetLoader_Concatenate(num_descriptors, subsets, true);
	} else {
		MknnDomain *domain = getMknnDomainDescriptorArray(td);
		return mknn_datasetLoader_PointerArray(descriptors, num_descriptors,
				domain, false, false, true);
	}
}
MknnDataset *descriptors2MknnDataset_positions(void **descriptors,
		MyVectorInt *positions, DescriptorType td,
		double sampleFraction_localsPerFrame) {
	int64_t num_positions = my_vectorInt_size(positions);
	if (td.dtype == DTYPE_LOCAL_VECTORS) {
		MknnDataset *subsets[num_positions];
		for (int64_t k = 0; k < num_positions; ++k) {
			int64_t pos = my_vectorInt_get(positions, k);
			MyLocalDescriptors *ldes = descriptors[pos];
			MknnDataset *sub = my_localDescriptors_createMknnDataset_vectors(
					ldes, false);
			if (sampleFraction_localsPerFrame <= 0)
				subsets[k] = sub;
			else
				subsets[k] = mknn_datasetLoader_SubsetRandomSample(sub,
						sampleFraction_localsPerFrame, true);
		}
		return mknn_datasetLoader_Concatenate(num_positions, subsets,
		true);
	} else {
		MknnDomain *domain = getMknnDomainDescriptorArray(td);
		int64_t maxpos = my_vectorInt_max(positions);
		MknnDataset *dataset = mknn_datasetLoader_PointerArray(descriptors,
				maxpos + 1, domain, false, false, true);
		return mknn_datasetLoader_SubsetPositions(dataset,
				my_vectorInt_array(positions), num_positions, true);
	}
}
MknnDataset *descriptors2MknnDataset_samples(void **descriptors,
		int64_t num_descriptors, DescriptorType td,
		double sampleFractionDescriptors, double sampleFraction_localsPerFrame) {
	MknnDataset *dataset_df;
	if (td.dtype == DTYPE_LOCAL_VECTORS) {
		MknnDataset *subsets[num_descriptors];
		for (int64_t k = 0; k < num_descriptors; ++k) {
			MyLocalDescriptors *ldes = descriptors[k];
			MknnDataset *sub = my_localDescriptors_createMknnDataset_vectors(
					ldes, false);
			if (sampleFraction_localsPerFrame <= 0)
				subsets[k] = sub;
			else
				subsets[k] = mknn_datasetLoader_SubsetRandomSample(sub,
						sampleFraction_localsPerFrame, true);
		}
		dataset_df = mknn_datasetLoader_Concatenate(num_descriptors, subsets,
		true);
	} else {
		MknnDomain *domain = getMknnDomainDescriptorArray(td);
		dataset_df = mknn_datasetLoader_PointerArray(descriptors,
				num_descriptors, domain, false, false, true);
	}
	if (sampleFractionDescriptors <= 0)
		return dataset_df;
	else
		return mknn_datasetLoader_SubsetRandomSample(dataset_df,
				sampleFractionDescriptors, true);
}
