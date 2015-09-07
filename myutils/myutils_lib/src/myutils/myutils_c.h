/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILLIBC_H
#define MYUTILLIBC_H

#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error "This library needs at least a C99 or C++11 compliant compiler"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if WIN32
#define __USE_MINGW_ANSI_STDIO 1
#endif

#if defined WIN32 || defined _WIN32
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#endif

//_POSIX_C_SOURCE
#if defined __linux || defined __linux__
#define IS_LINUX 1
#else
#define IS_LINUX 0
#endif

#ifdef __GNUC__
#define SUPPRESS_NOT_USED_WARN __attribute__ ((unused))
#else
#define SUPPRESS_NOT_USED_WARN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

typedef struct MyVectorInt MyVectorInt;
typedef struct MyVectorDouble MyVectorDouble;
typedef struct MyVectorString MyVectorString;
typedef struct MyVectorObj MyVectorObj;
typedef struct MyMapObjObj MyMapObjObj;
typedef struct MyMapStringObj MyMapStringObj;
typedef struct MyLineReader MyLineReader;
typedef struct MyProgress MyProgress;
typedef struct MyTimer MyTimer;
typedef struct {
	int8_t my_datatype_code;
} MyDatatype;

#include "myutils_c/mem_util.h"
#include "myutils_c/log_util.h"
#include "myutils_c/timer_util.h"
#include "myutils_c/collection_util.h"
#include "myutils_c/progress_util.h"
#include "myutils_c/parallel_util.h"
#include "myutils_c/assert_util.h"
#include "myutils_c/env_util.h"
#include "myutils_c/datatype_util.h"
#include "myutils_c/math_stats.h"
#include "myutils_c/math_util.h"
#include "myutils_c/string_util.h"
#include "myutils_c/string_sbuffer_util.h"
#include "myutils_c/string_tokenizer_util.h"
#include "myutils_c/io_util.h"
#include "myutils_c/io_lreader_util.h"
#include "myutils_c/io_system_util.h"
#include "myutils_c/io_temp_util.h"
#include "myutils_c/random_util.h"
#include "myutils_c/parse_util.h"
#include "myutils_c/sparse_array.h"

#ifdef __cplusplus
}
#endif

#endif
