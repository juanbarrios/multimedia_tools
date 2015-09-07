/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_MATCH {
	char *filename, *localDescriptor;
	VideoFrame* video_frame_2;
	MyImagePair *imagenUnida;
	MyImageResizer *resizer;
	Extractor *ext_1, *ext_2;
	MyLocalDescriptors *des_2;
	struct PairMatcher *matcher;
	uchar hasInit;
};

static void tra_config_match(const char *trCode, const char *trParameters,
		void **out_state) {
	MyTokenizer *tk = my_tokenizer_new(trParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct Proceso_MATCH *es = MY_MALLOC(1, struct Proceso_MATCH);
	const char *descriptorLocal = my_tokenizer_nextToken(tk);
	es->ext_1 = getExtractor(descriptorLocal);
	es->ext_2 = getExtractor(descriptorLocal);
	const char *nombreMatcher = my_tokenizer_nextToken(tk);
	es->matcher = newPairMatcher(nombreMatcher);
	es->filename = my_newString_string(my_tokenizer_nextToken(tk));
	my_tokenizer_releaseValidateEnd(tk);
	es->resizer = my_imageResizer_newScreenSize();
	*out_state = es;
}

static IplImage *tra_transformar_match(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_MATCH *es = estado;
	MyLocalDescriptors *des_1 = extractVolatileDescriptor(es->ext_1, imagen);
	if (!es->hasInit) {
		es->video_frame_2 = openFile(es->filename, 1);
		if (!loadNextFrame(es->video_frame_2))
			return NULL;
		es->hasInit = 1;
		IplImage *frame2 = getCurrentFrameOrig(es->video_frame_2);
		es->des_2 = extractVolatileDescriptor(es->ext_2, frame2);
	} else if (getIsVideo(es->video_frame_2)
			&& loadNextFrame(es->video_frame_2)) {
		IplImage *frame2 = getCurrentFrameOrig(es->video_frame_2);
		es->des_2 = extractVolatileDescriptor(es->ext_2, frame2);
	}
	if (es->imagenUnida == NULL)
		es->imagenUnida = my_imagePair_new(imagen,
				getCurrentFrameOrig(es->video_frame_2),
				MY_IMAGEPAIR_HORIZONTAL_BOTTOM, 0, true);
	else
		my_imagePair_updateImages(es->imagenUnida, imagen,
				getCurrentFrameOrig(es->video_frame_2));
	int64_t numMatches = computePairMatches(es->matcher, des_1, es->des_2);
	my_log_info(
			"vectors-L:%4"PRIi64"  vectors-R:%4"PRIi64"  Matches:%3"PRIi64"\n",
			my_localDescriptors_getNumDescriptors(des_1),
			my_localDescriptors_getNumDescriptors(es->des_2), numMatches);
	if (numMatches > 0) {
		struct MatchesAndModel *pairs = getLastMatchesAndModel(es->matcher);
		my_imagePair_drawMatches(es->imagenUnida, des_1, es->des_2,
				pairs->numMatches, pairs->idVectorQ, pairs->idVectorR);
	}
	return my_imageResizer_resizeImage(my_imagePair_getIplImage(es->imagenUnida), es->resizer);
}
static void tra_release_match(void *estado) {
	struct Proceso_MATCH *es = estado;
	my_imagePair_release(es->imagenUnida);
	closeVideo(es->video_frame_2);
	releaseExtractor(es->ext_1);
	releaseExtractor(es->ext_2);
	my_imageResizer_release(es->resizer);
	MY_FREE(es);
}
void tra_reg_match() {
	Transform_Def *def = newTransformDef("MATCH",
			"LocalDescriptor_LocalDistance_filename");
	def->func_new = tra_config_match;
	def->func_transform_frame = tra_transformar_match;
	def->func_release = tra_release_match;
}

#endif
