/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

MknnDataset* getAlreadyComputedDescriptorsSample_singleFile(
		LoadDescriptors *desloader, FileDB *fdb, double sampleFractionSegments,
		double sampleFractionPerFrame) {
	struct DescriptorsFile *df = loadDescriptorsFileDB(desloader, fdb);
	MknnDataset *dataset_df = loadDescriptorsFile_getMknnDataset(df,
			sampleFractionSegments, sampleFractionPerFrame);
	MknnDataset *d = mknn_dataset_clone(dataset_df);
	mknn_dataset_release(dataset_df);
	releaseDescriptorsFile(df);
	return d;
}
static MknnDataset *combineAllDatasets(MyVectorObj *subdatasets,
		double sampleFractionGlobal) {
	MknnDataset *combined = mknn_datasetLoader_Concatenate(
			my_vectorObj_size(subdatasets),
			(MknnDataset **) my_vectorObj_array(subdatasets), true);
	if (sampleFractionGlobal <= 0) {
		MknnDataset *copied = mknn_dataset_clone(combined);
		mknn_dataset_release(combined);
		return copied;
	} else {
		MknnDataset *subset = mknn_datasetLoader_SubsetRandomSample(combined,
				sampleFractionGlobal, true);
		MknnDataset *copied = mknn_dataset_clone(subset);
		mknn_dataset_release(subset);
		return copied;
	}
}
MknnDataset *getAlreadyComputedDescriptorsSample(LoadDescriptors *desloader,
		double sampleFractionGlobal, double sampleFractionFiles,
		double sampleFractionSegments, double sampleFractionPerFrame) {
	DB *db = loadDescriptors_getDb(desloader);
	MyVectorInt *sample_list = my_random_sampleNoRepetitionsSorted(0,
			db->numFilesDb, sampleFractionFiles);
	MyProgress *lt = my_progress_new("loading descriptor files",
			my_vectorInt_size(sample_list), 1);
	MyVectorObj *subdatasets = my_vectorObj_new();
	for (int64_t i = 0; i < my_vectorInt_size(sample_list); ++i) {
		int64_t num_file = my_vectorInt_get(sample_list, i);
		MknnDataset *sub = getAlreadyComputedDescriptorsSample_singleFile(
				desloader, db->filesDb[num_file], sampleFractionSegments,
				sampleFractionPerFrame);
		my_vectorObj_add(subdatasets, sub);
		my_progress_add1(lt);
	}
	my_vectorInt_release(sample_list);
	my_progress_release(lt);
	MknnDataset* dataset = combineAllDatasets(subdatasets,
			sampleFractionGlobal);
	my_vectorObj_release(subdatasets, false);
	return dataset;
}

#ifndef NO_OPENCV
MknnDataset* computeDescriptorsSample_singleFile(LoadSegmentation *loader,
		Extractor *ex, FileDB *fdb, double sampleFractionSegments,
		double sampleFractionPerFrame) {
	const struct Segmentation *seg = loadSegmentationFileDB(loader, fdb);
	MyVectorInt *positions = my_random_sampleNoRepetitionsSorted(0,
			seg->num_segments, sampleFractionSegments);
	struct Segmentation *sampleseg = createNewSegmentation(
			my_vectorInt_size(positions), fdb->numObjsPerSecond);
	for (int64_t i = 0; i < my_vectorInt_size(positions); ++i) {
		int64_t pos = my_vectorInt_get(positions, i);
		sampleseg->segments[i] = seg->segments[pos];
	}
	my_vectorInt_release(positions);
	void **descriptors = MY_MALLOC(sampleseg->num_segments, void*);
	extractPersistentDescriptors_seg(ex, fdb, sampleseg, 0,
			sampleseg->num_segments, descriptors);
	DescriptorType td = getDescriptorType(ex);
	MknnDataset *dataset_df = descriptors2MknnDataset_samples(descriptors,
			sampleseg->num_segments, td, 0, sampleFractionPerFrame);
	MknnDataset *d = mknn_dataset_clone(dataset_df);
	mknn_dataset_release(dataset_df);
	releasePersistentDescriptors(descriptors, sampleseg->num_segments, td);
	releaseSegmentation(sampleseg);
	return d;
}
struct ParallelCompute {
	LoadSegmentation *segloader;
	Extractor **exs;
	double sampleFractionSegments, sampleFractionPerFrame;
	MyVectorInt *sample_files_list;
	MyVectorObj *subdatasets;
	pthread_mutex_t mutex;
};
static void func_incremental(int64_t current_process, void *state_object,
		int64_t current_thread) {
	struct ParallelCompute *p = state_object;
	int64_t num_file = my_vectorInt_get(p->sample_files_list, current_process);
	DB *db = loadSegmentation_getDb(p->segloader);
	MknnDataset *sub = computeDescriptorsSample_singleFile(p->segloader,
			p->exs[current_thread], db->filesDb[num_file],
			p->sampleFractionSegments, p->sampleFractionPerFrame);
	MY_MUTEX_LOCK(p->mutex);
	my_vectorObj_add(p->subdatasets, sub);
	MY_MUTEX_UNLOCK(p->mutex);
}
MknnDataset *computeDescriptorsSample(LoadSegmentation *segloader,
		Extractor **exs, int64_t numThreads, double sampleFractionGlobal,
		double sampleFractionFiles, double sampleFractionSegments,
		double sampleFractionPerFrame) {
	int64_t num_files = loadSegmentation_getDb(segloader)->numFilesDb;
	struct ParallelCompute p = { 0 };
	p.sampleFractionSegments = sampleFractionSegments;
	p.sampleFractionPerFrame = sampleFractionPerFrame;
	p.segloader = segloader;
	p.sample_files_list = my_random_sampleNoRepetitionsSorted(0, num_files,
			sampleFractionFiles);
	p.subdatasets = my_vectorObj_new();
	p.exs = exs;
	MY_MUTEX_INIT(p.mutex);
	my_parallel_incremental(my_vectorInt_size(p.sample_files_list), &p,
			func_incremental, "extract", numThreads);
	MknnDataset* dataset = combineAllDatasets(p.subdatasets,
			sampleFractionGlobal);
	my_vectorInt_release(p.sample_files_list);
	my_vectorObj_release(p.subdatasets, false);
	MY_MUTEX_DESTROY(p.mutex);
	return dataset;
}

MknnDataset *computeDescriptorsSample_nameExtractor(LoadSegmentation *segloader,
		const char *nameExtractor, int64_t numThreads,
		double sampleFractionGlobal, double sampleFractionFiles,
		double sampleFractionSegments, double sampleFractionPerFrame) {
	Extractor *exs[numThreads];
	for (int64_t i = 0; i < numThreads; ++i)
		exs[i] = getExtractor(nameExtractor);
	MknnDataset *dataset = computeDescriptorsSample(segloader, exs, numThreads,
			sampleFractionGlobal, sampleFractionFiles, sampleFractionSegments,
			sampleFractionPerFrame);
	for (int64_t i = 0; i < numThreads; ++i)
		releaseExtractor(exs[i]);
	return dataset;
}
#endif
