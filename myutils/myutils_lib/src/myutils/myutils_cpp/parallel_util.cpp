/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "parallel_util.hpp"
#include "../myutils_c.h"

using namespace my;

void parallel::setNumberOfCores(long long num_cores) {
	my_parallel_setNumberOfCores(num_cores);
}
long long parallel::getNumberOfCores() {
	return my_parallel_getNumberOfCores();
}

void parallel::sleep(long long milliseconds) {
	my_parallel_sleep(milliseconds);
}

ParallelProcess::~ParallelProcess() {
}

static void func_parallel(int64_t current_process, void *state_object,
		int64_t current_thread) {
	ParallelProcess *proc = (ParallelProcess*) state_object;
	proc->execute(current_process, current_thread);
}

void parallel::incremental(long long total_processes,
		const ParallelProcess &object, std::string logger_name,
		long long max_threads) {
	my_parallel_incremental(total_processes, (void*) &object, func_parallel,
			logger_name.c_str(), max_threads);
}
