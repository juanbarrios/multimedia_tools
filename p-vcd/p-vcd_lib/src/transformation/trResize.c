/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct TR_Resize {
	MyImageResizer *resizer;
};

static void tra_config_resize(const char *trCode, const char *trParameters,
		void **out_state) {
	*out_state = my_imageResizer_newTextOptions(trParameters);
}
static IplImage *tra_transformar_resize(IplImage *imagen, int64_t numFrame,
		void* estado) {
	MyImageResizer *resizer = estado;
	return my_imageResizer_resizeImage(imagen, resizer);
}
static void tra_release_resize(void *estado) {
	MyImageResizer *resizer = estado;
	my_imageResizer_release(resizer);
}
void tra_reg_resize() {
	Transform_Def *def = newTransformDef("RESIZE", my_imageResizer_getTextOptions());
	def->func_new = tra_config_resize;
	def->func_transform_frame = tra_transformar_resize;
	def->func_release = tra_release_resize;
}
#endif
