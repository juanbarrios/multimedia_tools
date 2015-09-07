/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef SEG_H
#define SEG_H

#include "../pvcd.h"

typedef void (*segmentator_func_new)(const char *segCode,
		const char *segParameters, void **out_state);
typedef struct Segmentation* (*segmentator_func_segment)(FileDB *fdb, void* state);
typedef void (*segmentator_func_release)(void* state);

typedef struct {
	const char *segCode, *helpText;
	segmentator_func_new func_new;
	segmentator_func_segment func_segment;
	segmentator_func_release func_release;
} Segmentator_Def;

typedef struct {
	Segmentator_Def *def;
	void *state;
} Segmentator;

void pvcd_register_default_segmentators();

void print_segmentators();

void addSegmentatorDef(const char *code, const char *helpText,
		segmentator_func_new func_new, segmentator_func_segment func_segment,
		segmentator_func_release func_release);

Segmentator_Def *findSegmentatorDef(const char *segCode, bool fail);
Segmentator *findSegmentator2(const char *code, const char *parameters);
Segmentator *findSegmentator(const char *codeAndParameters);
struct Segmentation *segmentVideoFile(Segmentator *sp, FileDB *fdb);
void releaseSegmentator(Segmentator *sp);

#endif
