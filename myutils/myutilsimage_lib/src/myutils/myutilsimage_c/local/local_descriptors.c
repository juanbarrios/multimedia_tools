/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "local_descriptors.h"
#include <metricknn/metricknn_c/metricknn_impl.h>

struct MyLocalDescriptors {
	int64_t num_descriptors;
	MyDatatype vector_datatype;
	int64_t vector_dimensions;
	size_t size_bytes_one_vector;
	size_t vectors_buffer_size;
	char *vectors_buffer;
	int64_t keypoints_buffer_size;
	struct MyLocalKeypoint *keypoints_buffer;
};

MyLocalDescriptors *my_localDescriptors_new(int64_t num_descriptors,
		MyDatatype vector_datatype, int64_t vector_dimensions) {
	MyLocalDescriptors *ldes = MY_MALLOC(1, MyLocalDescriptors);
	my_localDescriptors_redefineVectorDatatype(ldes, vector_datatype,
			vector_dimensions, num_descriptors);
	return ldes;
}
MyLocalDescriptors *my_localDescriptors_newEmpty() {
	return my_localDescriptors_new(0, MY_DATATYPE_INT8, 0);
}

MyDatatype my_localDescriptors_getVectorDatatype(MyLocalDescriptors *ldes) {
	return ldes->vector_datatype;
}
int64_t my_localDescriptors_getVectorDimensions(MyLocalDescriptors *ldes) {
	return ldes->vector_dimensions;
}
int64_t my_localDescriptors_getNumDescriptors(MyLocalDescriptors *ldes) {
	return ldes->num_descriptors;
}
void my_localDescriptors_redefineVectorDatatype(MyLocalDescriptors *ldes,
		MyDatatype vector_datatype, int64_t vector_dimensions,
		int64_t num_descriptors) {
	ldes->vector_datatype = vector_datatype;
	ldes->vector_dimensions = vector_dimensions;
	ldes->size_bytes_one_vector = my_datatype_sizeof(vector_datatype)
			* vector_dimensions;
	my_localDescriptors_redefineNumDescriptors(ldes, num_descriptors);
}
void my_localDescriptors_redefineNumDescriptors(MyLocalDescriptors *ldes,
		int64_t num_descriptors) {
	if (num_descriptors > ldes->keypoints_buffer_size) {
		MY_REALLOC(ldes->keypoints_buffer, num_descriptors,
				struct MyLocalKeypoint);
		ldes->keypoints_buffer_size = num_descriptors;
	}
	size_t new_size = num_descriptors * ldes->size_bytes_one_vector;
	if (new_size > ldes->vectors_buffer_size) {
		MY_REALLOC(ldes->vectors_buffer, new_size, char);
		ldes->vectors_buffer_size = new_size;
	}
	ldes->num_descriptors = num_descriptors;
}
static void validateRadiusAngle(double radius, double angle) {
	if (radius < 0)
		my_log_info("invalid keypoint radius (%1.1lf)\n", radius);
	if (angle < -M_PI || angle > M_PI)
		my_log_info(
				"invalid keypoint angle (%1.1lf) out of range (%1.1lf,%1.1lf) (angle is not in radians?)\n",
				angle, -M_PI, M_PI);
}
void my_localDescriptors_setKeypoint(MyLocalDescriptors *ldes,
		int64_t idDescriptor, double x, double y, double radius, double angle) {
	struct MyLocalKeypoint kp = myLocalKeypoint(x, y);
	kp.radius = radius;
	kp.angle = angle;
	validateRadiusAngle(kp.radius, kp.angle);
	ldes->keypoints_buffer[idDescriptor] = kp;
}
void my_localDescriptors_setKeypointSt(MyLocalDescriptors *ldes,
		int64_t idDescriptor, struct MyLocalKeypoint kp) {
	validateRadiusAngle(kp.radius, kp.angle);
	ldes->keypoints_buffer[idDescriptor] = kp;
}
struct MyLocalKeypoint my_localDescriptors_getKeypoint(MyLocalDescriptors *ldes,
		int64_t idDescriptor) {
	return ldes->keypoints_buffer[idDescriptor];
}
void my_localDescriptors_setVector(MyLocalDescriptors *ldes,
		int64_t idDescriptor, const void *vector) {
	char *first = ldes->vectors_buffer
			+ idDescriptor * ldes->size_bytes_one_vector;
	memcpy(first, vector, ldes->size_bytes_one_vector);
}
void *my_localDescriptors_getVector(MyLocalDescriptors *ldes,
		int64_t idDescriptor) {
	char *first = ldes->vectors_buffer
			+ idDescriptor * ldes->size_bytes_one_vector;
	return first;
}
void my_localDescriptors_scaleConvertVectors(MyLocalDescriptors *ldes,
		double scaleFactor, MyDatatype vector_new_datatype) {
	if (my_datatype_areEqual(ldes->vector_datatype, vector_new_datatype)
			&& scaleFactor == 1)
		return;
	my_function_copy_vector func_copyToDouble =
			my_datatype_getFunctionCopyVector(ldes->vector_datatype,
					MY_DATATYPE_FLOAT64);
	my_function_copy_vector func_copyFromDouble =
			my_datatype_getFunctionCopyVector(MY_DATATYPE_FLOAT64,
					vector_new_datatype);
	int64_t elements = ldes->vector_dimensions * ldes->num_descriptors;
	double *buffer = MY_MALLOC(elements, double);
	func_copyToDouble(ldes->vectors_buffer, buffer, elements);
	my_localDescriptors_redefineVectorDatatype(ldes, vector_new_datatype,
			ldes->vector_dimensions, ldes->num_descriptors);
	if (scaleFactor != 1) {
		for (int64_t i = 0; i < elements; ++i)
			buffer[i] *= scaleFactor;
	}
	func_copyFromDouble(buffer, ldes->vectors_buffer, elements);
	MY_FREE(buffer);
}
void my_localDescriptors_release(MyLocalDescriptors *ldes) {
	if (ldes == NULL)
		return;
	if (ldes->keypoints_buffer != NULL)
		free(ldes->keypoints_buffer);
	if (ldes->vectors_buffer != NULL)
		free(ldes->vectors_buffer);
	free(ldes);
}
static void validateKpPosition(double x, double y, int64_t width,
		int64_t height) {
	if (x < 0 || x > width || y < 0 || y > height) {
		my_log_info(
				"keypoint (%1.1lf, %1.1lf) out of range (%"PRIi64"x%"PRIi64") (incorrect processed size?)\n",
				x, y, width, height);
	}
}

void my_localDescriptors_rescaleKeypoints(MyLocalDescriptors *ldes,
		int64_t processed_width, int64_t processed_height, int64_t new_width,
		int64_t new_height) {
	if (processed_width == new_width && processed_height == new_height)
		return;
	double scaleInvX = new_width / ((double) processed_width);
	double scaleInvY = new_height / ((double) processed_height);
	double scaleInvAvg = (scaleInvX + scaleInvY) / 2.0;
	for (int64_t i = 0; i < ldes->num_descriptors; ++i) {
		struct MyLocalKeypoint *kp = ldes->keypoints_buffer + i;
		validateKpPosition(kp->x, kp->y, processed_width, processed_height);
		kp->x *= scaleInvX;
		kp->y *= scaleInvY;
		kp->radius *= scaleInvAvg;
		if (kp->x > new_width - 1)
			kp->x = new_width - 1;
		if (kp->y > new_height - 1)
			kp->y = new_height - 1;
		validateKpPosition(kp->x, kp->y, new_width, new_height);
	}
}
char *keypoint_to_string(struct MyLocalKeypoint *kp) {
	char *st1 = my_newString_doubleDec(kp->x, 2);
	char *st2 = my_newString_doubleDec(kp->y, 2);
	char *st3 = my_newString_doubleDec(kp->radius, 2);
	char *st4 = my_newString_doubleDec(kp->angle, 3);
	char *st5 = my_newString_format("%s\t%s\t%s\t%s", st1, st2, st3, st4);
	MY_FREE_MULTI(st1, st2, st3, st4);
	return st5;
}
char *my_localDescriptors_toString(MyLocalDescriptors *ldes) {
	my_function_to_string vectorToString = my_datatype_getFunctionToString(
			ldes->vector_datatype);
	MyStringBuffer *sb = my_stringbuf_new();
	my_stringbuf_appendInt(sb, ldes->num_descriptors);
	for (int64_t i = 0; i < ldes->num_descriptors; ++i) {
		char *st_kp = keypoint_to_string(ldes->keypoints_buffer + i);
		void *vector = my_localDescriptors_getVector(ldes, i);
		char *st_vc = vectorToString(vector, ldes->vector_dimensions, "", " ",
				"");
		char *st_l = my_newString_format("\n %s\t%s", st_kp, st_vc);
		my_stringbuf_appendString(sb, st_l);
		MY_FREE_MULTI(st_kp, st_vc, st_l);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
MknnDataset *my_localDescriptors_newDatasetVectors(MyLocalDescriptors *ldes) {
	MknnDataset *dataset = mknn_datasetLoader_PointerCompactVectors(
			ldes->vectors_buffer, false, ldes->num_descriptors,
			ldes->vector_dimensions,
			mknn_datatype_convertMy2Mknn(ldes->vector_datatype));
	return dataset;
}

MyLocalDescriptors *my_localDescriptors_clone(MyLocalDescriptors *ldes) {
	size_t size_kp = ldes->num_descriptors * sizeof(struct MyLocalKeypoint);
	size_t size_vectors = ldes->num_descriptors * ldes->vector_dimensions
			* my_datatype_sizeof(ldes->vector_datatype);
	MyLocalDescriptors *newdes = my_localDescriptors_new(ldes->num_descriptors,
			ldes->vector_datatype, ldes->vector_dimensions);
	memcpy(newdes->keypoints_buffer, ldes->keypoints_buffer, size_kp);
	memcpy(newdes->vectors_buffer, ldes->vectors_buffer, size_vectors);
	return newdes;
}
size_t my_localDescriptors_serializePredictSizeBytes(MyLocalDescriptors *ldes) {
	if (ldes->num_descriptors <= 0)
		return sizeof(int64_t);
	size_t size_header = 2 * sizeof(int64_t) + sizeof(MyDatatype);
	size_t size_kp = ldes->num_descriptors * sizeof(struct MyLocalKeypoint);
	size_t size_vectors = ldes->num_descriptors * ldes->vector_dimensions
			* my_datatype_sizeof(ldes->vector_datatype);
	return size_header + size_kp + size_vectors;
}
size_t my_localDescriptors_serialize(MyLocalDescriptors *ldes,
		void *data_buffer) {
	if (ldes->num_descriptors <= 0) {
		((int64_t*) data_buffer)[0] = ldes->num_descriptors;
		return sizeof(int64_t);
	}
	size_t size_header = 2 * sizeof(int64_t) + sizeof(MyDatatype);
	size_t size_kp = ldes->num_descriptors * sizeof(struct MyLocalKeypoint);
	size_t size_vectors = ldes->num_descriptors * ldes->vector_dimensions
			* my_datatype_sizeof(ldes->vector_datatype);
	char *start_header = data_buffer;
	char *start_kp = start_header + size_header;
	void *start_vector = start_header + size_header + size_kp;
	((int64_t*) start_header)[0] = ldes->num_descriptors;
	((int64_t*) start_header)[1] = ldes->vector_dimensions;
	((MyDatatype*) (start_header + 2 * sizeof(int64_t)))[0] =
			ldes->vector_datatype;
	memcpy(start_kp, ldes->keypoints_buffer, size_kp);
	memcpy(start_vector, ldes->vectors_buffer, size_vectors);
	return size_header + size_kp + size_vectors;
}
size_t my_localDescriptors_deserializePredictReadBytes(void *data_buffer) {
	if (data_buffer == NULL)
		return 0;
	int64_t num_descriptors = ((int64_t*) data_buffer)[0];
	if (num_descriptors == 0)
		return sizeof(int64_t);
	size_t size_header = 2 * sizeof(int64_t) + sizeof(MyDatatype);
	char *start_header = data_buffer;
	int64_t vector_dimensions = ((int64_t*) start_header)[1];
	MyDatatype vector_datatype = ((MyDatatype*) (start_header
			+ 2 * sizeof(int64_t)))[0];
	size_t size_kp = num_descriptors * sizeof(struct MyLocalKeypoint);
	size_t size_vectors = num_descriptors * vector_dimensions
			* my_datatype_sizeof(vector_datatype);
	return size_header + size_kp + size_vectors;
}
size_t my_localDescriptors_deserialize(void *data_buffer,
		MyLocalDescriptors *ldes) {
	if (data_buffer == NULL)
		return 0;
	int64_t num_descriptors = ((int64_t*) data_buffer)[0];
	if (num_descriptors == 0)
		return sizeof(int64_t);
	size_t size_header = 2 * sizeof(int64_t) + sizeof(MyDatatype);
	char *start_header = data_buffer;
	int64_t vector_dimensions = ((int64_t*) start_header)[1];
	MyDatatype vector_datatype = ((MyDatatype*) (start_header
			+ 2 * sizeof(int64_t)))[0];
	size_t size_kp = num_descriptors * sizeof(struct MyLocalKeypoint);
	size_t size_vectors = num_descriptors * vector_dimensions
			* my_datatype_sizeof(vector_datatype);
	my_localDescriptors_redefineVectorDatatype(ldes, vector_datatype,
			vector_dimensions, num_descriptors);
	char *start_kp = start_header + size_header;
	void *start_vector = start_header + size_header + size_kp;
	memcpy(ldes->keypoints_buffer, start_kp, size_kp);
	memcpy(ldes->vectors_buffer, start_vector, size_vectors);
	return size_header + size_kp + size_vectors;
}

static int64_t my_localDescriptors_getNumObjects(void *data_pointer) {
	MyLocalDescriptors *local_descriptors = data_pointer;
	return my_localDescriptors_getNumDescriptors(local_descriptors);
}
static void *my_localDescriptors_getObject(void *data_pointer, int64_t pos) {
	MyLocalDescriptors *local_descriptors = data_pointer;
	return my_localDescriptors_getVector(local_descriptors, pos);
}
static void my_localDescriptors_pushObject(void *data_pointer, void *object) {
	MyLocalDescriptors *local_descriptors = data_pointer;
	int64_t size = my_localDescriptors_getNumDescriptors(local_descriptors);
	my_localDescriptors_redefineNumDescriptors(local_descriptors, size + 1);
	my_localDescriptors_setVector(local_descriptors, size, object);
}
static void my_localDescriptors_releaseDataPointer(void *data_pointer) {
	MyLocalDescriptors *local_descriptors = data_pointer;
	my_localDescriptors_release(local_descriptors);
}
MknnDataset *my_localDescriptors_createMknnDataset_vectors(
		MyLocalDescriptors *local_descriptors,
		bool free_descriptors_on_dataset_release) {
	int64_t dims = my_localDescriptors_getVectorDimensions(local_descriptors);
	MyDatatype mtype = my_localDescriptors_getVectorDatatype(local_descriptors);
	MknnDomain *dom = mknn_domain_newVector(dims,
			mknn_datatype_convertMy2Mknn(mtype));
	MknnDataset *dataset = mknn_datasetLoader_Custom(local_descriptors,
			my_localDescriptors_getNumObjects, my_localDescriptors_getObject,
			my_localDescriptors_pushObject,
			free_descriptors_on_dataset_release ?
					my_localDescriptors_releaseDataPointer : NULL, dom, true);
	return dataset;
}
MknnDataset *my_localDescriptors_createConcatenateMknnDataset_vectors(
		int64_t num_local_descriptors, MyLocalDescriptors **local_descriptors,
		bool free_all_descriptors_on_dataset_release) {
	MknnDataset *subsets[num_local_descriptors];
	for (int64_t i = 0; i < num_local_descriptors; ++i) {
		subsets[i] = my_localDescriptors_createMknnDataset_vectors(
				local_descriptors[i], free_all_descriptors_on_dataset_release);
	}
	MknnDataset *dataset = mknn_datasetLoader_Concatenate(num_local_descriptors,
			subsets, true);
	return dataset;
}
MknnDataset *my_localDescriptors_createMknnDataset_positions(
		MyLocalDescriptors *local_descriptors,
		bool free_descriptors_on_dataset_release) {
	float *positions = MY_MALLOC_NOINIT(local_descriptors->num_descriptors,
			float);
	for (int64_t i = 0; i < local_descriptors->num_descriptors; ++i) {
		positions[2 * i] = local_descriptors->keypoints_buffer[i].x;
		positions[2 * i + 1] = local_descriptors->keypoints_buffer[i].y;
	}
	return mknn_datasetLoader_PointerCompactVectors(positions, true,
			local_descriptors->num_descriptors, 2,
			MKNN_DATATYPE_FLOATING_POINT_32bits);
}
MknnDataset *my_localDescriptors_createConcatenateMknnDataset_positions(
		int64_t num_local_descriptors, MyLocalDescriptors **local_descriptors,
		bool free_all_descriptors_on_dataset_release) {
	MknnDataset *subsets[num_local_descriptors];
	for (int64_t i = 0; i < num_local_descriptors; ++i) {
		subsets[i] = my_localDescriptors_createMknnDataset_positions(
				local_descriptors[i], free_all_descriptors_on_dataset_release);
	}
	MknnDataset *dataset = mknn_datasetLoader_Concatenate(num_local_descriptors,
			subsets, true);
	return dataset;
}
