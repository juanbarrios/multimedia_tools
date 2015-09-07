/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef PVCD_H
#define PVCD_H

#if WIN32
#define __USE_MINGW_ANSI_STDIO 1
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
#include <pthread.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include <myutils/myutils_c.h>
#include <myutils/myutilsimage_c.h>
#include <metricknn/metricknn_c.h>

//descriptor types
#define DTYPE_ARRAY_UCHAR 1
#define DTYPE_ARRAY_FLOAT 2
#define DTYPE_ARRAY_DOUBLE 6
#define DTYPE_MATRIX_BITS 7
#define DTYPE_SPARSE_ARRAY 10
#define DTYPE_LOCAL_VECTORS 25

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int64_t dtype;
	int64_t array_length;
	int64_t num_subarrays;
	int64_t subarray_length;
} DescriptorType;

typedef struct FileDB {
	bool isVideo, isImage, isAudio;
	char *id, *pathReal, *filenameReal;
	int64_t filesize;
	//images and video. No audio.
	int64_t width, height;
	//video and audio. No image. length=endTime - startTime
	double secStartTime, secEndTime, lengthSec;
	//video (frames per second) and audio (samples per second). No image.
	double numObjsPerSecond;
	int64_t numTransfs;
	char **nameTransf, **preprocessDataTransf;
	struct DB *db;
	int64_t internal_id;
} FileDB;

typedef struct DB {
	char *pathBase, *pathFiles, *pathLogs, *pathSegmentations, *pathDescriptors,
			*pathClustering;
	int64_t numFilesDb;
	bool isImageDb, isAudioDb, isVideoDb;
	struct FileDB **filesDb;
	struct FileDB **filesDb_sortId;
	struct FileDB **filesDb_sortFilenameReal;
} DB;

typedef struct VideoFrame VideoFrame;
typedef struct CmdParams CmdParams;

extern int64_t NUM_CORES;

#include "util/systemCall.h"
#include "util/dbs.h"
#include "util/videos.h"
#include "util/utils.h"
#include "util/audio.h"

struct VideoSegment;
struct Segmentation;
typedef struct LoadSegmentation LoadSegmentation;
typedef struct SaveSegmentation SaveSegmentation;
typedef struct LoadDescriptors LoadDescriptors;
typedef struct SaveDescriptors SaveDescriptors;

#include "util/loadSegmentation.h"
#include "util/saveSegmentation.h"
#include "util/loadDescriptors.h"
#include "util/saveDescriptors.h"

struct SearchSegment;
struct SearchFile;
struct SearchCollection;
struct SearchProfile;

#include "search/bus.h"
#include "extraction/ex.h"
#include "segmentation/seg.h"
#include "transformation/tr.h"
#include "processes/process.h"
#include "detection/detection.h"

#include "pvcd_commands.h"

#ifdef __cplusplus
}
#endif

#endif
