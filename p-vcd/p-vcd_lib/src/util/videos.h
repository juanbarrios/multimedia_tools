/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef VIDEOS_H
#define VIDEOS_H

#include "../pvcd.h"

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>

VideoFrame *openFile(const char *filename, bool fail);
VideoFrame *openFileDB(FileDB *fdb, bool fail);

uchar addTransformation(VideoFrame *video_frame, const char *trName);
uchar addTransformationPreprocessed(VideoFrame *video_frame, const char *trName,
		const char *preprocessState);
void removeLastTransformation(VideoFrame *video_frame);
FileDB *getNewFileDB(VideoFrame *video_frame);
bool loadNextFrame(VideoFrame *video_frame);
IplImage *getCurrentFrameOrig(VideoFrame *video_frame);
IplImage *getCurrentFrameGray(VideoFrame *video_frame);
int64_t getCurrentNumFrame(VideoFrame *video_frame);
double getVideoTimeStart(VideoFrame *video_frame);
double getVideoTimeEnd(VideoFrame *video_frame);
char *getVideoName(VideoFrame *video_frame);
double getFpsVideo(VideoFrame *video_frame);
CvSize getFrameSize(VideoFrame *video_frame);
void closeVideo(VideoFrame *video_frame);
bool getIsVideo(VideoFrame *video_frame);
bool getIsImage(VideoFrame *video_frame);
bool getIsAudio(VideoFrame *video_frame);
bool getIsWebcam(VideoFrame *video_frame);
bool seekVideoToFrame(VideoFrame *video_frame, int64_t desiredFrame);
void extractVideoSegment(VideoFrame *video_frame, double secondsStart,
		double secondsEnd, const char *newFilename);

void computeVideoLength(const char *filename, bool useFastApprox,
		double *out_secondsLength, double *out_fps);

#endif
#endif
