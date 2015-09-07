/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "heap.h"

struct MknnHeapNode {
	double distance;
	int64_t object_id;
};

static void maxHeap_appendToTail(struct MknnHeapNode *data_array,
		struct MknnHeapNode data, int64_t *length_ptr) {
	int64_t pos = *length_ptr;
	while (pos > 0) {
		int64_t parent_pos = (pos - 1) / 2;
		if (data.distance <= data_array[parent_pos].distance)
			break;
		data_array[pos] = data_array[parent_pos];
		pos = parent_pos;
	}
	data_array[pos] = data;
	(*length_ptr)++;
}
static void minHeap_appendToTail(struct MknnHeapNode *data_array,
		struct MknnHeapNode data, int64_t *length_ptr) {
	int64_t pos = *length_ptr;
	while (pos > 0) {
		int64_t parent_pos = (pos - 1) / 2;
		if (data.distance >= data_array[parent_pos].distance)
			break;
		data_array[pos] = data_array[parent_pos];
		pos = parent_pos;
	}
	data_array[pos] = data;
	(*length_ptr)++;
}
static void maxHeap_replaceHead(struct MknnHeapNode *data_array,
		struct MknnHeapNode data, int64_t length) {
	int64_t pos = 0;
	for (;;) {
		int64_t left = 2 * pos + 1;
		int64_t right = left + 1;
		if (left < length && right < length) {
			if (data_array[left].distance > data.distance
					&& data_array[left].distance
							>= data_array[right].distance) {
				data_array[pos] = data_array[left];
				pos = left;
				continue;
			} else if (data_array[right].distance > data.distance
					&& data_array[right].distance
							>= data_array[left].distance) {
				data_array[pos] = data_array[right];
				pos = right;
				continue;
			}
		} else if (left < length && data_array[left].distance > data.distance) {
			data_array[pos] = data_array[left];
			pos = left;
			continue;
		}
		break;
	}
	data_array[pos] = data;
}
static void minHeap_replaceHead(struct MknnHeapNode *data_array,
		struct MknnHeapNode data, int64_t length) {
	int64_t pos = 0;
	for (;;) {
		int64_t left = 2 * pos + 1;
		int64_t right = left + 1;
		if (left < length && right < length) {
			if (data_array[left].distance < data.distance
					&& data_array[left].distance
							<= data_array[right].distance) {
				data_array[pos] = data_array[left];
				pos = left;
				continue;
			} else if (data_array[right].distance < data.distance
					&& data_array[right].distance
							<= data_array[left].distance) {
				data_array[pos] = data_array[right];
				pos = right;
				continue;
			}
		} else if (left < length && data_array[left].distance < data.distance) {
			data_array[pos] = data_array[left];
			pos = left;
			continue;
		}
		break;
	}
	data_array[pos] = data;
}

typedef void (*mknn_func_replaceHead)(struct MknnHeapNode *data_array,
		struct MknnHeapNode data, int64_t length);
typedef void (*mknn_func_storeBestDistances)(struct MknnHeapNode data,
		MknnHeap *heap, double *out_current_threshold);

struct MknnHeap {
	int64_t current_length, max_length;
	bool isSorted;
	struct MknnHeapNode *data_array;
	mknn_func_replaceHead func_replaceHead;
	mknn_func_storeBestDistances func_storeBestDistances;
};

static void maxHeap_storeBestDistances(struct MknnHeapNode data, MknnHeap *heap,
		double *out_current_threshold) {
	if (data.distance > *out_current_threshold) {
		return;
	} else if (heap->current_length < heap->max_length) {
		maxHeap_appendToTail(heap->data_array, data, &heap->current_length);
		if (heap->current_length == heap->max_length
				&& heap->data_array[0].distance < *out_current_threshold)
			*out_current_threshold = heap->data_array[0].distance;
	} else if (data.distance < heap->data_array[0].distance) {
		maxHeap_replaceHead(heap->data_array, data, heap->current_length);
		*out_current_threshold = heap->data_array[0].distance;
	}
}
static void minHeap_storeBestDistances(struct MknnHeapNode data, MknnHeap *heap,
		double *out_current_threshold) {
	if (data.distance < *out_current_threshold) {
		return;
	} else if (heap->current_length < heap->max_length) {
		minHeap_appendToTail(heap->data_array, data, &heap->current_length);
		if (heap->current_length == heap->max_length
				&& heap->data_array[0].distance > *out_current_threshold)
			*out_current_threshold = heap->data_array[0].distance;
	} else if (data.distance > heap->data_array[0].distance) {
		minHeap_replaceHead(heap->data_array, data, heap->current_length);
		*out_current_threshold = heap->data_array[0].distance;
	}
}

//the MaxHeap is used to locate the lowest values (i.e., the k-NN).
MknnHeap *mknn_heap_newMaxHeap(int64_t heap_size) {
	MknnHeap *heap = MY_MALLOC(1, MknnHeap);
	heap->max_length = heap_size;
	heap->data_array = MY_MALLOC_NOINIT(heap_size, struct MknnHeapNode);
	heap->func_replaceHead = maxHeap_replaceHead;
	heap->func_storeBestDistances = maxHeap_storeBestDistances;
	return heap;
}
MknnHeap *mknn_heap_newMinHeap(int64_t heap_size) {
	MknnHeap *heap = MY_MALLOC(1, MknnHeap);
	heap->max_length = heap_size;
	heap->data_array = MY_MALLOC_NOINIT(heap_size, struct MknnHeapNode);
	heap->func_replaceHead = minHeap_replaceHead;
	heap->func_storeBestDistances = minHeap_storeBestDistances;
	return heap;
}
void mknn_heap_storeBestDistances(double distance, int64_t object_id,
		MknnHeap *heap, double *current_threshold_ptr) {
	struct MknnHeapNode data = { .distance = distance, .object_id = object_id };
	heap->func_storeBestDistances(data, heap, current_threshold_ptr);
}
int64_t mknn_heap_getSize(MknnHeap *heap) {
	return heap->current_length;
}
void mknn_heap_sortElements(MknnHeap *heap) {
	if (heap->isSorted)
		return;
	struct MknnHeapNode *data_array = heap->data_array;
	int64_t length = heap->current_length;
	while (length > 0) {
		struct MknnHeapNode data = data_array[length - 1];
		data_array[length - 1] = data_array[0];
		length--;
		heap->func_replaceHead(heap->data_array, data, length);
	}
	heap->isSorted = true;
}
double mknn_heap_getDistanceAtPosition(MknnHeap *heap, int64_t position) {
	return heap->data_array[position].distance;
}
int64_t mknn_heap_getObjectIdAtPosition(MknnHeap *heap, int64_t position) {
	return heap->data_array[position].object_id;
}

void mknn_heap_reset(MknnHeap *heap) {
	heap->current_length = 0;
	heap->isSorted = false;
}
void mknn_heap_release(MknnHeap *heap) {
	MY_FREE(heap->data_array);
	MY_FREE(heap);
}
MknnHeap **mknn_heap_newMultiMaxHeap(int64_t heap_size, int64_t num_heaps) {
	MknnHeap **heaps = MY_MALLOC(num_heaps, MknnHeap *);
	for (int64_t i = 0; i < num_heaps; ++i) {
		heaps[i] = mknn_heap_newMaxHeap(heap_size);
	}
	return heaps;
}
MknnHeap **mknn_heap_newMultiMinHeap(int64_t heap_size, int64_t num_heaps) {
	MknnHeap **heaps = MY_MALLOC(num_heaps, MknnHeap *);
	for (int64_t i = 0; i < num_heaps; ++i) {
		heaps[i] = mknn_heap_newMinHeap(heap_size);
	}
	return heaps;
}
void mknn_heap_releaseMulti(MknnHeap **heaps, int64_t num_heaps) {
	for (int64_t i = 0; i < num_heaps; ++i) {
		mknn_heap_release(heaps[i]);
	}
	MY_FREE(heaps);
}
