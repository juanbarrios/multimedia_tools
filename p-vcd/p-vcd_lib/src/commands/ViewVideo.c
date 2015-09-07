/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

#ifndef NO_OPENCV
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

struct GlobalProcess {
	MyVectorObj *videos;
	int64_t msWaitFrames;
	bool isStopped;
	double seekSizeSeconds, jumpSizeSeconds;
	int64_t numSeeksInBuffer;
	uchar withTimestamp;
};
struct BufferSlot {
	IplImage *image;
	int64_t numFrame;
	double timeFrame;
};
struct InputVideo {
	const char *filename;
	MyVectorString *transfs, *preprocs;
	double timeStart, timeEnd;
	int64_t idInput;
	char *windowName;
	VideoFrame *video_frame;
	MyImageResizer *resizerToScreen;
	int64_t seekSizeFrames;
	uchar hasChangedFrame;
	double fps;
	uchar isRecordingVideo;
	VideoWriter *videoRecorder;
	struct GlobalProcess *global;
	int64_t buffer_position, buffer_distance, buffer_size, buffer_busy_size;
	struct BufferSlot *buffer_prevs;
	struct BufferSlot *current;
};

static void v_saveFrame(struct GlobalProcess *p, int64_t numVideo) {
	if (numVideo >= my_vectorObj_size(p->videos)) {
		my_log_info("can't find video %"PRIi64"\n", numVideo + 1);
		return;
	}
	struct InputVideo *sv = my_vectorObj_get(p->videos, numVideo);
	if (sv == NULL)
		return;
	char *fn = my_newString_format("%s-frame_%"PRIi64".png", sv->filename,
			sv->current->numFrame);
	my_io_removeInvalidChars(fn);
	if (my_io_existsFile(fn))
		my_log_info("overwriting %s\n", fn);
	if (cvSaveImage(fn, sv->current->image, NULL))
		my_log_info("image %s saved (%ix%i)\n", fn, sv->current->image->width,
				sv->current->image->height);
	else
		my_log_info("error saving %s\n", fn);
	MY_FREE(fn);
}
static void v_addStamp(IplImage *frame, char *text) {
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, CV_AA);
	cvRectangle(frame, cvPoint(5, 5), cvPoint(450, 30),
			cvScalar(210, 210, 210, 0),
			CV_FILLED, 8, 0);
	cvPutText(frame, text, cvPoint(5, 30), &font, cvScalar(30, 30, 150, 0));
}

static void v_printHelp(struct GlobalProcess *p) {
	my_log_info(
			"  ESC,Q=quit  <,>=fastest/slower  ENTER=print number  ?=help\n");
	my_log_info(
			"  SPACE=start/stop  {,}=rew/fwd %1.0lf secs  [,]=rew/fwd %1.0lf secs\n",
			p->seekSizeSeconds, p->jumpSizeSeconds);
	my_log_info(
			"  W=start/stop record  +,-=next/prev frame  a,z=start/end video  s,x=resize\n");
	my_log_info("  CTRL-A=SetStart  CTRL-Z=SetEnd  F=ffmpeg  V=vlc\n");
	int64_t numVideos = my_vectorObj_size(p->videos);
	if (numVideos > 0)
		my_log_info("  VIDEO 1: Y=next frame  H=prev frame  1=save frame\n");
	if (numVideos > 1)
		my_log_info("  VIDEO 2: U=next frame  J=prev frame  2=save frame\n");
	if (numVideos > 2)
		my_log_info("  VIDEO 3: I=next frame  K=prev frame  3=save frame\n");
	if (numVideos > 3)
		my_log_info("  VIDEO 4: O=next frame  L=prev frame  4=save frame\n");
	if (numVideos > 4)
		my_log_info("  VIDEO 5: P=next frame  .=prev frame  5=save frame\n");
}

#define ACTION_PRINT_FRAME 10
#define ACTION_QUIT 20

#define ACTION_NEXT_FRAME 40
#define ACTION_PREV_FRAME 50

#define ACTION_FF_FRAMES 60
#define ACTION_RW_FRAMES 70

#define ACTION_JUMP_FF 80
#define ACTION_JUMP_RW 90

#define ACTION_JUMP_TO_START 100
#define ACTION_JUMP_TO_END 110

#define ACTION_SET_START 200
#define ACTION_SET_END 210

#define ACTION_EXEC_FFMPEG 310
#define ACTION_EXEC_VLC 320

#define ACTION_RESIZE_SMALLER 350
#define ACTION_RESIZE_LARGER 360

#define ACTION_RECORD_VIDEO 900

#define ACTION_ON_ALL_WINDOWS -1

struct Action {
	int64_t actionCode, numWindow;
};
static struct Action v_actionAllW(int64_t actionCode) {
	struct Action ac = { actionCode, ACTION_ON_ALL_WINDOWS };
	return ac;
}
static struct Action v_actionOneW(int64_t actionCode, int64_t numWindow) {
	struct Action ac = { actionCode, numWindow };
	return ac;
}
static struct Action v_actionNone() {
	struct Action ac = { -2, -2 };
	return ac;
}

static struct Action v_waitAction(struct GlobalProcess *p) {
	int c = cvWaitKey(p->isStopped ? 0 : p->msWaitFrames);
	if (c == -1)
		return v_actionAllW(ACTION_NEXT_FRAME);
	c &= 0x7F;
	bool wasStopped = p->isStopped;
	p->isStopped = true;
	switch (c) {
	case 27: //esc
	case 113: // 'q'
		return v_actionAllW(ACTION_QUIT);
	case 32: // SPACE BAR
		p->isStopped = !wasStopped;
		return v_actionNone();
	case 60: // '<'
		if (p->msWaitFrames > 1) {
			p->msWaitFrames = 1;
			my_log_info("wait time: %"PRIi64" ms.\n", p->msWaitFrames);
		}
		p->isStopped = wasStopped;
		return v_actionNone();
	case 62: // '>'
		p->msWaitFrames++;
		my_log_info("wait time: %"PRIi64" ms.\n", p->msWaitFrames);
		p->isStopped = wasStopped;
		return v_actionNone();
	case 43: // '+'
		return v_actionAllW(ACTION_NEXT_FRAME);
	case 45: // '-'
		return v_actionAllW(ACTION_PREV_FRAME);
	case 125: // '}'
		return v_actionAllW(ACTION_FF_FRAMES);
	case 123: // '{'
		return v_actionAllW(ACTION_RW_FRAMES);
	case 93: // ']'
		return v_actionAllW(ACTION_JUMP_FF);
	case 91: // '['
		return v_actionAllW(ACTION_JUMP_RW);
	case 121: // 'y'
		return v_actionOneW(ACTION_NEXT_FRAME, 0);
	case 104: // 'h'
		return v_actionOneW(ACTION_PREV_FRAME, 0);
	case 117: // 'u'
		return v_actionOneW(ACTION_NEXT_FRAME, 1);
	case 106: // 'j'
		return v_actionOneW(ACTION_PREV_FRAME, 1);
	case 105: // 'i'
		return v_actionOneW(ACTION_NEXT_FRAME, 2);
	case 107: // 'k'
		return v_actionOneW(ACTION_PREV_FRAME, 2);
	case 111: // 'o'
		return v_actionOneW(ACTION_NEXT_FRAME, 3);
	case 108: // 'l'
		return v_actionOneW(ACTION_PREV_FRAME, 3);
	case 112: // 'p'
		return v_actionOneW(ACTION_NEXT_FRAME, 4);
	case 46: // '.'
		return v_actionOneW(ACTION_PREV_FRAME, 4);
	case 97: // 'a'
		return v_actionAllW(ACTION_JUMP_TO_START);
	case 122: // 'z'
		return v_actionAllW(ACTION_JUMP_TO_END);
	case 1: // 'control-a'
		return v_actionAllW(ACTION_SET_START);
	case 26: // 'control-z'
		return v_actionAllW(ACTION_SET_END);
	case 102: // 'f'
		return v_actionAllW(ACTION_EXEC_FFMPEG);
	case 118: // 'v'
		return v_actionAllW(ACTION_EXEC_VLC);
	case 13: //ENTER
	case 10: //ENTER
		return v_actionAllW(ACTION_PRINT_FRAME);
	case 63: //?
		v_printHelp(p);
		return v_actionNone();
	case 49: // '1'
	case 50: // '2'
	case 51: // '3'
	case 52: // '4'
	case 53: // '5'
		v_saveFrame(p, c - 49);
		return v_actionNone();
	case 115: //'s'
		p->isStopped = false;
		return v_actionAllW(ACTION_RESIZE_SMALLER);
	case 120: //'x'
		p->isStopped = false;
		return v_actionAllW(ACTION_RESIZE_LARGER);
	case 119: //'w'
		p->isStopped = false;
		return v_actionAllW(ACTION_RECORD_VIDEO);
	default:
		my_log_info("unknown key %i (\"%c\"), press \"?\" for help\n", c, c);
		break;
	}
	return v_actionNone();
}
static void v_addCurrentImageToBuffer(struct InputVideo *sv) {
	IplImage *image = getCurrentFrameOrig(sv->video_frame);
	int64_t numFrame = getCurrentNumFrame(sv->video_frame);
	sv->buffer_position = (sv->buffer_position + 1) % sv->buffer_size;
	if (sv->buffer_busy_size < sv->buffer_size)
		sv->buffer_busy_size++;
	struct BufferSlot *bs = sv->buffer_prevs + sv->buffer_position;
	if (bs->image == NULL) {
		bs->image = my_image_duplicate(image);
	} else if (image->width != bs->image->width
			|| image->height != bs->image->height
			|| image->depth != bs->image->depth
			|| image->nChannels != bs->image->nChannels) {
		my_image_release(bs->image);
		bs->image = my_image_duplicate(image);
	} else {
		cvCopy(image, bs->image, NULL);
	}
	bs->numFrame = numFrame;
	if (sv->fps > 0)
		bs->timeFrame = numFrame / sv->fps;
	if (sv->global->withTimestamp) {
		char *text = NULL;
		if (sv->fps > 0) {
			text = my_newString_hhmmssfff(bs->timeFrame);
		} else {
			text = my_timer_currentClock_newString();
		}
		v_addStamp(bs->image, text);
		MY_FREE(text);
	}
	if (sv->isRecordingVideo)
		addVideoFrame(sv->videoRecorder, bs->image);
}
static void v_updateCurrentFrame(struct InputVideo *sv) {
	int64_t idx = (sv->buffer_position - sv->buffer_distance + sv->buffer_size)
			% sv->buffer_size;
	sv->current = sv->buffer_prevs + idx;
	sv->hasChangedFrame = 1;
}
static void v_seekFrames(struct InputVideo *sv, int64_t deltaNumFrames) {
	if (deltaNumFrames < 0) {
		if (sv->buffer_distance - deltaNumFrames < sv->buffer_busy_size) {
			sv->buffer_distance -= deltaNumFrames;
			v_updateCurrentFrame(sv);
		} else {
			my_log_info("first frame at %s\n", sv->filename);
			sv->global->isStopped = 1;
		}
	} else if (deltaNumFrames > 0) {
		if (sv->buffer_distance > 0) {
			int64_t m = MIN(sv->buffer_distance, deltaNumFrames);
			sv->buffer_distance -= m;
			deltaNumFrames -= m;
		}
		while (deltaNumFrames > 0) {
			if (!loadNextFrame(sv->video_frame))
				break;
			v_addCurrentImageToBuffer(sv);
			deltaNumFrames--;
		}
		if (deltaNumFrames > 0) {
			my_log_info("last frame at %s\n", sv->windowName);
			sv->global->isStopped = 1;
		}
		v_updateCurrentFrame(sv);
		if (sv->timeEnd > 0 && sv->current->timeFrame > sv->timeEnd) {
			char *st = my_newString_hhmmss(sv->timeEnd);
			my_log_info("passing end %s\n", st);
			MY_FREE(st);
		}
	}
}
static void v_jumpToTime(struct InputVideo *sv, double absoluteTimeStart) {
	int64_t numFramesJump = my_math_round_int(absoluteTimeStart * sv->fps);
	int64_t numFramesSeek = MIN(numFramesJump, sv->buffer_size);
	numFramesJump -= numFramesSeek;
	if (!seekVideoToFrame(sv->video_frame, numFramesJump)) {
		char *st = my_newString_hhmmssfff(absoluteTimeStart);
		my_log_info("can't jump to %s\n", st);
		MY_FREE(st);
		return;
	}
	//jump invalidates buffer
	sv->buffer_busy_size = sv->buffer_distance = sv->buffer_position = 0;
	if (numFramesSeek > 0) {
		v_seekFrames(sv, numFramesSeek);
	} else {
		v_addCurrentImageToBuffer(sv);
		v_updateCurrentFrame(sv);
	}
}

#define RESIZE_FACTOR 0.75

//avi MJPG 10
//avi XVID 10
//avi IYUV 10
static const char *DEFAULT_OUT_EXTENSION = "mp4";
static const char *DEFAULT_OUT_CODEC = "h264";
static const char *DEFAULT_OUT_FPS = "25";

static void v_processAction(struct InputVideo *sv, struct Action act) {
	if (act.numWindow != ACTION_ON_ALL_WINDOWS && act.numWindow != sv->idInput)
		return;
	if (act.actionCode == ACTION_NEXT_FRAME) {
		v_seekFrames(sv, 1);
	} else if (act.actionCode == ACTION_FF_FRAMES) {
		v_seekFrames(sv, sv->seekSizeFrames);
	} else if (act.actionCode == ACTION_PREV_FRAME) {
		v_seekFrames(sv, -1);
	} else if (act.actionCode == ACTION_RW_FRAMES) {
		v_seekFrames(sv, -sv->seekSizeFrames);
	} else if (act.actionCode == ACTION_JUMP_TO_START) {
		v_jumpToTime(sv, sv->timeStart);
	} else if (act.actionCode == ACTION_JUMP_TO_END) {
		v_jumpToTime(sv, sv->timeEnd);
	} else if (act.actionCode == ACTION_JUMP_FF) {
		double tt = sv->current->timeFrame + sv->global->jumpSizeSeconds;
		v_jumpToTime(sv, tt);
	} else if (act.actionCode == ACTION_JUMP_RW) {
		double tt = MAX(0,
				sv->current->timeFrame - sv->global->jumpSizeSeconds);
		v_jumpToTime(sv, tt);
	}
	if (act.actionCode == ACTION_RESIZE_SMALLER
			|| act.actionCode == ACTION_RESIZE_LARGER) {
		IplImage *scaled = my_imageResizer_resizeImage(sv->current->image,
				sv->resizerToScreen);
		int64_t w = scaled->width;
		int64_t h = scaled->height;
		double factor =
				(act.actionCode == ACTION_RESIZE_SMALLER) ?
						RESIZE_FACTOR : (1.0 / RESIZE_FACTOR);
		w = my_math_round_int(w * factor);
		h = my_math_round_int(h * factor);
		my_imageResizer_release(sv->resizerToScreen);
		sv->resizerToScreen = my_imageResizer_newFixedSize(w, h);
		sv->hasChangedFrame = 1;
	}
	if (sv->hasChangedFrame) {
		IplImage *scaled = my_imageResizer_resizeImage(sv->current->image,
				sv->resizerToScreen);
		cvShowImage(sv->windowName, scaled);
	}
	if (act.actionCode == ACTION_PRINT_FRAME) {
		struct BufferSlot *bs = sv->current;
		if (sv->fps > 0) {
			char *stTime = my_newString_hhmmssfff(bs->timeFrame);
			my_log_info("%s #%"PRIi64", %ix%i, %5.2lf fps, %s\n", sv->filename,
					bs->numFrame, bs->image->width, bs->image->height, sv->fps,
					stTime);
			MY_FREE(stTime);
		} else {
			my_log_info("%s (%ix%i)\n", sv->filename, bs->image->width,
					bs->image->height);
		}
	} else if (act.actionCode == ACTION_RECORD_VIDEO) {
		if (sv->isRecordingVideo) {
			sv->isRecordingVideo = 0;
		} else {
			sv->isRecordingVideo = 1;
			if (sv->videoRecorder == NULL) {
				sv->videoRecorder = newVideoWriter(sv->filename,
						DEFAULT_OUT_EXTENSION, DEFAULT_OUT_CODEC,
						(sv->fps > 0) ?
								sv->fps : my_parse_fraction(DEFAULT_OUT_FPS),
						sv->current->image);
			}
		}
	} else if (act.actionCode == ACTION_SET_START) {
		double tt = MAX(0, sv->current->timeFrame - (1 / sv->fps / 3));
		char *stTime = my_newString_hhmmssfff(tt);
		sv->timeStart = tt;
		my_log_info("set start time to %s (length=%1.1lf seconds)\n", stTime,
				sv->timeEnd - sv->timeStart);
		MY_FREE(stTime);
	} else if (act.actionCode == ACTION_SET_END) {
		double tt = sv->current->timeFrame + (1 / sv->fps / 3);
		char *stTime = my_newString_hhmmssfff(tt);
		sv->timeEnd = tt;
		my_log_info("set end time to %s (length=%1.1lf seconds)\n", stTime,
				sv->timeEnd - sv->timeStart);
		MY_FREE(stTime);
	} else if (act.actionCode == ACTION_EXEC_FFMPEG) {
		char *nam = my_subStringC_startLast(sv->filename, '.');
		char *newname = my_newString_format("%s_%1.0lf_%1.0lf", nam,
				sv->timeStart, sv->timeEnd);
		extractVideoSegment(sv->video_frame, sv->timeStart, sv->timeEnd,
				newname);
		MY_FREE_MULTI(nam, newname);
	} else if (act.actionCode == ACTION_EXEC_VLC) {
		char *stLength = my_newString_doubleDec(sv->timeEnd - sv->timeStart, 3);
		char *command = my_newString_format(
				"\"%s\" --start-time=%1.3lf --run-time=%s --play-and-pause",
				sv->filename, sv->timeStart, stLength);
		my_log_info("vlc %s\n", command);
		pvcd_system_call_vlc(command);
		free(command);
		free(stLength);
	}
}
static void v_parseStartTime(struct InputVideo *sv) {
	if (my_string_indexOf(sv->filename, "@") > 0) {
		char *nn = my_subStringC_lastEnd(sv->filename, '@');
		char *tt = my_subStringC_lastEnd(sv->filename, '@');
		if (sv->timeStart == 0)
			sv->timeStart = my_parse_seconds(tt);
		free(tt);
		sv->filename = nn;
	}
}
static void v_openVideo(struct InputVideo *sv) {
	if (my_string_indexOf(sv->filename, "#") > 0) {
		char *nameDB = my_subStringC_startLast(sv->filename, '#');
		char *idFile = my_subStringC_lastEnd(sv->filename, '#');
		FileDB *fdb = loadFileDB(nameDB, idFile, true);
		sv->filename = idFile;
		sv->video_frame = openFileDB(fdb, true);
		free(nameDB);
	} else {
		sv->video_frame = openFile(sv->filename, 1);
	}
	sv->fps = getFpsVideo(sv->video_frame);
	sv->seekSizeFrames = my_math_round_int(
			sv->fps * sv->global->seekSizeSeconds);
}
static void v_firstJump(struct InputVideo *sv) {
	loadNextFrame(sv->video_frame);/*
	 if (getIsVideo(sv->video_frame)) {
	 double videoTimeStart = getVideoTimeStart(sv->video_frame);
	 double videoTimeEnd = getVideoTimeEnd(sv->video_frame);
	 if ((sv->timeStart != 0 && videoTimeStart != 0 && videoTimeEnd != 0)
	 && !MY_BETWEEN(sv->timeStart, videoTimeStart, videoTimeEnd)) {
	 my_log_error("invalid time start %1.3lf (not in %1.3lf - %1.3lf)",
	 sv->timeStart, videoTimeStart, videoTimeEnd);
	 } else if ((sv->timeEnd != 0 && videoTimeStart != 0 && videoTimeEnd != 0)
	 && !MY_BETWEEN(sv->timeEnd, videoTimeStart, videoTimeEnd)) {
	 my_log_error("invalid time end %1.3lf (not in %1.3lf - %1.3lf)",
	 sv->timeEnd, videoTimeStart, videoTimeEnd);
	 }
	 if (sv->timeStart == 0 && videoTimeStart != 0)
	 sv->timeStart = videoTimeStart;
	 if (sv->timeEnd == 0 && videoTimeEnd != 0)
	 sv->timeEnd = videoTimeEnd;
	 if (sv->timeStart > 0)
	 v_jumpToTime(sv, sv->timeStart);
	 char *st = my_newString_hhmmssfff(sv->timeEnd - sv->timeStart);
	 my_log_info("open %s length=%s\n", sv->filename, st);
	 free(st);
	 } else {
	 v_jumpToTime(sv, 0);
	 }*/
}
static void v_openInputFile(struct InputVideo *sv) {
	v_parseStartTime(sv);
	v_openVideo(sv);
	for (int64_t i = 0; i < my_vectorString_size(sv->transfs); ++i) {
		const char *transf = my_vectorString_get(sv->transfs, i);
		const char *preproceso = my_vectorString_get(sv->preprocs, i);
		if (preproceso == NULL)
			addTransformation(sv->video_frame, transf);
		else
			addTransformationPreprocessed(sv->video_frame, transf, preproceso);
	}
	sv->windowName = my_newString_format("V%"PRIi64": %s", sv->idInput + 1,
			sv->filename);
	sv->resizerToScreen = my_imageResizer_newScreenSize();
	sv->buffer_size = sv->global->numSeeksInBuffer * sv->seekSizeFrames + 1;
	sv->buffer_prevs = MY_MALLOC(sv->buffer_size, struct BufferSlot);
	v_firstJump(sv);
}
static void v_closeVideo(struct InputVideo *sv) {
	cvDestroyWindow(sv->windowName);
	if (sv->videoRecorder != NULL)
		closeVideoWriter(sv->videoRecorder);
	closeVideo(sv->video_frame);
	if (sv->buffer_size > 0) {
		int64_t i;
		for (i = 0; i < sv->buffer_size; ++i)
			my_image_release(sv->buffer_prevs[i].image);
		MY_FREE(sv->buffer_prevs);
	}
}

static void v_processVideos(struct GlobalProcess *global) {
	my_assert_greaterInt("numVideos", my_vectorObj_size(global->videos), 0);
	int64_t k;
	for (k = 0; k < my_vectorObj_size(global->videos); ++k) {
		struct InputVideo *sv = my_vectorObj_get(global->videos, k);
		v_openInputFile(sv);
	}
	if (global->msWaitFrames < 1) {
		struct InputVideo *sv = my_vectorObj_get(global->videos, 0);
		global->msWaitFrames =
				(sv->fps > 0) ? my_math_round_int(1000 / sv->fps * 0.75) : 1;
		global->msWaitFrames = MAX(1, global->msWaitFrames);
	}
	for (k = 0; k < my_vectorObj_size(global->videos); ++k) {
		struct InputVideo *sv = my_vectorObj_get(global->videos, k);
		cvNamedWindow(sv->windowName, CV_WINDOW_AUTOSIZE);
	}
	v_printHelp(global);
	struct Action act = v_actionAllW(ACTION_NEXT_FRAME);
	do {
		for (k = 0; k < my_vectorObj_size(global->videos); ++k) {
			struct InputVideo *sv = my_vectorObj_get(global->videos, k);
			v_processAction(sv, act);
		}
		act = v_waitAction(global);
	} while (act.actionCode != ACTION_QUIT);
	for (k = 0; k < my_vectorObj_size(global->videos); ++k) {
		struct InputVideo *sv = my_vectorObj_get(global->videos, k);
		v_closeVideo(sv);
	}
}
#endif

#define DEFAULT_SEGS_SEEK "5"

#define DEFAULT_SEGS_JUMP "1"
#define DEFAULT_NUM_SEEKS_IN_BUFFER "1"
//#define DEFAULT_SEGS_JUMP "300"
//#define DEFAULT_NUM_SEEKS_IN_BUFFER "7"

int pvcd_viewImageVideos(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s\n", getBinaryName(cmd_params));
		my_log_info("  filename\n");
		my_log_info(
				"    [-tr transf  [-pr preprocess]]+     Optional. One or many.\n");
		my_log_info("    [-start mm:ss.xxx]                  Optional.\n");
		my_log_info("    [-end   mm:ss.xxx]                  Optional.\n");
		my_log_info(
				"    [-wait ms]                           default=0 (1/fps)\n");
		my_log_info(
				"    [-jumpSize secs]                    default=%s secs (long unbuffered jump)\n",
				DEFAULT_SEGS_JUMP);
		my_log_info(
				"    [-seekSize secs]                    default=%s secs (small buffered jump)\n",
				DEFAULT_SEGS_SEEK);
		my_log_info(
				"    [-numSeeksInBuffer num]             default=%s (buffer size)\n",
				DEFAULT_NUM_SEEKS_IN_BUFFER);
		my_log_info("    [-outVideo ext codec fps]          default=%s %s %s\n",
				DEFAULT_OUT_EXTENSION, DEFAULT_OUT_CODEC, DEFAULT_OUT_FPS);
		my_log_info("    [-withTimestamp]\n");
		my_log_info("%s -show\n", getBinaryName(cmd_params));
		return pvcd_system_exit_error();
	}
	struct GlobalProcess p = { 0 };
	struct InputVideo *currentVideo = NULL;
	p.jumpSizeSeconds = my_parse_double(DEFAULT_SEGS_JUMP);
	p.seekSizeSeconds = my_parse_double(DEFAULT_SEGS_SEEK);
	p.numSeeksInBuffer = my_parse_int(DEFAULT_NUM_SEEKS_IN_BUFFER);
	p.videos = my_vectorObj_new();
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-show")) {
			pvcd_print_transformations();
			return pvcd_system_exit_error();
		} else if (isNextParam(cmd_params, "-wait")) {
			p.msWaitFrames = nextParamInt(cmd_params);
			if (p.msWaitFrames < 1)
				p.isStopped = 1;
		} else if (isNextParam(cmd_params, "-jumpSize")) {
			p.jumpSizeSeconds = nextParamDouble(cmd_params);
		} else if (isNextParam(cmd_params, "-seekSize")) {
			p.seekSizeSeconds = nextParamDouble(cmd_params);
		} else if (isNextParam(cmd_params, "-numSeeksInBuffer")) {
			p.numSeeksInBuffer = nextParamInt(cmd_params);
		} else if (isNextParam(cmd_params, "-tr")) {
			const char *transf = nextParam(cmd_params);
			const char *preproceso = NULL;
			if (isNextParam(cmd_params, "-pr"))
				preproceso = nextParam(cmd_params);
			my_assert_notNull("filename to transform", currentVideo);
			my_vectorStringConst_add(currentVideo->transfs, transf);
			my_vectorStringConst_add(currentVideo->preprocs, preproceso);
		} else if (isNextParam(cmd_params, "-start")) {
			my_assert_notNull("filename", currentVideo);
			currentVideo->timeStart = nextParamTime(cmd_params);
		} else if (isNextParam(cmd_params, "-end")) {
			my_assert_notNull("filename", currentVideo);
			currentVideo->timeEnd = nextParamTime(cmd_params);
		} else if (isNextParam(cmd_params, "-outVideo")) {
			DEFAULT_OUT_EXTENSION = nextParam(cmd_params);
			DEFAULT_OUT_CODEC = nextParam(cmd_params);
			DEFAULT_OUT_FPS = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-withTimestamp")) {
			p.withTimestamp = 1;
		} else {
			const char *filename = nextParam(cmd_params);
			currentVideo = MY_MALLOC(1, struct InputVideo);
			currentVideo->filename = filename;
			currentVideo->transfs = my_vectorString_new();
			currentVideo->preprocs = my_vectorString_new();
			currentVideo->global = &p;
			currentVideo->idInput = my_vectorObj_size(p.videos);
			my_vectorObj_add(p.videos, currentVideo);
		}
	}
	if (my_vectorObj_size(p.videos) == 0)
		my_log_error("filename is mandatory\n");
	if (my_vectorObj_size(p.videos) > 10)
		my_log_error("too many files\n");
	v_processVideos(&p);
#endif
	return pvcd_system_exit_ok(cmd_params);
}

