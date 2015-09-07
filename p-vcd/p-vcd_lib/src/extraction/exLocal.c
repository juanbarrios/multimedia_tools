/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"
#include <myutils/myutilsimage_c.h>

#ifndef NO_OPENCV
struct State_CD {
	char *filenameImg;
	MyImageResizer *resizer;
	MyLocalDescriptors *descriptor;
	void *options;
	bool is_bay, is_lowe, is_mk, is_cv;
#ifndef NO_VLFEAT
	bool is_vl;
#endif
	uchar *save_buffer;
	MknnPcaAlgorithm *pca;
	int64_t pca_dimensions;
	MyLocalDescriptors *pca_descriptor;
};

static void new_local(const char *code, const char *parameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	struct State_CD *es = MY_MALLOC(1, struct State_CD);
	if (my_string_equals_ignorecase(code, "BAY")) {
		es->is_bay = true;
	} else if (my_string_equals_ignorecase(code, "LOWE")) {
		es->is_lowe = true;
	} else if (my_string_equals_ignorecase(code, "MK")) {
		es->is_mk = true;
	} else if (my_string_equals_ignorecase(code, "CV")) {
		es->is_cv = true;
#ifndef NO_VLFEAT
	} else if (my_string_equals_ignorecase(code, "VL")) {
		es->is_vl = true;
#endif
	} else {
		my_log_error("unknown local %s\n", code);
	}
	MyTokenizer *tk = my_tokenizer_new(parameters, ',');
	if (es->is_mk)
		es->options = my_local_mikolajczyk_newOptionsParse(
				my_tokenizer_nextToken(tk));
	else if (es->is_cv)
		es->options = my_local_opencv_newOptionsParse(
				my_tokenizer_nextToken(tk));
#ifndef NO_VLFEAT
	else if (es->is_vl)
		es->options = my_local_vlfeat_newOptionsParse(
				my_tokenizer_nextToken(tk));
#endif
	es->resizer = my_imageResizer_newTextOptions(my_tokenizer_nextToken(tk));
	if (my_tokenizer_hasNext(tk)) {
		MyTokenizer *tk2 = my_tokenizer_new(my_tokenizer_nextToken(tk), '_');
		my_assert_equalString("PCA parameters", my_tokenizer_nextToken(tk2),
				"PCA");
		es->pca = mknn_pca_new();
		const char *filename = my_tokenizer_nextToken(tk2);
		mknn_pca_restore(es->pca, filename);
		es->pca_dimensions = my_tokenizer_nextInt(tk2);
		my_tokenizer_releaseValidateEnd(tk2);
		es->pca_descriptor = my_localDescriptors_new(0, MY_DATATYPE_FLOAT32,
				es->pca_dimensions);
	}
	my_tokenizer_releaseValidateEnd(tk);
	if (es->is_mk)
		es->filenameImg = my_io_temp_createNewFile(".png");
	else if (es->is_bay || es->is_lowe)
		es->filenameImg = my_io_temp_createNewFile(".pgm");
	es->descriptor = my_localDescriptors_new(0, MY_DATATYPE_INT8, 0);
	*out_state = es;
	*out_td = descriptorType(DTYPE_LOCAL_VECTORS, 0, 0, 0);
	*out_useImgGray = (es->is_bay || es->is_lowe
#ifndef NO_VLFEAT
			|| es->is_vl
#endif
			) ? true : false;
}
static void* extract_local(IplImage *image, void *state) {
	struct State_CD *es = state;
	IplImage *imgResized = my_imageResizer_resizeImage(image, es->resizer);
	if (es->is_bay) {
		my_image_pgm_saveBuffer(es->filenameImg, imgResized, &es->save_buffer);
		my_local_bay_extract(imgResized, es->descriptor);
	} else if (es->is_lowe) {
		my_image_pgm_saveBuffer(es->filenameImg, imgResized, &es->save_buffer);
		my_local_lowe_extract(imgResized, es->descriptor);
	} else if (es->is_mk) {
		cvSaveImage(es->filenameImg, imgResized, NULL);
		my_local_mikolajczyk_extract_filename(es->filenameImg, es->descriptor,
				es->options);
	} else if (es->is_cv) {
		my_local_opencv_extract(imgResized, es->descriptor, es->options);
#ifndef NO_VLFEAT
	} else if (es->is_vl) {
		my_local_vlfeat_extract(imgResized, es->descriptor, es->options);
#endif
	}
	my_localDescriptors_rescaleKeypoints(es->descriptor, imgResized->width,
			imgResized->height, image->width, image->height);
	if (es->pca != NULL) {
		applyPcaToLocalDescriptors(es->pca, es->descriptor, es->pca_descriptor);
		return es->pca_descriptor;
	} else {
		return es->descriptor;
	}
}
static void release_local(void *state) {
	struct State_CD *es = state;
	if (es->filenameImg != NULL) {
		my_io_deleteFile(es->filenameImg, false);
		free(es->filenameImg);
	}
	if (es->is_mk)
		my_local_mikolajczyk_releaseOptions(es->options);
	else if (es->is_cv)
		my_local_opencv_releaseOptions(es->options);
#ifndef NO_VLFEAT
	else if (es->is_vl)
		my_local_vlfeat_releaseOptions(es->options);
#endif
	my_imageResizer_release(es->resizer);
	my_localDescriptors_release(es->descriptor);
	if (es->save_buffer != NULL)
		free(es->save_buffer);
	free(es);
}
void reg_local_local() {
	addExtractorGlobalDef(true, "BAY",
			"[resizeOptions],[quantOptions],[PCA_filestate_dims]", new_local,
			extract_local, release_local);
	addExtractorGlobalDef(true, "LOWE",
			"[resizeOptions],[quantOptions],[PCA_filestate_dims]", new_local,
			extract_local, release_local);
	addExtractorGlobalDef(true, "MK",
			my_newString_format(
					"%s,[resizeOptions],[quantOptions],[PCA_filestate_dims]",
					my_local_mikolajczyk_helpOptions()), new_local,
			extract_local, release_local);
	addExtractorGlobalDef(true, "CV",
			my_newString_format(
					"%s,[resizeOptions],[quantOptions],[PCA_filestate_dims]",
					my_local_opencv_helpOptions()), new_local, extract_local,
			release_local);
#ifndef NO_VLFEAT
	addExtractorGlobalDef(true, "VL",
			my_newString_format(
					"%s,[resizeOptions],[quantOptions],[PCA_filestate_dims]",
					my_local_vlfeat_helpOptions()), new_local, extract_local,
			release_local);
#endif
}
#endif
