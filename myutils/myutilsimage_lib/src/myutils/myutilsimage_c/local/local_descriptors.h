/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_LOCAL_LOCAL_DESCRIPTORS_H
#define MYUTILSIMAGE_LOCAL_LOCAL_DESCRIPTORS_H

#include "local.h"

MyLocalDescriptors *my_localDescriptors_new(int64_t num_descriptors,
		MyDatatype vector_datatype, int64_t vector_dimensions);
MyLocalDescriptors *my_localDescriptors_newEmpty();
MyDatatype my_localDescriptors_getVectorDatatype(MyLocalDescriptors *ldes);
int64_t my_localDescriptors_getVectorDimensions(MyLocalDescriptors *ldes);
int64_t my_localDescriptors_getNumDescriptors(MyLocalDescriptors *ldes);
void my_localDescriptors_redefineVectorDatatype(MyLocalDescriptors *ldes,
		MyDatatype vector_datatype, int64_t vector_dimensions,
		int64_t num_descriptors);
void my_localDescriptors_redefineNumDescriptors(MyLocalDescriptors *ldes,
		int64_t num_descriptors);
void my_localDescriptors_setKeypoint(MyLocalDescriptors *ldes,
		int64_t idDescriptor, double x, double y, double radius, double angle);
void my_localDescriptors_setKeypointSt(MyLocalDescriptors *ldes,
		int64_t idDescriptor, struct MyLocalKeypoint kp);
struct MyLocalKeypoint my_localDescriptors_getKeypoint(MyLocalDescriptors *ldes,
		int64_t idDescriptor);
void my_localDescriptors_setVector(MyLocalDescriptors *ldes,
		int64_t idDescriptor, const void *vector);
void *my_localDescriptors_getVector(MyLocalDescriptors *ldes,
		int64_t idDescriptor);
void my_localDescriptors_scaleConvertVectors(MyLocalDescriptors *ldes,
		double scaleFactor, MyDatatype vector_new_datatype);
void my_localDescriptors_release(MyLocalDescriptors *ldes);

void my_localDescriptors_rescaleKeypoints(MyLocalDescriptors *ldes,
		int64_t processed_width, int64_t processed_height, int64_t new_width,
		int64_t new_height);
char *my_localDescriptors_toString(MyLocalDescriptors *ldes);

MyLocalDescriptors *my_localDescriptors_clone(MyLocalDescriptors *ldes);

size_t my_localDescriptors_serializePredictSizeBytes(MyLocalDescriptors *ldes);
size_t my_localDescriptors_serialize(MyLocalDescriptors *ldes,
		void *data_buffer);
size_t my_localDescriptors_deserializePredictReadBytes(void *data_buffer);
size_t my_localDescriptors_deserialize(void *data_buffer,
		MyLocalDescriptors *ldes);

MknnDataset *my_localDescriptors_createMknnDataset_vectors(
		MyLocalDescriptors *local_descriptors,
		bool free_descriptors_on_dataset_release);
MknnDataset *my_localDescriptors_createConcatenateMknnDataset_vectors(
		int64_t num_local_descriptors, MyLocalDescriptors **local_descriptors,
		bool free_all_descriptors_on_dataset_release);
MknnDataset *my_localDescriptors_createMknnDataset_positions(
		MyLocalDescriptors *local_descriptors,
		bool free_descriptors_on_dataset_release);
MknnDataset *my_localDescriptors_createConcatenateMknnDataset_positions(
		int64_t num_local_descriptors, MyLocalDescriptors **local_descriptors,
		bool free_all_descriptors_on_dataset_release);

#endif
