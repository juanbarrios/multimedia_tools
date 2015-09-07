/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "videowrite.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV
#include <opencv2/highgui/highgui_c.h>

struct VideoWriter {
	char *filename;
	CvVideoWriter *out;
	int64_t current_num_frame;
};

static int64_t getCodec(const char *codecName) {
	if (codecName == NULL || my_string_equals_ignorecase(codecName, "NULL"))
		return 0;
	else if (my_string_equals_ignorecase(codecName,
			"PROMPT") || my_string_equals_ignorecase(codecName, "ASK"))
		return CV_FOURCC_PROMPT;
	my_assert_equalInt("length fourcc", strlen(codecName), 4);
	return CV_FOURCC(codecName[0], codecName[1], codecName[2], codecName[3]);
}
VideoWriter *newVideoWriter(const char *newFilename, const char *fileExtension,
		const char *codecName, double fps, IplImage *first_frame) {
	char *fname = my_newString_format("%s.%s", newFilename, fileExtension);
	int64_t c = 0;
	while (my_io_existsFile(fname)) {
		MY_FREE(fname);
		fname = my_newString_format("%s (%"PRIi64").%s", newFilename, c++, fileExtension);
		if (c > 9)
			my_log_error("can't create file %s", newFilename);
	}
	VideoWriter *video_writer = MY_MALLOC(1, VideoWriter);
	video_writer->filename = fname;
	my_log_info("creating video '%s'\n", video_writer->filename);
	video_writer->out = cvCreateVideoWriter(video_writer->filename,
			getCodec(codecName), fps, cvGetSize(first_frame),
			first_frame->nChannels == 1 ? 0 : 1);
	if (video_writer->out == NULL)
		my_log_error("can't create file %s\n", video_writer->filename);
	cvWriteFrame(video_writer->out, first_frame);
	video_writer->current_num_frame = 0;
	return video_writer;
}
void addVideoFrame(VideoWriter *video_writer, IplImage *frame) {
	cvWriteFrame(video_writer->out, frame);
	video_writer->current_num_frame++;
}
int64_t getCurrentFrame(VideoWriter *video_writer) {
	return video_writer->current_num_frame;
}
char *getFilename(VideoWriter *video_writer) {
	return video_writer->filename;
}
void closeVideoWriter(VideoWriter *video_writer) {
	if (video_writer == NULL)
		return;
	if (video_writer->out != NULL) {
		my_log_info("closing video '%s'\n", video_writer->filename);
		cvReleaseVideoWriter(&video_writer->out);
		int64_t size = my_io_getFilesize(video_writer->filename);
		my_log_info("video '%s' recorded ok, %"PRIi64" frames, %1.1lf MB.\n",
				video_writer->filename, video_writer->current_num_frame + 1,
				size / (1024.0 * 1024));
	}
	MY_FREE(video_writer->filename);
	MY_FREE(video_writer);
}
#endif
