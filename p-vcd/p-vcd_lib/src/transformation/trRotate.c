/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct TR_Rotate {
	IplImage *img;
	uchar vflip, hflip;
	int64_t degree;
};

static void tra_config_rot(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct TR_Rotate *es = MY_MALLOC(1, struct TR_Rotate);
	es->vflip = my_tokenizer_isNext(tk, "VFLIP") ? 1 : 0;
	es->hflip = my_tokenizer_isNext(tk, "HFLIP") ? 1 : 0;
	if (my_tokenizer_hasNext(tk))
		es->degree = my_tokenizer_nextInt(tk);
	my_tokenizer_release(tk);
	*out_state = es;
}
static IplImage *tra_transformar_rot(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct TR_Rotate *es = estado;
	if ((es->degree == 90 || es->degree == -90 || es->degree == 270
			|| es->degree == -270) && es->img == NULL)
		es->img = cvCreateImage(cvSize(imagen->height, imagen->width),
				imagen->depth, imagen->nChannels);
	if (es->vflip && es->hflip)
		cvFlip(imagen, NULL, -1);
	else if (es->vflip)
		cvFlip(imagen, NULL, 1);
	else if (es->hflip)
		cvFlip(imagen, NULL, 0);
	if (es->degree == -90 || es->degree == 270) {
		cvTranspose(imagen, es->img);
		return es->img;
	} else if (es->degree == 90 || es->degree == -270) {
		cvFlip(imagen, NULL, -1);
		cvTranspose(imagen, es->img);
		return es->img;
	} else if (es->degree == 180) {
		cvFlip(imagen, NULL, -1);
		return imagen;
	} else {
		return imagen;
	}
}
static void tra_release_rot(void *estado) {
	struct TR_Rotate *es = estado;
	my_image_release(es->img);
	MY_FREE(es);
}
void tra_reg_rot() {
	Transform_Def *def = newTransformDef("ROTATE", "[VFLIP|HFLIP|degree]");
	def->func_new = tra_config_rot;
	def->func_transform_frame = tra_transformar_rot;
	def->func_release = tra_release_rot;
}
#endif
