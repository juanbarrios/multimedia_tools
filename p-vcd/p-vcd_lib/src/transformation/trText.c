/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Estado_TEXT {
	char *text;
};

static void tra_config_text(const char *trCode, const char *trParameters, void **out_state) {
	//MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct Estado_TEXT *es = MY_MALLOC(1, struct Estado_TEXT);
	es->text = my_newString_string(trParameters);
	//my_tokenizer_release(tk);
	//0,1 -> none
	//2 -> high
	//300 -> low
	*out_state = es;
}
static IplImage *tra_transformar_text(IplImage *imagen, int64_t numFrame, void* estado) {
	struct Estado_TEXT *es = estado;
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, CV_AA);
	int64_t i;
	for (i = 0; i < 3; ++i) {
		cvPutText(imagen, es->text, cvPoint(10 + i, 30), &font,
				cvScalar(30, 30, 150, 0));
	}
	return imagen;
}
static void tra_release_text(void* estado) {
	struct Estado_TEXT *es = estado;
	MY_FREE(es);
}
void tra_reg_text() {
	Transform_Def *def = newTransformDef("TEXT", "(text)");
	def->func_new = tra_config_text;
	def->func_transform_frame = tra_transformar_text;
	def->func_release = tra_release_text;
}
#endif
