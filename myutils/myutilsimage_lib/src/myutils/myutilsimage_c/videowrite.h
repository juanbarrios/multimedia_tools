/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_VIDEOWRITE_H
#define MYUTILSIMAGE_VIDEOWRITE_H

#include "../myutilsimage_c.h"

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>

typedef struct VideoWriter VideoWriter;

VideoWriter *newVideoWriter(const char *newFilename, const char *fileExtension,
		const char *codecName, double fps, IplImage *first_frame);
void addVideoFrame(VideoWriter *video_writer, IplImage *frame);
int64_t getCurrentFrame(VideoWriter *video_writer);
char *getFilename(VideoWriter *video_writer);
void closeVideoWriter(VideoWriter *video_writer);

#endif
#endif
