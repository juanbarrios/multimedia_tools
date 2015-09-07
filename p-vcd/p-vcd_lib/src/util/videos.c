/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "videos.h"

#ifndef NO_OPENCV
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

struct V_Layer0_image {
	IplImage *image;
	bool isShown;
};
struct V_Layer0_video {
	CvCapture* video;
	double timeStart, timeEnd, fps;
	int64_t physicalStartFrame, physicalEndFrame;
	int64_t currentPhysicalFrame;
};
struct V_Layer0_camera {
	CvCapture* webcam;
	int64_t currentFrame;
};
struct V_Layer0_audio {
	double timeStart, timeEnd, sampleRate;
};
struct V_Layer0_opencv {
	bool isImage, isVideo, isAudio, isWebcam;
	char *file_full_path;
	struct V_Layer0_image dImage;
	struct V_Layer0_video dVideo;
	struct V_Layer0_camera dCamera;
	struct V_Layer0_audio dAudio;
};
struct V_Layer1_transformations {
	//a list of Transform*
	MyVectorObj *transform;
	IplImage *imagen_base;
	struct V_Layer0_opencv *layer0;
};
struct V_Layer3_conversions {
	bool flag_new_original, flag_must_convert;
	IplImage *img_original, *img_converted;
	MyImageColor *converter;
	struct V_Layer1_transformations *layer1;
};
struct VideoFrame {
	char *name;
	FileDB *fdb;
	struct V_Layer3_conversions *layer3;
};

MY_MUTEX_NEWSTATIC(video_open_mutex);

static struct V_Layer0_opencv *int_new_layer0_img(const char *imagefilename) {
	char *inputFilename = my_io_normalizeFilenameToRead(imagefilename);
	IplImage *img = NULL;
	if (my_string_endsWith_ignorecase(inputFilename, ".pgm")) {
		img = my_image_pgm_loadImage(inputFilename);
	} else {
		MY_MUTEX_LOCK(video_open_mutex);
		img = cvLoadImage(inputFilename, CV_LOAD_IMAGE_UNCHANGED);
		MY_MUTEX_UNLOCK(video_open_mutex);
	}
	if (img == NULL) {
		free(inputFilename);
		return NULL;
	}
	struct V_Layer0_opencv *v = MY_MALLOC(1, struct V_Layer0_opencv);
	v->isImage = true;
	v->file_full_path = inputFilename;
	v->dImage.isShown = false;
	v->dImage.image = img;
	return v;
}
static struct V_Layer0_opencv *int_new_layer0_audio(const char *audiofilename,
		double timeStart, double timeEnd, double sampleRate) {
	struct V_Layer0_opencv *v = MY_MALLOC(1, struct V_Layer0_opencv);
	v->isAudio = true;
	v->file_full_path = my_io_normalizeFilenameToRead(audiofilename);
	v->dAudio.timeStart = timeStart;
	v->dAudio.timeEnd = timeEnd;
	v->dAudio.sampleRate = sampleRate;
	return v;
}

#define MAX_DISTANCE_TO_SEEK_BY_FRAME 300
#define NUMFRAME_NOT_OPENED -1

static bool int_forwardVideoFrameByFrame(struct V_Layer0_opencv *v,
		int64_t numFrames) {
	for (int64_t i = 0; i < numFrames; ++i) {
		if (!cvGrabFrame(v->dVideo.video))
			return false;
		if (v->dVideo.currentPhysicalFrame == NUMFRAME_NOT_OPENED)
			v->dVideo.currentPhysicalFrame = 0;
		else
			v->dVideo.currentPhysicalFrame += 1;
	}
	return true;
}

static bool int_seekVideoToNumFrame(struct V_Layer0_opencv *v, int64_t numFrame) {
	my_assert_isTrue("isVideo", v->isVideo);
	my_assert_greaterEqual_int("numFrame", numFrame, 0);
	if (numFrame < v->dVideo.physicalStartFrame
			|| (v->dVideo.physicalEndFrame > 0
					&& numFrame >= v->dVideo.physicalEndFrame))
		return false;
	int64_t diff =
			(v->dVideo.currentPhysicalFrame == NUMFRAME_NOT_OPENED) ?
					numFrame + 1 : numFrame - v->dVideo.currentPhysicalFrame;
	if (MY_BETWEEN(diff, 0, MAX_DISTANCE_TO_SEEK_BY_FRAME)) {
		bool ok = int_forwardVideoFrameByFrame(v, diff);
		if (!ok) {
			my_log_info(
					"WARNING: premature end of %s (real end at %"PRIi64" frames)\n",
					v->file_full_path, v->dVideo.currentPhysicalFrame);
			v->dVideo.physicalEndFrame = v->dVideo.currentPhysicalFrame;
			return false;
		}
		return ok;
	}
	int ok = cvSetCaptureProperty(v->dVideo.video, CV_CAP_PROP_POS_FRAMES,
			numFrame);
	my_assert_notEqualInt("cvSetCaptureProperty", ok, 0);
	int64_t real_pos = (int64_t) cvGetCaptureProperty(v->dVideo.video,
			CV_CAP_PROP_POS_FRAMES);
	v->dVideo.currentPhysicalFrame = real_pos;
	if (real_pos == numFrame)
		return true;
	else
		return false;
}
static struct V_Layer0_opencv *int_new_layer0_vid(const char *videofilename,
		double timeStart, double timeEnd, double fps) {
	char *inputFilename = my_io_normalizeFilenameToRead(videofilename);
	char *name = my_io_getFilename(inputFilename);
	my_log_info("opening %s\n", name);
	free(name);
	MY_MUTEX_LOCK(video_open_mutex);
	CvCapture *cap = cvCreateFileCapture(inputFilename);
	MY_MUTEX_UNLOCK(video_open_mutex);
	if (cap == NULL) {
		free(inputFilename);
		return NULL;
	}
	if (fps == 0) {
		fps = cvGetCaptureProperty(cap, CV_CAP_PROP_FPS);
		my_assert_greaterDouble("fps", fps, 0);
	}
	if (timeEnd == 0) {
		cvSetCaptureProperty(cap, CV_CAP_PROP_POS_AVI_RATIO, 1.0);
		double val = cvGetCaptureProperty(cap, CV_CAP_PROP_POS_FRAMES);
		timeEnd = val / fps;
		cvSetCaptureProperty(cap, CV_CAP_PROP_POS_AVI_RATIO, 0);
	}
	struct V_Layer0_opencv *v = MY_MALLOC(1, struct V_Layer0_opencv);
	v->isVideo = true;
	v->file_full_path = inputFilename;
	v->dVideo.timeStart = timeStart;
	v->dVideo.timeEnd = timeEnd;
	v->dVideo.fps = fps;
	v->dVideo.video = cap;
	v->dVideo.physicalStartFrame = my_math_floor_int(timeStart * v->dVideo.fps);
	v->dVideo.physicalEndFrame = my_math_round_int(timeEnd * v->dVideo.fps);
	v->dVideo.currentPhysicalFrame = NUMFRAME_NOT_OPENED;
	return v;
}

#define CAMERA_NAME "webcam"
#define WEBCAM_INIT_FRAMES 10
#define WEBCAM_INIT_WIDTH 1280
#define WEBCAM_INIT_HEIGHT 720

static struct V_Layer0_opencv *int_new_layer0_webcam(const char *cameraFilename) {
	struct V_Layer0_opencv *v = MY_MALLOC(1, struct V_Layer0_opencv);
	v->isWebcam = true;
	v->file_full_path = my_newString_string(CAMERA_NAME);
	v->dCamera.currentFrame = -1;
	int64_t cameraId =
			my_string_equals(cameraFilename, CAMERA_NAME) ?
					0 :
					my_parse_int_isubFromEnd(cameraFilename,
							strlen(CAMERA_NAME));
	MY_MUTEX_LOCK(video_open_mutex);
	my_log_info("opening %s...\n", cameraFilename);
	CvCapture *cap = cvCreateCameraCapture(cameraId);
	MY_MUTEX_UNLOCK(video_open_mutex);
	if (cap == NULL) {
		MY_FREE(v);
		return NULL;
	}
	//fps = cvGetCaptureProperty(cap, CV_CAP_PROP_FPS);
	//cvSetCaptureProperty(v->video, CV_CAP_PROP_FRAME_WIDTH, WEBCAM_INIT_WIDTH);
	//cvSetCaptureProperty(v->video, CV_CAP_PROP_FRAME_HEIGHT,	WEBCAM_INIT_HEIGHT);
	my_log_info("%lf fps %lf x %lf\n",
			cvGetCaptureProperty(cap, CV_CAP_PROP_FPS),
			cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH),
			cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT));
	//cvSetCaptureProperty(v->video, CV_CAP_PROP_FPS, 30);
	//padding to initialize webcam
	for (int64_t i = 0; i < WEBCAM_INIT_FRAMES; ++i)
		cvQueryFrame(cap);
	v->dCamera.webcam = cap;
	return v;
}
static struct V_Layer0_opencv *int_new_layer0_fdb(FileDB *fdb) {
	if (fdb == NULL || !my_io_existsFile(fdb->pathReal))
		return NULL;
	struct V_Layer0_opencv *v = NULL;
	if (fdb->isImage) {
		v = int_new_layer0_img(fdb->pathReal);
	} else if (fdb->isAudio) {
		v = int_new_layer0_audio(fdb->pathReal, fdb->secStartTime,
				fdb->secEndTime, fdb->numObjsPerSecond);
	} else if (fdb->isVideo) {
		v = int_new_layer0_vid(fdb->pathReal, fdb->secStartTime,
				fdb->secEndTime, fdb->numObjsPerSecond);
	} else {
		my_log_error("unknown type %s\n", fdb->pathReal);
	}
	if (v == NULL)
		return NULL;
	return v;
}
static struct V_Layer1_transformations *int_new_layer1(
		struct V_Layer0_opencv *capa0) {
	struct V_Layer1_transformations *v = MY_MALLOC(1,
			struct V_Layer1_transformations);
	v->layer0 = capa0;
	v->transform = my_vectorObj_new();
	return v;
}
static struct V_Layer1_transformations *int_new_layer1_fdb(
		struct V_Layer0_opencv *capa0, FileDB *fdb) {
	struct V_Layer1_transformations *v = int_new_layer1(capa0);
	for (int64_t i = 0; i < fdb->numTransfs; ++i) {
		Transform *tr = findTransform(fdb->nameTransf[i]);
		if (tr->mustPreprocessCompute) {
			tr->preprocessSavedData = my_newString_string(
					fdb->preprocessDataTransf[i]);
			tr->mustPreprocessCompute = 0;
		}
		if (tr->mustPreprocessLoad) {
			tr->def->func_preprocess_load(tr->state, tr->preprocessSavedData);
			tr->mustPreprocessLoad = 0;
		}
		my_vectorObj_add(v->transform, tr);
	}
	return v;
}
static struct V_Layer3_conversions *int_new_layer3(
		struct V_Layer1_transformations *capa1) {
	struct V_Layer3_conversions *v = MY_MALLOC(1, struct V_Layer3_conversions);
	v->flag_new_original = true;
	v->flag_must_convert = true;
	v->converter = my_imageColor_newConverterToGray();
	v->layer1 = capa1;
	return v;
}
/**/
static void int_release_layer0(struct V_Layer0_opencv *v) {
	if (v->isVideo) {
		char *name = my_io_getFilename(v->file_full_path);
		my_log_info("closing %s\n", name);
		MY_FREE(name);
		MY_MUTEX_LOCK(video_open_mutex);
		my_assert_notNull("video", v->dVideo.video);
		cvReleaseCapture(&v->dVideo.video);
		MY_MUTEX_UNLOCK(video_open_mutex);
	} else if (v->isWebcam) {
		MY_MUTEX_LOCK(video_open_mutex);
		my_assert_notNull("video", v->dCamera.webcam);
		cvReleaseCapture(&v->dCamera.webcam);
		MY_MUTEX_UNLOCK(video_open_mutex);
	} else if (v->isImage) {
		my_image_release(v->dImage.image);
	}
	MY_FREE(v->file_full_path);
	MY_FREE(v);
}
static void int_release_layer1(struct V_Layer1_transformations *v) {
	int_release_layer0(v->layer0);
	for (int64_t i = 0; i < my_vectorObj_size(v->transform); ++i) {
		Transform *tr = my_vectorObj_get(v->transform, i);
		releaseTransform(tr);
	}
	my_vectorObj_release(v->transform, 0);
	my_image_release(v->imagen_base);
	MY_FREE(v);
}
static void int_release_layer3(struct V_Layer3_conversions *v) {
	int_release_layer1(v->layer1);
	my_imageColor_release(v->converter);
	MY_FREE(v);
}
/**/
static bool seekVideoToFrame_layer0(struct V_Layer0_opencv *v,
		int64_t desiredFrame) {
	my_assert_isFalse("isAudio", v->isAudio);
	my_assert_greaterEqual_int("desiredFrame", desiredFrame, 0);
	if (v->isImage) {
		if (!v->dImage.isShown) {
			v->dImage.isShown = true;
			return true;
		}
	} else if (v->isWebcam) {
		if (desiredFrame < v->dCamera.currentFrame)
			my_log_error("can't seek backwards\n");
		while (v->dCamera.currentFrame < desiredFrame) {
			if (!cvGrabFrame(v->dCamera.webcam))
				return false;
			v->dCamera.currentFrame++;
		}
		return true;
	} else if (v->isVideo) {
		return int_seekVideoToNumFrame(v, desiredFrame);
	}
	return false;
}
static bool seekVideoToFrame_layer3(struct V_Layer3_conversions *v,
		int64_t desiredFrame) {
	bool ret = seekVideoToFrame_layer0(v->layer1->layer0, desiredFrame);
	if (ret && !v->flag_new_original) {
		v->flag_new_original = true;
		v->flag_must_convert = true;
	}
	return ret;
}
/**/
static int64_t getCurrentNumFrame_layer0(struct V_Layer0_opencv *v) {
	my_assert_isFalse("isAudio", v->isAudio);
	if (v->isVideo)
		return v->dVideo.currentPhysicalFrame;
	else if (v->isWebcam)
		return v->dCamera.currentFrame;
	else if (v->isImage)
		return v->dImage.isShown ? 0 : NUMFRAME_NOT_OPENED;
	my_log_error("file %s without frames\n", v->file_full_path);
	return 0;
}
/**/
static IplImage *getFrame_layer0(struct V_Layer0_opencv *v) {
	my_assert_isFalse("isAudio", v->isAudio);
	if (getCurrentNumFrame_layer0(v) == NUMFRAME_NOT_OPENED)
		seekVideoToFrame_layer0(v, 0);
	if (v->isVideo) {
		return cvRetrieveFrame(v->dVideo.video, 0);
	} else if (v->isWebcam) {
		return cvRetrieveFrame(v->dCamera.webcam, 0);
	} else if (v->isImage) {
		return v->dImage.image;
	}
	my_log_error("file %s without frames\n", v->file_full_path);
	return NULL;
}
static IplImage *getFrame_layer1(struct V_Layer1_transformations *v) {
	if (my_vectorObj_size(v->transform) == 0)
		return getFrame_layer0(v->layer0);
	IplImage *frame = getFrame_layer0(v->layer0);
	if (v->imagen_base == NULL)
		v->imagen_base = cvCreateImage(cvGetSize(frame), frame->depth,
				frame->nChannels);
	my_assert_equalInt("frame width", v->imagen_base->width, frame->width);
	my_assert_equalInt("frame height", v->imagen_base->height, frame->height);
	my_assert_equalInt("frame depth", v->imagen_base->depth, frame->depth);
	my_assert_equalInt("frame nChannels", v->imagen_base->nChannels,
			frame->nChannels);
	cvCopy(frame, v->imagen_base, NULL);
	IplImage *imagen = v->imagen_base;
	for (int64_t i = 0; i < my_vectorObj_size(v->transform); ++i) {
		Transform *tr = my_vectorObj_get(v->transform, i);
		imagen = tr->def->func_transform_frame(imagen,
				getCurrentNumFrame_layer0(v->layer0), tr->state);
	}
	return imagen;
}
static IplImage *getFrame_layer3_orig(struct V_Layer3_conversions *v) {
	if (v->flag_new_original) {
		v->img_original = getFrame_layer1(v->layer1);
		v->flag_new_original = false;
		v->flag_must_convert = true;
	}
	return v->img_original;
}
static IplImage *getFrame_layer3_gray(struct V_Layer3_conversions *v) {
	if (v->flag_must_convert) {
		IplImage *orig = getFrame_layer3_orig(v);
		v->img_converted = my_imageColor_convertFromBGR(orig, v->converter);
		v->flag_must_convert = false;
	}
	return v->img_converted;
}
static VideoFrame *openFileDB_internal(FileDB *fdb) {
	struct V_Layer0_opencv *c0 = int_new_layer0_fdb(fdb);
	if (c0 == NULL)
		return NULL;
	VideoFrame *v = MY_MALLOC(1, VideoFrame);
	v->layer3 = int_new_layer3(int_new_layer1_fdb(c0, fdb));
	v->fdb = fdb;
	v->name = my_newString_string(fdb->id);
	return v;
}

static VideoFrame *openFileName_internal(const char *fname) {
	if (!my_string_startsWith(fname, CAMERA_NAME) && !my_io_existsFile(fname))
		return NULL;
	struct V_Layer0_opencv *c0 = NULL;
	if (is_audio_ext(fname)) {
		c0 = int_new_layer0_audio(fname, 0, 0, 0);
	} else if (is_image_ext(fname)) {
		c0 = int_new_layer0_img(fname);
	} else if (is_video_ext(fname)) {
		c0 = int_new_layer0_vid(fname, 0, 0, 0);
	} else if (my_string_startsWith(fname, CAMERA_NAME)) {
		c0 = int_new_layer0_webcam(fname);
	} else {
		my_log_info(
				"unknown filetype %s, use -setAudioFileExtensions -setImageFileExtensions, -setVideoFileExtensions...\n",
				fname);
		return NULL;
	}
	if (c0 == NULL)
		return NULL;
	VideoFrame *v = MY_MALLOC(1, VideoFrame);
	v->layer3 = int_new_layer3(int_new_layer1(c0));
	char *s1 = my_io_getFilename(fname);
	v->name = my_subStringC_startLast(s1, '.');
	MY_FREE(s1);
	return v;
}
/************************************************************************/
VideoFrame *openFile(const char *filename, bool fail) {
	VideoFrame *v = openFileName_internal(filename);
	if (v == NULL && fail)
		my_log_error("can't open %s\n", filename);
	return v;
}
VideoFrame *openFileDB(FileDB *fdb, bool fail) {
	if (fdb == NULL) {
		if (fail)
			my_log_error("can't open file NULL\n");
		else
			return NULL;
	}
	VideoFrame *v = openFileDB_internal(fdb);
	if (v == NULL && fail)
		my_log_error("can't open %s (%s)\n", fdb->id, fdb->pathReal);
	return v;
}
void closeVideo(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return;
	int_release_layer3(video_frame->layer3);
	MY_FREE(video_frame->name);
	MY_FREE(video_frame);
}
/**************************************************************************/
FileDB *getNewFileDB(VideoFrame *video_frame) {
	struct V_Layer0_opencv *c0 = video_frame->layer3->layer1->layer0;
	FileDB *fdb = MY_MALLOC(1, FileDB);
	fdb->isVideo = c0->isVideo;
	fdb->isAudio = c0->isAudio;
	fdb->isImage = c0->isImage;
	fdb->pathReal = my_io_getAbsolutPath(c0->file_full_path);
	fdb->filenameReal = my_io_getFilename(fdb->pathReal);
	fdb->id = my_newString_string(video_frame->name);
	fdb->filesize = my_io_getFilesize(fdb->pathReal);
	if (c0->isVideo) {
		CvSize size = getFrameSize(video_frame);
		fdb->width = size.width;
		fdb->height = size.height;
		fdb->secStartTime = c0->dVideo.timeStart;
		fdb->secEndTime = c0->dVideo.timeEnd;
		fdb->numObjsPerSecond = c0->dVideo.fps;
	} else if (c0->isImage) {
		CvSize size = getFrameSize(video_frame);
		fdb->width = size.width;
		fdb->height = size.height;
	} else if (c0->isAudio) {
		fdb->secStartTime = c0->dAudio.timeStart;
		fdb->secEndTime = c0->dAudio.timeEnd;
		fdb->numObjsPerSecond = c0->dAudio.sampleRate;
	} else {
		my_log_error("cannot create a file db\n");
	}
	fdb->lengthSec = fdb->secEndTime - fdb->secStartTime;
	if (my_vectorObj_size(video_frame->layer3->layer1->transform) > 0) {
		fdb->numTransfs = my_vectorObj_size(
				video_frame->layer3->layer1->transform);
		fdb->nameTransf = MY_MALLOC(fdb->numTransfs, char*);
		fdb->preprocessDataTransf = MY_MALLOC(fdb->numTransfs, char*);
		for (int64_t i = 0; i < fdb->numTransfs; ++i) {
			Transform *tr = my_vectorObj_get(
					video_frame->layer3->layer1->transform, i);
			fdb->nameTransf[i] = my_newString_string(tr->codeAndParameters);
			fdb->preprocessDataTransf[i] = my_newString_string(
					tr->preprocessSavedData);
		}
	}
	return fdb;
}
static VideoFrame *reopenVideo(VideoFrame *video_frame) {
	VideoFrame *vtemp;
	if (video_frame->fdb != NULL) {
		vtemp = openFileDB(video_frame->fdb, 1);
	} else {
		vtemp = openFile(video_frame->layer3->layer1->layer0->file_full_path,
				1);
	}
	MY_FREE(vtemp->name);
	vtemp->name = my_newString_string(video_frame->name);
	return vtemp;
}
static uchar myAddTransformation(VideoFrame *video_frame, const char *trName,
bool wasPreprocessed, const char *preprocessSavedData) {
	Transform *tr = findTransform(trName);
	if (tr->mustPreprocessCompute) {
		if (wasPreprocessed) {
			tr->preprocessSavedData = my_newString_string(preprocessSavedData);
		} else {
			my_log_info("starting preprocess %s on %s\n", tr->codeAndParameters,
					video_frame->name);
			VideoFrame *vtemp = reopenVideo(video_frame);
			MyVectorObj *listPrev = vtemp->layer3->layer1->transform;
			vtemp->layer3->layer1->transform =
					video_frame->layer3->layer1->transform;
			uchar enabled = tr->def->func_preprocess_compute(vtemp,
					video_frame->fdb, tr->state, &tr->preprocessSavedData);
			vtemp->layer3->layer1->transform = listPrev;
			closeVideo(vtemp);
			if (!enabled) {
				releaseTransform(tr);
				return 0;
			}
		}
		tr->mustPreprocessCompute = 0;
	}
	if (tr->mustPreprocessLoad) {
		tr->def->func_preprocess_load(tr->state, tr->preprocessSavedData);
		tr->mustPreprocessLoad = 0;
	}
	my_vectorObj_add(video_frame->layer3->layer1->transform, tr);
	video_frame->layer3->flag_new_original = true;
	return 1;
}
uchar addTransformation(VideoFrame *video_frame, const char *trName) {
	return myAddTransformation(video_frame, trName, 0, NULL);
}
uchar addTransformationPreprocessed(VideoFrame *video_frame, const char *trName,
		const char *preprocessState) {
	return myAddTransformation(video_frame, trName, 1, preprocessState);
}
void removeLastTransformation(VideoFrame *video_frame) {
	struct V_Layer1_transformations *v = video_frame->layer3->layer1;
	int64_t nt = my_vectorObj_size(v->transform);
	my_assert_greaterInt("numTransformations", nt, 0);
	Transform *tr = my_vectorObj_remove(v->transform, nt - 1);
	releaseTransform(tr);
}
char *getVideoName(VideoFrame *video_frame) {
	return video_frame->name;
}
double getFpsVideo(VideoFrame *video_frame) {
	//assertTrue("isVideo", video_frame->layer3->layer2->layer1->layer0->isVideo);
	return video_frame->layer3->layer1->layer0->dVideo.fps;
}
CvSize getFrameSize(VideoFrame *video_frame) {
	if (getCurrentNumFrame(video_frame) == NUMFRAME_NOT_OPENED) {
		if (!loadNextFrame(video_frame))
			my_log_error("can't read a frame\n");
	}
	IplImage *img = getCurrentFrameOrig(video_frame);
	return cvSize(img->width, img->height);
}
int64_t getCurrentNumFrame(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return 0;
	return getCurrentNumFrame_layer0(video_frame->layer3->layer1->layer0);
}
bool seekVideoToFrame(VideoFrame *video_frame, int64_t desiredFrame) {
	if (video_frame == NULL)
		return 0;
	return seekVideoToFrame_layer3(video_frame->layer3, desiredFrame);
}
double getVideoTimeStart(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return 0;
	if (getIsVideo(video_frame))
		return video_frame->layer3->layer1->layer0->dVideo.timeStart;
	else
		return 0;
}
double getVideoTimeEnd(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return 0;
	if (getIsVideo(video_frame))
		return video_frame->layer3->layer1->layer0->dVideo.timeEnd;
	else
		return 1;
}
bool loadNextFrame(VideoFrame *video_frame) {
	if (getCurrentNumFrame(video_frame) == NUMFRAME_NOT_OPENED) {
		int64_t first = 0;
		if (getIsVideo(video_frame))
			first =
					video_frame->layer3->layer1->layer0->dVideo.physicalStartFrame;
		return seekVideoToFrame_layer3(video_frame->layer3, first);
	} else {
		return seekVideoToFrame_layer3(video_frame->layer3,
				getCurrentNumFrame(video_frame) + 1);
	}
}
IplImage *getCurrentFrameOrig(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return NULL;
	return getFrame_layer3_orig(video_frame->layer3);
}
IplImage *getCurrentFrameGray(VideoFrame *video_frame) {
	if (video_frame == NULL)
		return NULL;
	return getFrame_layer3_gray(video_frame->layer3);
}
bool getIsVideo(VideoFrame *video_frame) {
	return video_frame->layer3->layer1->layer0->isVideo;
}
bool getIsImage(VideoFrame *video_frame) {
	return video_frame->layer3->layer1->layer0->isImage;
}
bool getIsAudio(VideoFrame *video_frame) {
	return video_frame->layer3->layer1->layer0->isAudio;
}
bool getIsWebcam(VideoFrame *video_frame) {
	return video_frame->layer3->layer1->layer0->isWebcam;
}
void extractVideoSegment(VideoFrame *video_frame, double secondsStart,
		double secondsEnd, const char *newFilename) {
	my_assert_isTrue("isVideo", video_frame->layer3->layer1->layer0->isVideo);
	char *stStart = my_newString_hhmmssfff(secondsStart);
	char *stLength = my_newString_doubleDec(secondsEnd - secondsStart, 3);
	char *command =	//-c:a libfaac
			my_newString_format(
					"-y -loglevel error -ss %s -i \"%s\" -t %s -c:v libx264 \"%s.mp4\"",
					stStart,
					video_frame->layer3->layer1->layer0->file_full_path,
					stLength, newFilename);
	my_log_info("extracting %s %s %s sec\n", video_frame->name, stStart,
			stLength);
	MyTimer *timer = my_timer_new();
	pvcd_system_call_ffmpeg(command);
	my_log_info("extracted ok: %1.1lf seconds\n", my_timer_getSeconds(timer));
	my_timer_release(timer);
	MY_FREE_MULTI(stStart, stLength, command);
}

static void getNumberOfFrames_fast(const char *file_full_path,
		int64_t *out_numFrames, double *out_fps) {
	MY_MUTEX_LOCK(video_open_mutex);
	CvCapture *cap = cvCreateFileCapture(file_full_path);
	if (cap == NULL)
		my_log_error("can't open %s\n", file_full_path);
	double fps = cvGetCaptureProperty(cap, CV_CAP_PROP_FPS);
	cvSetCaptureProperty(cap, CV_CAP_PROP_POS_AVI_RATIO, 1.0);
	double val = cvGetCaptureProperty(cap, CV_CAP_PROP_POS_FRAMES);
	cvReleaseCapture(&cap);
	MY_MUTEX_UNLOCK(video_open_mutex);
	*out_numFrames = (int64_t) val;
	*out_fps = fps;
}
static void getNumberOfFrames_slow(const char *file_full_path,
		int64_t *out_numFrames, double *out_fps) {
	MY_MUTEX_LOCK(video_open_mutex);
	CvCapture *cap = cvCreateFileCapture(file_full_path);
	if (cap == NULL)
		my_log_error("can't open %s\n", file_full_path);
	MY_MUTEX_UNLOCK(video_open_mutex);
	double fps = cvGetCaptureProperty(cap, CV_CAP_PROP_FPS);
	int64_t cont = 0;
	while (cvGrabFrame(cap))
		cont++;
	MY_MUTEX_LOCK(video_open_mutex);
	cvReleaseCapture(&cap);
	MY_MUTEX_UNLOCK(video_open_mutex);
	*out_numFrames = cont;
	*out_fps = fps;
}
void computeVideoLength(const char *videofilename, bool useFastApprox,
		double *out_secondsLength, double *out_fps) {
	char *inputFilename = my_io_normalizeFilenameToRead(videofilename);
	char *name = my_io_getFilename(inputFilename);
	int64_t numFrames = 0;
	double fps = 0;
	if (useFastApprox) {
		my_log_info("calculating number of frames in %s\n", name);
		getNumberOfFrames_fast(inputFilename, &numFrames, &fps);
		if (numFrames <= 0 || numFrames >= 1000000) {
			my_log_info(
					"%s reports %"PRIi64" frames! retrying frame-by-frame...\n",
					name, numFrames);
			useFastApprox = false;
		}
	}
	if (!useFastApprox) {
		my_log_info("calculating number of frames in %s frame-by-frame\n",
				name);
		getNumberOfFrames_slow(inputFilename, &numFrames, &fps);
	}
	*out_secondsLength = (fps == 0) ? 0 : numFrames / fps;
	*out_fps = fps;
	MY_FREE_MULTI(inputFilename, name);
}
#endif
