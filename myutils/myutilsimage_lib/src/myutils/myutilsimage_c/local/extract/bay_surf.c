/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "bay_surf.h"
#include <myutils/myutils_c.h>

static void parse_file_bay(char *filenameTxt, MyLocalDescriptors *descriptors) {
	if (!my_io_existsFile(filenameTxt))
		return;
	MyVectorString *lines = my_io_loadLinesFile(filenameTxt);
	if (my_vectorString_size(lines) < 3) {
		my_vectorString_release(lines, true);
		return;
	}
	int64_t dimension = my_parse_int(my_vectorString_get(lines, 0)) - 1;
	int64_t numDesc = my_parse_int(my_vectorString_get(lines, 1));
	my_assert_equalInt("numLines", numDesc, my_vectorString_size(lines) - 2);
	my_localDescriptors_redefineVectorDatatype(descriptors, MY_DATATYPE_FLOAT32,
			dimension, numDesc);
	float vector[dimension];
	for (int64_t i = 0; i < my_vectorString_size(lines) - 2; ++i) {
		char *line = my_vectorString_get(lines, i + 2);
		MyVectorString *values = my_tokenizer_splitLine(line, ' ');
		my_assert_equalInt("line length", my_vectorString_size(values),
				dimension + 6);
		double x = my_parse_double(my_vectorString_get(values, 0));
		double y = my_parse_double(my_vectorString_get(values, 1));
		//a b c sign
		for (int64_t j = 0; j < dimension; ++j)
			vector[j] = my_parse_double(my_vectorString_get(values, j + 6));
		my_vectorString_release(values, true);
		my_localDescriptors_setKeypoint(descriptors, i, x, y, 0, 0);
		my_localDescriptors_setVector(descriptors, i, vector);
	}
	my_vectorString_release(lines, true);
}

static void extract_bay(const char *filename, MyLocalDescriptors *descriptors) {
	my_localDescriptors_redefineNumDescriptors(descriptors, 0);
	const char *binary = my_env_getString_def2("BAY_BIN", "surfWINDLLDemo.exe",
			"surf.ln");
	char *fLog = my_newString_format("%s.log", filename);
	char *fDesc = my_newString_format("%s.desc", filename);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	char *cmd = my_newString_format("%s -i %s -o %s > %s", binary, filename,
			fDesc, fLog);
	my_io_system(cmd);
	parse_file_bay(fDesc, descriptors);
	my_io_deleteFile(fLog, false);
	my_io_deleteFile(fDesc, false);
	MY_FREE_MULTI(fLog, fDesc, cmd);
}

void my_local_bay_extract_filenamePgm(const char *filename,
		MyLocalDescriptors *descriptors) {
	extract_bay(filename, descriptors);
}

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

void my_local_bay_extract(IplImage *image, MyLocalDescriptors *descriptors) {
	char *filename = my_io_temp_createNewFile(".pgm");
	my_image_pgm_saveImage(filename, image);
	extract_bay(filename, descriptors);
	my_io_deleteFile(filename, false);
	free(filename);
}
#endif
