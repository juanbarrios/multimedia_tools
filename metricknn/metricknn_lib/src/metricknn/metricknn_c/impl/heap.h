/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_HEAP_H
#define MKNN_HEAP_H

#include "../metricknn_impl.h"

MknnHeap *mknn_heap_newMaxHeap(int64_t heap_size);
MknnHeap *mknn_heap_newMinHeap(int64_t heap_size);
void mknn_heap_storeBestDistances(double distance, int64_t object_id,
		MknnHeap *heap, double *current_threshold_ptr);
int64_t mknn_heap_getSize(MknnHeap *heap);
void mknn_heap_sortElements(MknnHeap *heap);
double mknn_heap_getDistanceAtPosition(MknnHeap *heap, int64_t position);
int64_t mknn_heap_getObjectIdAtPosition(MknnHeap *heap, int64_t position);
void mknn_heap_reset(MknnHeap *heap);
void mknn_heap_release(MknnHeap *heap);

MknnHeap **mknn_heap_newMultiMaxHeap(int64_t heap_size, int64_t num_heaps);
MknnHeap **mknn_heap_newMultiMinHeap(int64_t heap_size, int64_t num_heaps);
void mknn_heap_releaseMulti(MknnHeap **heaps, int64_t num_heaps);

#endif
