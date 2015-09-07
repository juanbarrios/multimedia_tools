/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "parallel_util.h"
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#if IS_WINDOWS
#include <windows.h>
#elif IS_LINUX
#include <unistd.h>
#endif

void my_mutex_init(pthread_mutex_t *mut) {
	int ret = pthread_mutex_init(mut, NULL);
	my_assert_equalInt("pthread_mutex_init", ret, 0); //see errno.h
}
void my_mutex_lock(pthread_mutex_t *mut) {
	int ret = pthread_mutex_lock(mut);
	my_assert_equalInt("pthread_mutex_lock", ret, 0); //see errno.h
}
void my_mutex_unlock(pthread_mutex_t *mut) {
	int ret = pthread_mutex_unlock(mut);
	my_assert_equalInt("pthread_mutex_unlock", ret, 0); //see errno.h
}
void my_mutex_destroy(pthread_mutex_t *mut) {
	int ret = pthread_mutex_destroy(mut);
	my_assert_equalInt("pthread_mutex_destroy", ret, 0); //see errno.h
}
void my_mutex_timedwait(pthread_cond_t *cv, pthread_mutex_t *external_mutex,
		int64_t maxWaitSeconds) {
	struct timespec to = { 0 };
	to.tv_sec = time(NULL) + maxWaitSeconds;
	int ret = pthread_cond_timedwait(cv, external_mutex, &to);
	if (ret != ETIMEDOUT)
		my_assert_equalInt("pthread_cond_timedwait", ret, 0);
}

struct ParamThread_Common {
	int64_t cont_threads, active_threads;
	int64_t total_processes;
	void* state_object;
	MyProgress *lt;
	pthread_mutex_t mutex;
};

static void common_init(struct ParamThread_Common *common,
		int64_t total_processes, void* state_object, const char *logger_name) {
	if (logger_name != NULL && strlen(logger_name) > 0)
		common->lt = my_progress_new(logger_name, total_processes, 1);
	common->state_object = state_object;
	common->total_processes = total_processes;
	MY_MUTEX_INIT(common->mutex);
}

static int64_t common_start(struct ParamThread_Common *common) {
	int64_t current_thread = common->cont_threads++;
	common->active_threads++;
	if (common->lt != NULL)
		my_progress_updateActiveThreads(common->lt, common->active_threads);
	return current_thread;
}
static void common_end(struct ParamThread_Common *common) {
	common->active_threads--;
	if (common->lt != NULL)
		my_progress_updateActiveThreads(common->lt, common->active_threads);
}
static void common_release(struct ParamThread_Common *common) {
	if (common->lt != NULL)
		my_progress_release(common->lt);
	MY_MUTEX_DESTROY(common->mutex);
}
struct ParamThread_Incremental {
	int64_t incremental_next_process;
	my_parallel_func_incremental func_incremental;
	struct ParamThread_Common *common;
};
static void* process_thread_sequential(void *params_threads) {
	struct ParamThread_Incremental *param =
			(struct ParamThread_Incremental*) params_threads;
	MY_MUTEX_LOCK(param->common->mutex);
	int64_t current_thread = common_start(param->common);
	while (param->incremental_next_process < param->common->total_processes) {
		int64_t current = param->incremental_next_process++;
		MY_MUTEX_UNLOCK(param->common->mutex);
		param->func_incremental(current, param->common->state_object,
				current_thread);
		MY_MUTEX_LOCK(param->common->mutex);
		if (param->common->lt != NULL)
			my_progress_add1(param->common->lt);
	}
	common_end(param->common);
	MY_MUTEX_UNLOCK(param->common->mutex);
	return NULL;
}

struct ParamThread_Buffered {
	int64_t next_first;
	int64_t buffer_size;
	my_parallel_func_buffered func_buffered;
	struct ParamThread_Common *common;
};

static void* process_thread_buffered(void *params_threads) {
	struct ParamThread_Buffered *param =
			(struct ParamThread_Buffered*) params_threads;
	MY_MUTEX_LOCK(param->common->mutex);
	int64_t current_thread = common_start(param->common);
	while (param->next_first < param->common->total_processes) {
		int64_t first = param->next_first;
		int64_t last = MIN(param->common->total_processes,
				param->next_first + param->buffer_size);
		param->next_first = last;
		MY_MUTEX_UNLOCK(param->common->mutex);
		param->func_buffered(first, last, param->common->state_object,
				param->common->lt, current_thread);
		MY_MUTEX_LOCK(param->common->mutex);
	}
	common_end(param->common);
	MY_MUTEX_UNLOCK(param->common->mutex);
	return NULL;
}
static void run_parallel_threads(void *params_threads,
		void *(*func_thread)(void *params_threads), int64_t max_threads) {
	pthread_t threads[max_threads];
	for (int64_t i = 0; i < max_threads; ++i) {
		int est = pthread_create(&threads[i], NULL, func_thread,
				params_threads);
		my_assert_equalInt("pthread_create", est, 0);
	}
	for (int64_t i = 0; i < max_threads; ++i) {
		int est = pthread_join(threads[i], NULL);
		my_assert_equalInt("pthread_join", est, 0);
	}
}
//processes are completed sequentially by threads
//max_threads < 0 implies maximum threads (number of cores)
void my_parallel_incremental(int64_t total_processes, void *state_object,
		my_parallel_func_incremental func_incremental, const char *logger_name,
		int64_t max_threads) {
	struct ParamThread_Common common = { 0 };
	common_init(&common, total_processes, state_object, logger_name);
	if (max_threads <= 0)
		max_threads = my_parallel_getNumberOfCores();
	max_threads = MAX(1, MIN(max_threads, total_processes));
	struct ParamThread_Incremental p = { 0 };
	p.common = &common;
	p.func_incremental = func_incremental;
	if (max_threads > 1) {
		run_parallel_threads(&p, process_thread_sequential, max_threads);
	} else {
		process_thread_sequential(&p);
	}
	common_release(&common);
}
//processes are completed in groups of buffer_size
//max_threads < 0 implies maximum threads (number of cores)
//buffer_size < 0 implies maximum size (total_processes/max_threads)
void my_parallel_buffered(int64_t total_processes, void *state_object,
		my_parallel_func_buffered func_buffered, const char *logger_name,
		int64_t max_threads, int64_t buffer_size) {
	if (max_threads <= 0)
		max_threads = my_parallel_getNumberOfCores();
	max_threads = MAX(1, MIN(max_threads, total_processes));
	if (buffer_size <= 0)
		buffer_size = my_math_ceil_int(total_processes / (double) max_threads);
	struct ParamThread_Common common = { 0 };
	common_init(&common, total_processes, state_object, logger_name);
	struct ParamThread_Buffered p = { 0 };
	p.common = &common;
	p.buffer_size = buffer_size;
	p.func_buffered = func_buffered;
	if (max_threads > 1) {
		run_parallel_threads(&p, process_thread_buffered, max_threads);
	} else {
		process_thread_buffered(&p);
	}
	common_release(&common);
}

static int64_t fixed_number_of_cores = 0;

void my_parallel_setNumberOfCores(int64_t num_cores) {
	fixed_number_of_cores = num_cores;
}
int64_t my_parallel_getNumberOfCores() {
	if (fixed_number_of_cores > 0)
		return fixed_number_of_cores;
	int64_t number = 0;
#if IS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int64_t sysNumber = sysinfo.dwNumberOfProcessors;
	number = my_env_getInt("NUMBER_OF_PROCESSORS", sysNumber);
#elif IS_LINUX
	number = sysconf( _SC_NPROCESSORS_ONLN );
#endif
	return (number < 1) ? 1 : number;
}
void my_parallel_sleep(int64_t milliseconds) {
	sleep(milliseconds / 1000);
//#ifdef IS_WINDOWS
//Sleep(milliseconds);
//#else
//usleep(milliseconds * 1000);
//#endif
}
