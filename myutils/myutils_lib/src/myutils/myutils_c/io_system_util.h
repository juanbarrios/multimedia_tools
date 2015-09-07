/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_IO_SYSTEM_UTIL_H
#define MY_IO_SYSTEM_UTIL_H

#include "../myutils_c.h"

typedef int (*my_io_system_function)(const char *command_line);

//it uses global pthread_mutex to callt to "system to avoid parallel executions
int my_io_system_sequential(const char *command_line);

//"system" method cannot be executed in parallel (the behavior is undefined)
//thus the default implementation is my_io_system_sequential
int my_io_system(const char *command_line);

//the default "system" can be used
//(default system may throw segmentation fault if it is called in parallel)
void my_io_system_setSystemImpl(my_io_system_function system_function);

#endif
