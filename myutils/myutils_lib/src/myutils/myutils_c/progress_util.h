/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_PROGRESS_UTIL_H
#define MY_PROGRESS_UTIL_H

#include "../myutils_c.h"

MyProgress* my_progress_new(const char *logger_name, int64_t contMaximum,
		int64_t startingThreads);
void my_progress_addN(MyProgress *lt, int64_t n);
void my_progress_add1(MyProgress *lt);
void my_progress_setN(MyProgress *lt, int64_t n);
void my_progress_substractFromMaximum(MyProgress *lt, int64_t n);
void my_progress_updateActiveThreads(MyProgress *lt, int64_t activeThreads);
void my_progress_release(MyProgress *lt);

#endif
