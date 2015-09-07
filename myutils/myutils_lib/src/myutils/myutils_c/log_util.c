/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "log_util.h"

MY_MUTEX_NEWSTATIC(priv_log_mutex);

static bool INIT_LOG = false;
static bool WITH_LOG_TO_STDOUT = true;
static FILE *priv_file_log = NULL;

MY_MUTEX_NEWSTATIC(priv_exit_mutex);

static void my_log_forceExit() {
	MY_MUTEX_LOCK(priv_exit_mutex);
	exit(EXIT_FAILURE);
	MY_MUTEX_UNLOCK(priv_exit_mutex);
}

static void (*func_onError)() = my_log_forceExit;

static void my_log_default_init() {
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	INIT_LOG = 1;
}
void my_log_noOutputStdout() {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	WITH_LOG_TO_STDOUT = 0;
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_setErrorFunction(void (*func_executeOnError)()) {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	func_onError = func_executeOnError;
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_setOutputFile(FILE *out) {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	priv_file_log = out;
	setvbuf(priv_file_log, NULL, _IONBF, 0);
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_closeOutputFile() {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	if (priv_file_log != NULL) {
		fclose(priv_file_log);
		priv_file_log = NULL;
	}
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_info(char* fmt, ...) {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	if (WITH_LOG_TO_STDOUT) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
	if (priv_file_log != NULL) {
		va_list args;
		va_start(args, fmt);
		vfprintf(priv_file_log, fmt, args);
		va_end(args);
	}
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_info_time(char* fmt, ...) {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	char *time_buff = my_timer_currentClock_newString();
	if (WITH_LOG_TO_STDOUT) {
		printf("[%s] ", time_buff);
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
	if (priv_file_log != NULL) {
		fprintf(priv_file_log, "[%s] ", time_buff);
		va_list args;
		va_start(args, fmt);
		vfprintf(priv_file_log, fmt, args);
		va_end(args);
	}
	MY_FREE(time_buff);
	MY_MUTEX_UNLOCK(priv_log_mutex);
}
void my_log_error(char* fmt, ...) {
	MY_MUTEX_LOCK(priv_log_mutex);
	if (!INIT_LOG)
		my_log_default_init();
	{
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
	if (priv_file_log != NULL) {
		va_list args;
		va_start(args, fmt);
		vfprintf(priv_file_log, fmt, args);
		va_end(args);
	}
	MY_MUTEX_UNLOCK(priv_log_mutex);
	func_onError();
}
