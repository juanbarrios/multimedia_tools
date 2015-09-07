/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

void tra_reg_acc();
void tra_reg_canny();
void tra_reg_crop();
void tra_reg_local();
void tra_reg_filtro();
void tra_reg_gris();
void tra_reg_hist();
void tra_reg_kf();
void tra_reg_laplace();
void tra_reg_match();
void tra_reg_noise();
void tra_reg_pca();
void tra_reg_pip();
void tra_reg_pipsel();
void tra_reg_resize();
void tra_reg_rot();
void tra_reg_shots();
void tra_reg_sobel();
void tra_reg_templ();
void tra_reg_text();
void tra_reg_vseg();
void tra_reg_dog();

void pvcd_register_default_transformations() {
#ifndef NO_OPENCV
	tra_reg_acc();
	tra_reg_canny();
	tra_reg_crop();
	tra_reg_local();
	tra_reg_filtro();
	tra_reg_gris();
	tra_reg_hist();
	tra_reg_kf();
	tra_reg_laplace();
	tra_reg_match();
	tra_reg_noise();
	tra_reg_pca();
	tra_reg_pip();
	tra_reg_pipsel();
	tra_reg_resize();
	tra_reg_rot();
	tra_reg_shots();
	tra_reg_sobel();
	tra_reg_templ();
	tra_reg_text();
	tra_reg_vseg();
	tra_reg_dog();
#endif
}


#ifndef NO_OPENCV
static MyVectorObj *defs_tr = NULL;
Transform_Def *newTransformDef(const char *code, const char *helpText) {
	Transform_Def *def = MY_MALLOC(1, Transform_Def);
	def->trCode = code;
	def->helpText = helpText;
	if (defs_tr == NULL)
		defs_tr = my_vectorObj_new();
	my_vectorObj_add(defs_tr, def);
	return def;
}
#endif

void pvcd_print_transformations() {
#ifndef NO_OPENCV
	my_log_info("\nAvailable Transformations:\n");
	for (int64_t i = 0; i < my_vectorObj_size(defs_tr); ++i) {
		Transform_Def *def = my_vectorObj_get(defs_tr, i);
		my_log_info("  %s%s%s\n", def->trCode,
				(def->helpText == NULL) ? "" : "_",
				(def->helpText == NULL) ? "" : def->helpText);
	}
#endif
}
#ifndef NO_OPENCV

Transform_Def *findTransformDef(const char *code) {
	if (defs_tr == NULL)
		my_log_error("no transformations defined\n");
	Transform_Def *def = NULL;
	int64_t i;
	for (i = 0; i < my_vectorObj_size(defs_tr); ++i) {
		Transform_Def *d = my_vectorObj_get(defs_tr, i);
		if (my_string_equals(d->trCode, code))
			return d;
	}
	my_log_error("unknown transformation %s\n", code);
	return def;
}
Transform *findTransform2(const char *code, const char *parameters) {
	Transform_Def *def = findTransformDef(code);
	void *state = NULL;
	def->func_new(code, parameters, &state);
	Transform *tr = MY_MALLOC(1, Transform);
	tr->def = def;
	tr->state = state;
	if (def->func_preprocess_compute != NULL)
		tr->mustPreprocessCompute = true;
	if (def->func_preprocess_load != NULL)
		tr->mustPreprocessLoad = true;
	return tr;
}
Transform *findTransform(const char *codeAndParameters) {
	MyTokenizer *tk = my_tokenizer_new(codeAndParameters, '_');
	const char *code = my_tokenizer_nextToken(tk);
	Transform *tr = findTransform2(code, my_tokenizer_getCurrentTail(tk));
	tr->codeAndParameters = my_newString_string(codeAndParameters);
	my_tokenizer_release(tk);
	return tr;
}
void releaseTransform(Transform *tr) {
	if (tr->def->func_release != NULL)
		tr->def->func_release(tr->state);
	MY_FREE_MULTI(tr->codeAndParameters, tr->preprocessSavedData, tr);
}
#endif
