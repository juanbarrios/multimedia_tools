/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_LOG_UTIL_H
#define MY_LOG_UTIL_H

#include "../myutils_c.h"

void my_log_init();
void my_log_noOutputStdout();
void my_log_setErrorFunction(void (*func_executeOnError)());

void my_log_info(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void my_log_info_time(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void my_log_error(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

void my_log_setOutputFile(FILE *out);
void my_log_closeOutputFile();

#endif
