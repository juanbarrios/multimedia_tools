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

struct Parameters {
	bool withKeypoints, whiteBg;
	Extractor *exQ, *exR;
	struct PairMatcher *matcher;
	char *nombreWindow;
	MyImageResizer *resizerToScreen1;
	MyImageResizer *resizerToScreen2;
	int64_t waitMs;
	bool onlyStitchImages;
	bool stitchWithBorders;
	double stitchBlendWeight;
	const char *stitchReplaceFile;
};
struct InputImg {
	const char *filename;
	MyVectorString *transfs;
	VideoFrame *video_frame;
	IplImage *img;
	bool finished;
};

static void mat_buscarMatch(struct Parameters *param, const char *nombreImg1,
		const char *nombreImg2, MyImagePair *imagenUnida,
		MyLocalDescriptors *desc1, MyLocalDescriptors *desc2) {
	if (param->withKeypoints)
		my_imagePair_drawKeypoints(imagenUnida, desc1, desc2);
	int64_t numMatches = computePairMatches(param->matcher, desc1, desc2);
	if (numMatches == 0)
		return;
	struct MatchesAndModel *pairs = getLastMatchesAndModel(param->matcher);
	if (pairs == NULL)
		return;
	my_imagePair_drawMatches(imagenUnida, desc1, desc2, pairs->numMatches,
			pairs->idVectorQ, pairs->idVectorR);
	int64_t numVectors1 = my_localDescriptors_getNumDescriptors(desc1);
	int64_t numVectors2 = my_localDescriptors_getNumDescriptors(desc2);
	my_log_info(
			"%"PRIi64"/%"PRIi64" %2.1lf%% vectors in %s matches %"PRIi64"/%"PRIi64" vectors in %s\n",
			pairs->numMatches, numVectors1,
			(100.0 * pairs->numMatches) / numVectors1, nombreImg1,
			pairs->numMatches, numVectors2, nombreImg2);
}
static void saveImage(struct InputImg *input_base, struct InputImg *input,
		IplImage *imgJoin) {
	char *n1 = my_io_getFilename(input_base->filename);
	char *n2 = my_io_getFilename(input->filename);
	char *s1a = my_subStringC_startLast(n1, '.');
	char *s2a = my_subStringC_startLast(n2, '.');
	char *imgName = my_newString_format("MATCH_%s_%s.png", s1a, s2a);
	my_io_removeInvalidChars(imgName);
	if (cvSaveImage(imgName, imgJoin, NULL))
		my_log_info("image %s saved\n", imgName);
	else
		my_log_info("error saving %s\n", imgName);
	MY_FREE_MULTI(n1, n2, s1a, s2a, imgName);
}
static void showImgInScreen1(struct Parameters *param, IplImage *img) {
	if (param->resizerToScreen1 == NULL)
		param->resizerToScreen1 = my_imageResizer_newScreenSize();
	IplImage *imgShow = my_imageResizer_resizeImage(img,
			param->resizerToScreen1);
	cvShowImage(param->nombreWindow, imgShow);
}
static void showImgInScreen2(struct Parameters *param, IplImage *img) {
	if (param->resizerToScreen2 == NULL)
		param->resizerToScreen2 = my_imageResizer_newScreenSize();
	IplImage *imgShow = my_imageResizer_resizeImage(img,
			param->resizerToScreen2);
	cvShowImage("stich", imgShow);
}
static IplImage *computeImageMatch(struct Parameters *param,
		struct InputImg *input_base, struct InputImg *input) {
	MyLocalDescriptors *descriptorQ = extractVolatileDescriptor(param->exQ,
			input_base->img);
	my_log_info("LEFT: %"PRIi64" vectors %s %"PRIi64"-d\n",
			my_localDescriptors_getNumDescriptors(descriptorQ),
			my_datatype_codeToDescription(
					my_localDescriptors_getVectorDatatype(descriptorQ)),
			my_localDescriptors_getVectorDimensions(descriptorQ));
	MyLocalDescriptors *descriptorR = extractVolatileDescriptor(param->exR,
			input->img);
	my_log_info("RIGHT: %"PRIi64" vectors %s %"PRIi64"-d\n",
			my_localDescriptors_getNumDescriptors(descriptorR),
			my_datatype_codeToDescription(
					my_localDescriptors_getVectorDatatype(descriptorR)),
			my_localDescriptors_getVectorDimensions(descriptorR));
	MyImagePair *imagenUnida = my_imagePair_new(input_base->img, input->img,
	MY_IMAGEPAIR_HORIZONTAL_TOP, 30, param->whiteBg);
	mat_buscarMatch(param, input_base->filename, input->filename, imagenUnida,
			descriptorQ, descriptorR);
	IplImage *imgJoin = my_imagePair_release_returnIplImage(imagenUnida);
	showImgInScreen1(param, imgJoin);
	return imgJoin;
}
static IplImage *loadImageLeftStitch(IplImage *imgsrc, const char *filename) {
	IplImage *img = cvLoadImage(filename, CV_LOAD_IMAGE_UNCHANGED);
	MyImageResizer *resizer = my_imageResizer_newFixedSize(imgsrc->width,
			imgsrc->height);
	IplImage *new_resized = my_imageResizer_resizeImage(img, resizer);
	IplImage *image_ret = my_image_duplicate(new_resized);
	cvReleaseImage(&img);
	my_imageResizer_release(resizer);
	return image_ret;
}
static IplImage *computeStitch(struct Parameters *param,
		struct InputImg *input_base, struct InputImg *input) {
	struct MatchesAndModel *pairs = getLastMatchesAndModel(param->matcher);
	if (pairs == NULL)
		return NULL;
	CvMat *transform = getLastTransform(param->matcher);
	if (transform == NULL)
		return NULL;
	IplImage *img1 = input_base->img;
	IplImage *img2 = input->img;
	if (param->stitchReplaceFile != NULL)
		img1 = loadImageLeftStitch(img1, param->stitchReplaceFile);
	IplImage *imgStitch = generateStitching(img1, img2, transform,
			param->stitchWithBorders, param->stitchBlendWeight);
	if (param->stitchReplaceFile != NULL)
		my_image_release(img1);
	showImgInScreen2(param, imgStitch);
	return imgStitch;
	//ffmpeg -f image2 -i %d.jpg -vf "crop=((in_w/2)*2):((in_h/2)*2)" -r 30 -c:v libx264 out.mp4
}
static bool mat_hacerMatch(struct Parameters *param,
		struct InputImg *input_base, struct InputImg *input) {
	IplImage *imgJoin = NULL;
	IplImage *imgStitch = NULL;
	if (!param->onlyStitchImages) {
		imgJoin = computeImageMatch(param, input_base, input);
	}
	if (param->onlyStitchImages) {
		imgStitch = computeStitch(param, input_base, input);
	}
	bool exit = false;
	for (;;) {
		int c = cvWaitKey(param->waitMs);
		if (c == -1)
			break;
		c &= 0x7F;
		if (c == 27) {
			exit = true;
			break;
		} else if (c == ' ') {
			param->waitMs = (param->waitMs == 0) ? 1 : 0;
			break;
		} else if (c == 'n') {
			break;
		} else if (c == 's') {
			if (imgJoin != NULL)
				saveImage(input_base, input, imgJoin);
		} else if (c == 'p') {
			imgStitch = computeStitch(param, input_base, input);
		} else if (c == 'q') {
			if (imgStitch != NULL)
				saveImage(input_base, input, imgStitch);
		} else if (c == 'v') {
			param->onlyStitchImages = !param->onlyStitchImages;
			break;
		}
	}
	my_image_release(imgJoin);
	my_image_release(imgStitch);
	return exit;
}
static void loadImg(struct InputImg *inputImg) {
	if (inputImg->finished)
		return;
	if (inputImg->video_frame == NULL) {
		inputImg->video_frame = openFile(inputImg->filename, 1);
		for (int64_t i = 0; i < my_vectorString_size(inputImg->transfs); ++i) {
			const char *tr = my_vectorString_get(inputImg->transfs, i);
			addTransformation(inputImg->video_frame, tr);
		}
	}
	if (getIsVideo(inputImg->video_frame)
			|| getIsWebcam(inputImg->video_frame)) {
		int64_t i;
		for (i = 0; !inputImg->finished && i < 3; ++i) {
			if (loadNextFrame(inputImg->video_frame))
				inputImg->img = getCurrentFrameOrig(inputImg->video_frame);
			else
				inputImg->finished = 1;
		}
	} else {
		if (inputImg->img == NULL) {
			inputImg->img = getCurrentFrameOrig(inputImg->video_frame);
		} else {
			inputImg->finished = 1;
		}
	}
}
static void print_help() {
	my_log_info("   n = shows next image\n");
	my_log_info(" [space] = play/stop\n");
	my_log_info("   s = saves current image\n");
	my_log_info("   p = performs stitching\n");
	my_log_info("   q = saves current stitching\n");
	my_log_info("   v = shows video stitching\n");
}
static void mat_matchear(struct Parameters *param, struct InputImg *input_base,
		MyVectorObj *images) {
	print_help();
	bool exit = false;
	while (!exit) {
		loadImg(input_base);
		if (input_base->finished)
			break;
		bool allFinished = true;
		for (int64_t i = 0; i < my_vectorObj_size(images); ++i) {
			struct InputImg *input = my_vectorObj_get(images, i);
			if (input->finished)
				continue;
			allFinished = false;
			loadImg(input);
			exit = mat_hacerMatch(param, input_base, input);
		}
		if (allFinished)
			break;
	}
	closeVideo(input_base->video_frame);
	for (int64_t i = 0; i < my_vectorObj_size(images); ++i) {
		struct InputImg *input = my_vectorObj_get(images, i);
		closeVideo(input->video_frame);
	}
}
static void showSingleBaseImage(struct Parameters *param,
		struct InputImg *input_base) {
	loadImg(input_base);
	MyLocalDescriptors *ldes = extractVolatileDescriptor(param->exQ,
			input_base->img);
	int64_t num_vectors = my_localDescriptors_getNumDescriptors(ldes);
	my_log_info("%"PRIi64" vectors\n", num_vectors);
	if (param->withKeypoints)
		my_local_drawKeypoints(input_base->img, ldes);
	showImgInScreen1(param, input_base->img);
	cvWaitKey(0);
}
#endif
int pvcd_viewLocalMatches(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s\n", getBinaryName(cmd_params));
		my_log_info("   (-desc|-descQ|-descR descriptor)  Mandatory.\n");
		my_log_info("   -matcher matcherParameters        Mandatory.\n");
		my_log_info("   [-withKeypoints]                  Optional.\n");
		my_log_info("   [-whiteBg]                        Optional.\n");
		my_log_info("   [-stitchWithBorders]              Optional.\n");
		my_log_info(
				"   [-stitchBlendWeight val]          Optional. 0 < val <= 1\n");
		my_log_info("   [-stitchReplaceFile filename]     Optional.\n");
		my_log_info("   imagenBase imagen1 imagen2...\n");
		print_extractors_local();
		my_log_info("\n Matcher:\n");
		my_log_info("   %s\n", pairMatcher_help());
		return pvcd_system_exit_error();
	}
	const char *descQ = NULL, *descR = NULL;
	const char *matcherParameters = NULL;
	struct InputImg *input_base = NULL;
	MyVectorObj *images = my_vectorObj_new();
	struct Parameters param = { 0 };
	struct InputImg *last_input = NULL;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-descQ")) {
			descQ = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-descR")) {
			descR = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-desc")) {
			descQ = descR = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-matcher")) {
			matcherParameters = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-withKeypoints")) {
			param.withKeypoints = true;
		} else if (isNextParam(cmd_params, "-whiteBg")) {
			param.whiteBg = true;
		} else if (isNextParam(cmd_params, "-stitchWithBorders")) {
			param.stitchWithBorders = true;
		} else if (isNextParam(cmd_params, "-stitchBlendWeight")) {
			param.stitchBlendWeight = nextParamFraction(cmd_params);
		} else if (isNextParam(cmd_params, "-stitchReplaceFile")) {
			param.stitchReplaceFile = nextParam(cmd_params);
			my_assert_fileExists(param.stitchReplaceFile);
		} else if (isNextParam(cmd_params, "-tr")) {
			my_assert_notNull("tr last_input", last_input);
			if (last_input->transfs == NULL)
				last_input->transfs = my_vectorString_new();
			my_vectorStringConst_add(last_input->transfs,
					nextParam(cmd_params));
		} else {
			const char *filename = nextParam(cmd_params);
			if (my_string_indexOf(filename, "#") < 0
					&& my_string_indexOf(filename, "@") < 0
					&& my_string_indexOf(filename, "webcam") < 0)
				my_assert_fileExists(filename);
			struct InputImg *input = MY_MALLOC(1, struct InputImg);
			input->filename = filename;
			last_input = input;
			if (input_base == NULL)
				input_base = input;
			else
				my_vectorObj_add(images, input);
		}
	}
	my_assert_notNull("input_base", input_base);
	char *s1a = my_io_getFilename(input_base->filename);
	param.nombreWindow = my_newString_concat("MATCH_", s1a);
	free(s1a);
	my_assert_notNull("descQ", descQ);
	param.exQ = getExtractor(descQ);
	my_assert_equalInt("descriptor type", getDescriptorType(param.exQ).dtype,
	DTYPE_LOCAL_VECTORS);
	if (my_vectorObj_size(images) == 0) {
		showSingleBaseImage(&param, input_base);
	} else {
		my_assert_notNull("descR", descR);
		param.exR = getExtractor(descR);
		my_assert_equalInt("descriptor type",
				getDescriptorType(param.exR).dtype,
				DTYPE_LOCAL_VECTORS);
		my_assert_notNull("matcher", matcherParameters);
		param.matcher = newPairMatcher(matcherParameters);
		mat_matchear(&param, input_base, images);
		releasePairMatcher(param.matcher);
		releaseExtractor(param.exR);
	}
	releaseExtractor(param.exQ);
	free(param.nombreWindow);
#endif
	return pvcd_system_exit_ok(cmd_params);
}
