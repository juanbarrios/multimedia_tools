/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_IO_LREADER_UTIL_H
#define MY_IO_LREADER_UTIL_H

#include "../myutils_c.h"

MyLineReader *my_lreader_open(FILE *inFile);
MyLineReader *my_lreader_open_params(FILE *inFile, int64_t buffer_size);
MyLineReader *my_lreader_config_open(FILE *inFile, const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version);
MyLineReader *my_lreader_config_open_params(FILE *inFile, int64_t buffer_size,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version);
const char *my_lreader_readLine(MyLineReader *reader);
const char *my_lreader_readLineOrComment(MyLineReader *reader);
char *my_lreader_readLine_newString(MyLineReader *reader);
void my_lreader_close(MyLineReader *reader, bool close_input_file);

#endif
