/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "seg.h"

void seg_reg_cte();
void seg_reg_fileshots();
void seg_reg_sample();

void pvcd_register_default_segmentators() {
	seg_reg_cte();
	seg_reg_fileshots();
	seg_reg_sample();
}

static MyVectorObj *defs_seg = NULL;

static MyVectorObj *pvcd_get_segmentators() {
	if (defs_seg == NULL)
		defs_seg = my_vectorObj_new();
	return defs_seg;
}
void print_segmentators() {
	MyVectorObj *defs = pvcd_get_segmentators();
	my_log_info("\nAvailable Segmentators:\n");
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		Segmentator_Def *def = my_vectorObj_get(defs, i);
		my_log_info("  %s%s%s\n", def->segCode,
				(def->helpText == NULL) ? "" : "_",
				(def->helpText == NULL) ? "" : def->helpText);
	}
}

void addSegmentatorDef(const char *code, const char *helpText,
		segmentator_func_new func_new, segmentator_func_segment func_segment,
		segmentator_func_release func_release) {
	Segmentator_Def *def = MY_MALLOC(1, Segmentator_Def);
	def->segCode = code;
	def->helpText = helpText;
	def->func_new = func_new;
	def->func_segment = func_segment;
	def->func_release = func_release;
	MyVectorObj *defs = pvcd_get_segmentators();
	my_vectorObj_add(defs, def);
}
Segmentator_Def *findSegmentatorDef(const char *code, bool fail) {
	MyVectorObj *defs = pvcd_get_segmentators();
	Segmentator_Def *def = NULL;
	for (int64_t i = 0; i < my_vectorObj_size(defs); ++i) {
		Segmentator_Def *d = my_vectorObj_get(defs, i);
		if (my_string_equals(d->segCode, code)) {
			if (def != NULL)
				my_log_error("not unique %s\n", code);
			def = d;
		}
	}
	if (def == NULL && fail)
		my_log_error("segmentator unknown %s\n", code);
	return def;
}
Segmentator *findSegmentator2(const char *code, const char *parameters) {
	Segmentator_Def *def = findSegmentatorDef(code, 1);
	void *state = NULL;
	def->func_new(code, parameters, &state);
	Segmentator *seg = MY_MALLOC(1, Segmentator);
	seg->def = def;
	seg->state = state;
	return seg;
}
Segmentator *findSegmentator(const char *codeAndParameters) {
	MyTokenizer *tk = my_tokenizer_new(codeAndParameters, '_');
	const char *code = my_tokenizer_nextToken(tk);
	Segmentator *sp = findSegmentator2(code, my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	return sp;
}
struct Segmentation *segmentVideoFile(Segmentator *sp, FileDB *fdb) {
	struct Segmentation *seg = sp->def->func_segment(fdb, sp->state);
	validateSegmentation(seg);
	return seg;
}
void releaseSegmentator(Segmentator *sp) {
	sp->def->func_release(sp->state);
	MY_FREE(sp);
}
