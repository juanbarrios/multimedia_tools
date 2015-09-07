/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_IO_TEMP_UTIL_H
#define MY_IO_TEMP_UTIL_H

#include "../myutils_c.h"

void my_io_temp_setTempDir(const char* dirname_temp);
void my_io_temp_setFilenamePrefix(const char* filename_prefix);
char *my_io_temp_createNewFile(const char* suffix);

#endif
