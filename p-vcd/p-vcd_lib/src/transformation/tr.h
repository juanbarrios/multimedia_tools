/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef TR_H
#define TR_H

#include "../pvcd.h"

void pvcd_register_default_transformations();
void pvcd_print_transformations();

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

typedef void (*tranform_func_new)(const char *trCode, const char *trParameters,
		void **out_state);

//performs prior computation on the whole video
//returns 1 is transformation is enabled (invoking func_init) or 0 to discard itself
typedef uchar (*tranform_func_preprocess_compute)(VideoFrame *video_frame,
		FileDB *fileDB, void* state, char **out_dataToSave);

typedef void (*tranform_func_preprocess_load)(void* state, const char *savedData);

//la transformacion puede modificar la imagen o puede retornar una nueva.
//los frames no necesariamente son consecutivos, xq se puede reiniciar el
//video si es que se le agrega otra transformacion que requiera un preproceso.
typedef IplImage* (*tranform_func_transform_frame)(IplImage *image,
		int64_t numFrame, void* state);

//hacer free de los objetos dentro del estado y del estado
typedef void (*tranform_func_release)(void* state);

typedef struct {
	const char *trCode, *helpText;
	tranform_func_new func_new;
	tranform_func_preprocess_compute func_preprocess_compute;
	tranform_func_preprocess_load func_preprocess_load;
	tranform_func_transform_frame func_transform_frame;
	tranform_func_release func_release;
} Transform_Def;

typedef struct {
	Transform_Def *def;
	char *codeAndParameters, *preprocessSavedData;
	bool mustPreprocessCompute, mustPreprocessLoad;
	void *state;
} Transform;

Transform_Def *newTransformDef(const char *code, const char *helpText);
Transform_Def *findTransformDef(const char *code);
Transform *findTransform2(const char *code, const char *parameters);
Transform *findTransform(const char *codeAndParameters);
void releaseTransform(Transform *tr);

#endif
#endif
