/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_PARALLEL_UTIL_HPP_
#define MY_PARALLEL_UTIL_HPP_

#include "../myutils_cpp.hpp"

namespace my {

class ParallelProcess {
public:
	virtual void execute(long long current_process, long long current_thread)=0;
	virtual ~ParallelProcess()=0;
};

class parallel {
public:

	static void setNumberOfCores(long long num_cores);
	static long long getNumberOfCores();

	static void sleep(long long milliseconds);

	static void incremental(long long total_processes,
			const my::ParallelProcess &object, std::string logger_name,
			long long max_threads);
};

}
#endif
