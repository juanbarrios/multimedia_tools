/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "io_util.h"
#include <errno.h>

MY_MUTEX_NEWSTATIC(mutex_system);
static bool first = true;

int my_io_system_sequential(const char *command_line) {
	MY_MUTEX_LOCK(mutex_system);
	if (first) {
		my_log_info("using sequential system\n");
		first = false;
	}
	my_log_info_time("system: %s\n", command_line);
	errno = 0;
	int exitCode = system(command_line);
	if (exitCode != 0) {
		my_log_info("system exit_code=%i (errno=%i)\n", exitCode, errno);
	}
	MY_MUTEX_UNLOCK(mutex_system);
	return exitCode;
}

static my_io_system_function my_system = my_io_system_sequential;

int my_io_system(const char *command_line) {
	return my_system(command_line);
}

void my_io_system_setSystemImpl(my_io_system_function system_function) {
	MY_MUTEX_LOCK(mutex_system);
	if (system_function == NULL)
		my_system = my_io_system_sequential;
	else
		my_system = system_function;
	MY_MUTEX_UNLOCK(mutex_system);
}
