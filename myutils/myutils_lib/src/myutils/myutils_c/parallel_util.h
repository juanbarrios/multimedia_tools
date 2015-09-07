/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_PARALLEL_UTIL_H_
#define MY_PARALLEL_UTIL_H_

#include "../myutils_c.h"

#define MY_MUTEX_NEWSTATIC(varMutex) static pthread_mutex_t varMutex = PTHREAD_MUTEX_INITIALIZER
#define MY_MUTEX_INIT(varMutex)(my_mutex_init(&(varMutex)))
#define MY_MUTEX_LOCK(varMutex)(my_mutex_lock(&(varMutex)))
#define MY_MUTEX_UNLOCK(varMutex)(my_mutex_unlock(&(varMutex)))
#define MY_MUTEX_DESTROY(varMutex)(my_mutex_destroy(&(varMutex)))
#define MY_MUTEX_TIMEDWAIT(varCond, varMutex, seconds)(my_mutex_timedwait(&(varCond), &(varMutex), seconds))

void my_mutex_init(pthread_mutex_t *mut);
void my_mutex_lock(pthread_mutex_t *mut);
void my_mutex_unlock(pthread_mutex_t *mut);
void my_mutex_destroy(pthread_mutex_t *mut);
void my_mutex_timedwait(pthread_cond_t *cv, pthread_mutex_t *external_mutex,
		int64_t maxWaitSeconds);

typedef void (*my_parallel_func_incremental)(int64_t current_process,
		void *state_object, int64_t current_thread);

void my_parallel_incremental(int64_t total_processes, void *state_object,
		my_parallel_func_incremental func_incremental, const char *logger_name,
		int64_t max_threads);

typedef void (*my_parallel_func_buffered)(int64_t start_process,
		int64_t end_process_notIncluded, void *state_object, MyProgress *lt,
		int64_t current_thread);

void my_parallel_buffered(int64_t total_processes, void *state_object,
		my_parallel_func_buffered func_buffered, const char *logger_name,
		int64_t max_threads, int64_t buffer_size);

void my_parallel_setNumberOfCores(int64_t num_cores);
int64_t my_parallel_getNumberOfCores();

void my_parallel_sleep(int64_t milliseconds);

#endif
