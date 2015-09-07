/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Estado_NOISE {
	double prob_black, prob_white;
};

void tra_config_noise(const char *trCode, const char *trParameters, void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	struct Estado_NOISE *es = MY_MALLOC(1, struct Estado_NOISE);
	es->prob_black = my_tokenizer_nextFraction(tk);
	es->prob_white = my_tokenizer_nextFraction(tk);
	my_tokenizer_release(tk);
	*out_state = es;
}
IplImage *tra_transformar_noise(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Estado_NOISE *es = estado;
	int64_t x, y, z;
	for (y = 0; y < imagen->height; ++y) {
		uchar *ptr = (uchar*) (imagen->imageData + imagen->widthStep * y);
		for (x = 0; x < imagen->width; ++x) {
			if (es->prob_black > 0 && my_random_double(0, 1) < es->prob_black) {
				for (z = 0; z < imagen->nChannels; ++z)
					ptr[imagen->nChannels * x + z] = 0;
			} else if (es->prob_white > 0
					&& my_random_double(0, 1) < es->prob_white) {
				for (z = 0; z < imagen->nChannels; ++z)
					ptr[imagen->nChannels * x + z] = 255;
			}
		}
	}
	return imagen;
}
void tra_release_noise(void* estado) {
	struct Estado_NOISE *es = estado;
	MY_FREE(es);
}
void tra_reg_noise() {
	Transform_Def *def = newTransformDef("NOISE", "probBlack_probWhite");
	def->func_new = tra_config_noise;
	def->func_transform_frame = tra_transformar_noise;
	def->func_release = tra_release_noise;
}
#endif
