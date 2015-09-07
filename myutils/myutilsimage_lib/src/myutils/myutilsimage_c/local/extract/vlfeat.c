/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "vlfeat.h"

#ifndef NO_VLFEAT
#ifndef NO_OPENCV
#include <vl/sift.h>
#include <vl/dsift.h>

struct MyVlfeatOptions {
	float *imgBuffer_float;
	VlSiftFilt *vlfeat_sift;
	VlDsiftFilter *vlfeat_dsift;
	int vl_sift_scales;
	bool vl_isDense, vl_sift_noangle, vl_sift_is4f;
	int lastSize_width, lastSize_height;
};

static void parse_file_vlfeat(char *filenameTxt,
		MyLocalDescriptors *descriptors) {
	if (!my_io_existsFile(filenameTxt))
		return;
	MyVectorString *lines = my_io_loadLinesFile(filenameTxt);
	if (my_vectorString_size(lines) < 1)
		return;
	int dimension = 128;
	my_localDescriptors_redefineVectorDatatype(descriptors, MY_DATATYPE_UINT8,
			dimension, my_vectorString_size(lines));
	uchar vector[dimension];
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		MyVectorString *values = my_tokenizer_splitLine(line, ' ');
		my_assert_equalInt("line length", my_vectorString_size(values),
				dimension + 4);
		double x = my_parse_double(my_vectorString_get(values, 0));
		double y = my_parse_double(my_vectorString_get(values, 1));
		double scale = my_parse_double(my_vectorString_get(values, 2));
		double orientation = my_parse_double(my_vectorString_get(values, 3));
		for (int64_t i = 0; i < dimension; ++i)
			vector[i] = my_parse_uint8(my_vectorString_get(values, i + 4));
		my_vectorString_release(values, true);
		my_localDescriptors_setKeypoint(descriptors, i - 1, x, y, scale,
				orientation);
		my_localDescriptors_setVector(descriptors, i - 1, vector);
	}
	my_vectorString_release(lines, true);
}

static void my_vlfeat_extract_filename(const char *filename,
		MyLocalDescriptors *descriptors) {
	my_localDescriptors_redefineNumDescriptors(descriptors, 0);
	const char *binary = my_env_getString_def2("VLFEAT_BIN", "sift.exe",
			"sift");
	char *fLog = my_newString_format("%s.log", filename);
	char *fDesc = my_newString_format("%s.desc", filename);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	char *cmd = my_newString_format("%s -o %s %s > %s 2>&1", binary, fDesc,
			filename, fLog);
	my_io_system(cmd);
	parse_file_vlfeat(fDesc, descriptors);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	MY_FREE_MULTI(fLog, fDesc, cmd);
}

static void my_vlfeat_addToDescriptors(double x, double y, double scale,
		double angle, const float *siftBuffer, bool is4f,
		MyLocalDescriptors *ldes) {
	int64_t cont = my_localDescriptors_getNumDescriptors(ldes);
	if (angle > M_PI)
		angle -= 2 * M_PI;
	my_localDescriptors_redefineNumDescriptors(ldes, cont + 1);
	my_localDescriptors_setKeypoint(ldes, cont, x, y, scale, angle);
	if (is4f) {
		my_localDescriptors_setVector(ldes, cont, siftBuffer);
	} else {
		//convertion to uchar is multiply by 512
		//as implemented in vlfeat/sift.c:743
		uchar vector[128];
		for (int h = 0; h < 128; ++h)
			vector[h] = (uchar) (512.0 * siftBuffer[h]);
		my_localDescriptors_setVector(ldes, cont, vector);
	}
}
static void my_vlfeat_compute_vlSift(VlSiftFilt *vl, float *imgBuffer,
bool noAngles, bool is4f, MyLocalDescriptors *ldes) {
	float siftBuffer[128];
	int64_t n = vl_sift_process_first_octave(vl, imgBuffer);
	while (n != VL_ERR_EOF) {
		vl_sift_detect(vl);
		int64_t nkeys = vl_sift_get_nkeypoints(vl);
		const VlSiftKeypoint *keys = vl_sift_get_keypoints(vl);
		for (int64_t y = 0; y < nkeys; ++y) {
			const VlSiftKeypoint *key = keys + y;
			if (noAngles) {
				vl_sift_calc_keypoint_descriptor(vl, siftBuffer, key, 0);
				my_vlfeat_addToDescriptors(key->x, key->y, key->sigma, 0,
						siftBuffer, is4f, ldes);
			} else {
				double angles[4];
				int64_t nangles = vl_sift_calc_keypoint_orientations(vl, angles,
						key);
				for (int64_t x = 0; x < nangles; ++x) {
					vl_sift_calc_keypoint_descriptor(vl, siftBuffer, key,
							angles[x]);
					my_vlfeat_addToDescriptors(key->x, key->y, key->sigma,
							angles[x], siftBuffer, is4f, ldes);
				}
			}
		}
		n = vl_sift_process_next_octave(vl);
	}
}
static void my_vlfeat_compute_vlSiftDense(VlDsiftFilter *vl, float *imgBuffer,
bool is4f, MyLocalDescriptors *ldes) {
	vl_dsift_process(vl, imgBuffer);
	int64_t nkeys = vl_dsift_get_keypoint_num(vl);
	int64_t nDes = vl_dsift_get_descriptor_size(vl);
	my_assert_equalInt("descriptor length", nDes, 128);
	const VlDsiftKeypoint *keys = vl_dsift_get_keypoints(vl);
	float const *desc = vl_dsift_get_descriptors(vl);
	for (int64_t y = 0; y < nkeys; ++y) {
		const VlDsiftKeypoint *key = keys + y;
		float const *descriptor = desc + nDes * y;
		my_vlfeat_addToDescriptors(key->x, key->y, key->s, key->norm,
				descriptor, is4f, ldes);
	}
}

#define MY_VL_SCALES_MORE 1
#define MY_VL_SCALES_NO 3

static int mt_getFirstOctave(struct MyVlfeatOptions *es) {
	if (es->vl_sift_scales == MY_VL_SCALES_MORE)
		return -1;
	return 0;
}
static int my_vlfeat_getNumOctaves(struct MyVlfeatOptions *es) {
	if (es->vl_sift_scales == MY_VL_SCALES_NO)
		return 1;
	int nOctaves = (int) (log(MIN(es->lastSize_width, es->lastSize_height))
			/ log(2));
	nOctaves = MAX(1, nOctaves);
	return nOctaves;
}
static void my_vlfeat_testSize(struct MyVlfeatOptions *es,
		int currentSize_width, int currentSize_height) {
	if (currentSize_width == es->lastSize_width
			&& currentSize_height == es->lastSize_height)
		return;
	es->lastSize_width = currentSize_width;
	es->lastSize_height = currentSize_height;
	if (es->vl_isDense) {
		if (es->vlfeat_dsift != NULL)
			vl_dsift_delete(es->vlfeat_dsift);
		if (currentSize_width == 0 && currentSize_height == 0)
			return;
		int step = MIN(currentSize_width, currentSize_height) / 10;
		es->vlfeat_dsift = vl_dsift_new_basic(currentSize_width,
				currentSize_height, step, step);
	} else {
		if (es->vlfeat_sift != NULL)
			vl_sift_delete(es->vlfeat_sift);
		if (currentSize_width == 0 && currentSize_height == 0)
			return;
		int firstOctave = mt_getFirstOctave(es);
		int nOctaves = my_vlfeat_getNumOctaves(es);
		es->vlfeat_sift = vl_sift_new(currentSize_width, currentSize_height,
				nOctaves, 3, firstOctave);
		//vl_sift_set_edge_thresh(es->vlfeat_sift, 10); //menor a 10
		//vl_sift_set_peak_thresh(es->vlfeat_sift, 0.015); //mayor a 0
	}
}

void my_local_vlfeat_extract(IplImage *image, MyLocalDescriptors *ldes,
		struct MyVlfeatOptions *es) {
	if (0) {
		char *filename = my_io_temp_createNewFile(".pgm");
		uchar *buffer = NULL;
		my_image_pgm_saveBuffer(filename, image, &buffer);
		//cvSaveImage(filename, image, NULL);
		if (buffer != NULL)
			free(buffer);
		my_vlfeat_extract_filename(filename, ldes);
		my_io_deleteFile(filename, false);
		free(filename);
	}
	my_localDescriptors_redefineVectorDatatype(ldes,
			(es->vl_sift_is4f) ? MY_DATATYPE_FLOAT32 : MY_DATATYPE_UINT8, 128,
			0);
	my_vlfeat_testSize(es, image->width, image->height);
	my_image_copyPixelsToFloatArray(image, &es->imgBuffer_float);
	if (es->vl_isDense) {
		my_vlfeat_compute_vlSiftDense(es->vlfeat_dsift, es->imgBuffer_float,
				es->vl_sift_is4f, ldes);
	} else {
		my_vlfeat_compute_vlSift(es->vlfeat_sift, es->imgBuffer_float,
				es->vl_sift_noangle, es->vl_sift_is4f, ldes);
	}
}
struct MyVlfeatOptions *my_local_vlfeat_newOptionsParse(const char *parameters) {
	struct MyVlfeatOptions *es = MY_MALLOC(1, struct MyVlfeatOptions);
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	while (my_tokenizer_hasNext(tk)) {
		const char *opt = my_tokenizer_nextToken(tk);
		if (my_string_equals_ignorecase(opt, "4F")) {
			es->vl_sift_is4f = true;
		} else if (my_string_equals_ignorecase(opt, "DENSE")) {
			es->vl_isDense = true;
		} else if (my_string_equals_ignorecase(opt, "NOANGLE")) {
			es->vl_sift_noangle = true;
		} else if (my_string_equals_ignorecase(opt, "MORESCALES")) {
			es->vl_sift_scales = MY_VL_SCALES_MORE;
		} else if (my_string_equals_ignorecase(opt, "NOSCALES")) {
			es->vl_sift_scales = MY_VL_SCALES_NO;
		} else {
			my_log_error("unknown option %s. help=%s\n", opt,
					my_local_vlfeat_helpOptions());
		}
	}
	my_tokenizer_releaseValidateEnd(tk);
	return es;

}
void my_local_vlfeat_releaseOptions(struct MyVlfeatOptions *options) {
	my_vlfeat_testSize(options, 0, 0);
	MY_FREE_MULTI(options->imgBuffer_float, options);
}
const char *my_local_vlfeat_helpOptions() {
	return "(NOSCALES)_(MORESCALES)_(DENSE)_(4F)_(NOANGLE)";
}
#endif
#endif
