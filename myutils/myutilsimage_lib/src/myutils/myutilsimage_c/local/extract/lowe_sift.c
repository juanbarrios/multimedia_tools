/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "lowe_sift.h"
#include <myutils/myutils_c.h>

//SIFT reference implementation
//Downloaded from: http://www.cs.ubc.ca/~lowe/keypoints/
static void parse_file_lowe(char *filenameTxt, MyLocalDescriptors *descriptors) {
	if (!my_io_existsFile(filenameTxt))
		return;
	MyVectorString *lines = my_io_loadLinesFile(filenameTxt);
	if (my_vectorString_size(lines) < 2)
		return;
	MyVectorString *values1 = my_tokenizer_splitLine(
			my_vectorString_get(lines, 0), ' ');
	int64_t numDesc = my_parse_int(my_vectorString_get(values1, 0));
	int64_t dimension = my_parse_int(my_vectorString_get(values1, 1));
	my_vectorString_release(values1, true);
	my_assert_equalInt("dimension", dimension, 128);
	my_assert_equalInt("num lines", my_vectorString_size(lines),
			8 * numDesc + 1);
	my_localDescriptors_redefineVectorDatatype(descriptors, MY_DATATYPE_UINT8,
			dimension, numDesc);
	uint8_t vector[dimension];
	int cont_vec = 0;
	for (int64_t i = 1; i < my_vectorString_size(lines); i += 8) {
		char *line_kp = my_vectorString_get(lines, i);
		MyVectorString *values = my_tokenizer_splitLine(line_kp, ' ');
		my_assert_equalInt("line length", my_vectorString_size(values), 4);
		double y = my_parse_double(my_vectorString_get(values, 0));
		double x = my_parse_double(my_vectorString_get(values, 1));
		double scale = my_parse_double(my_vectorString_get(values, 2));
		double orientation = my_parse_double(my_vectorString_get(values, 3));
		my_vectorString_release(values, true);
		int pos = 0;
		for (int j = 0; j < 7; ++j) {
			char *line_vec = my_vectorString_get(lines, i + j + 1);
			my_assert_equalInt("line prefix", line_vec[0], ' ');
			MyVectorString *values2 = my_tokenizer_splitLine(line_vec + 1, ' ');
			my_assert_lessEqualInt("line vector length",
					pos + my_vectorString_size(values2), dimension);
			for (int64_t k = 0; k < my_vectorString_size(values2); ++k)
				vector[pos++] = my_parse_uint8(my_vectorString_get(values2, k));
			my_vectorString_release(values2, true);
		}
		my_assert_equalInt("dimension vector", pos, dimension);
		my_localDescriptors_setKeypoint(descriptors, cont_vec, x, y, scale,
				orientation);
		my_localDescriptors_setVector(descriptors, cont_vec, vector);
		cont_vec++;
	}
	my_vectorString_release(lines, true);
}
static void extract_lowe(const char *filename, MyLocalDescriptors *descriptors) {
	my_localDescriptors_redefineNumDescriptors(descriptors, 0);
	const char *binary = my_env_getString_def2("COMPUTEDESCRIPTORS",
			"siftWin32.exe", "sift");
	char *fLog = my_newString_format("%s.log", filename);
	char *fDesc = my_newString_format("%s.desc", filename);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	char *cmd = my_newString_format("%s < %s > %s 2> %s", binary, filename,
			fDesc, fLog);
	my_io_system(cmd);
	parse_file_lowe(fDesc, descriptors);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	MY_FREE_MULTI(fLog, fDesc, cmd);
}

void my_local_lowe_extract_filenamePgm(const char *filename,
		MyLocalDescriptors *descriptors) {
	extract_lowe(filename, descriptors);
}

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

void my_local_lowe_extract(IplImage *image, MyLocalDescriptors *descriptors) {
	char *filename = my_io_temp_createNewFile(".pgm");
	my_image_pgm_saveImage(filename, image);
	extract_lowe(filename, descriptors);
	my_io_deleteFile(filename, false);
	free(filename);
}
#endif
