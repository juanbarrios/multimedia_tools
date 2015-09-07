/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

#ifndef NO_OPENCV
struct S_Params {
	uchar checkVideoFrames, noRev, revCamcording, revPipIn, revPipOut;
	MyVectorString *transformations;
	MyVectorString *newTransfs;
	MyVectorString *newSuffixes;
	DB *newDB;
	MyVectorString *filesToAdd;
	MyMapStringObj *renameFiles, *divideVideos;
	FILE *out;
};
struct S_DatosVideo {
	char *newId;
	double newFps;
};

MY_MUTEX_NEWSTATIC(prep_mutex);

static void pre_appendFilename(struct S_Params *params, VideoFrame *video_frame,
		char *suffix) {
	FileDB *fdb = getNewFileDB(video_frame);
	if (suffix != NULL) {
		char *id = my_newString_concat(fdb->id, suffix);
		MY_FREE(fdb->id);
		fdb->id = id;
	}
	my_io_removeInvalidChars(fdb->id);
	my_string_replaceChar(fdb->id, ' ', '_');
	MY_MUTEX_LOCK(prep_mutex);
	if (params->out == NULL) {
		if (my_io_existsFile(params->newDB->pathFiles)) {
			params->out = my_io_openFileAppend1(params->newDB->pathFiles);
		} else {
			params->out = newDBfile(params->newDB);
		}
	}
	addFileDB_toDB(params->out, fdb);
	MY_MUTEX_UNLOCK(prep_mutex);
	releaseFileDB(fdb);
}
static void pre_normalizationVideo(VideoFrame *video_frame,
		struct S_Params *params, char *suffix) {
	int64_t i, cont = 0;
	for (i = 0; i < my_vectorString_size(params->transformations); ++i) {
		char *name = my_vectorString_get(params->transformations, i);
		if (addTransformation(video_frame, name))
			cont++;
	}
	pre_appendFilename(params, video_frame, suffix);
	for (i = 0; i < my_vectorString_size(params->newTransfs); ++i) {
		char *newTr = my_vectorString_get(params->newTransfs, i);
		char *newSuffix = my_vectorString_get(params->newSuffixes, i);
		if (addTransformation(video_frame, newTr)) {
			char *sf = my_newString_concat(suffix, newSuffix);
			pre_appendFilename(params, video_frame, sf);
			removeLastTransformation(video_frame);
		}
	}
	for (i = 0; i < cont; ++i) {
		removeLastTransformation(video_frame);
	}
}
static void pre_addVideo(FileDB *fdb, struct S_Params *params) {
	VideoFrame *video_frame = openFileDB(fdb, 0);
	if (video_frame == NULL) {
		my_log_info("can't open %s\n", fdb->pathReal);
		return;
	}
	if (params->noRev) {
		pre_normalizationVideo(video_frame, params, NULL);
	}
	if (params->revCamcording) {
		if (addTransformation(video_frame, "ACC_FORCED")) {
			pre_normalizationVideo(video_frame, params, "_cc");
			removeLastTransformation(video_frame);
		} else {
			my_log_info("Camcording not detected in %s\n",
					getVideoName(video_frame));
		}
	}
	if (params->revPipIn || params->revPipOut) {
		if (addTransformation(video_frame, "PIP")) {
			if (params->revPipIn) {
				addTransformation(video_frame, "PSEL_IN");
				pre_normalizationVideo(video_frame, params, "_in");
				removeLastTransformation(video_frame);
			}
			if (params->revPipOut) {
				addTransformation(video_frame, "PSEL_OUT");
				pre_normalizationVideo(video_frame, params, "_out");
				removeLastTransformation(video_frame);
			}
			removeLastTransformation(video_frame);
		} else {
			my_log_info("PIP not detected in %s\n",
					getVideoName(video_frame));
		}
	}
	closeVideo(video_frame);
}
static void pre_procesarFile(int64_t currentProcess, void* state,
		int64_t numThread) {
	struct S_Params *params = state;
	char *filename = my_vectorString_get(params->filesToAdd, currentProcess);
	//log_info("%s\n", name);
	bool isImage = is_image_ext(filename);
	bool isVideo = is_video_ext(filename);
	bool isAudio = is_audio_ext(filename);
	double secondsLength = 0;
	double numObjsPerSecond = 0;
	if (isVideo) {
		computeVideoLength(filename, params->checkVideoFrames ? 0 : 1,
				&secondsLength, &numObjsPerSecond);
	} else if (isAudio) {
		computeAudioLength(filename, &secondsLength, &numObjsPerSecond);
	}
	char *id = NULL;
	if (params->renameFiles == NULL) {
		char *name = my_io_getFilename(filename);
		id = my_subStringC_startLast(name, '.');
		MY_FREE(name);
	} else {
		struct S_DatosVideo *datos = my_mapStringObj_get(params->renameFiles,
				filename);
		if (datos == NULL) {
			my_log_info("can't find data for %s\n", filename);
			return;
		} else if (datos->newId == NULL) {
			my_log_info("can't find new name for %s\n", filename);
			return;
		}
		id = my_newString_string(datos->newId);
		if (datos->newFps > 0)
			numObjsPerSecond = datos->newFps;
	}
	FileDB *fdbTemp = MY_MALLOC(1, FileDB);
	fdbTemp->pathReal = my_io_getAbsolutPath(filename);
	fdbTemp->isImage = isImage;
	fdbTemp->isVideo = isVideo;
	fdbTemp->isAudio = isAudio;
	fdbTemp->id = id;
	fdbTemp->numObjsPerSecond = numObjsPerSecond;
	fdbTemp->secStartTime = 0;
	fdbTemp->secEndTime = secondsLength;
	if (params->divideVideos != NULL) {
		if (!isVideo && !isAudio)
			my_log_error("only videos and audio can be divided\n");
		MyVectorDouble *limits = my_mapStringObj_get(params->divideVideos, id);
		if (limits == NULL) {
			my_log_info("can't find limits for %s\n", id);
		} else {
			int64_t k;
			for (k = 1; k < my_vectorDouble_size(limits); ++k) {
				fdbTemp->secStartTime = my_vectorDouble_get(limits, k - 1);
				fdbTemp->secEndTime = my_vectorDouble_get(limits, k);
				fdbTemp->id = my_newString_format("%s_s%"PRIi64"", id, k);
				my_assert_greaterEqualDouble("start segment",
						fdbTemp->secStartTime, 0);
				my_assert_lessDouble("consecutive segments",
						fdbTemp->secStartTime, fdbTemp->secEndTime);
				my_assert_lessEqualDouble("end segment/totalFrames",
						fdbTemp->secEndTime, secondsLength);
				pre_addVideo(fdbTemp, params);
				MY_FREE(fdbTemp->id);
			}
		}
	} else {
		pre_addVideo(fdbTemp, params);
	}
	MY_FREE_MULTI(id, fdbTemp->pathReal, fdbTemp);
}
//file format: original_filename\tnew_filename\tnew_fps
static MyMapStringObj *priv_loadNewFilenames(const char *dataTxt) {
	my_log_info("loading filenames from %s\n", dataTxt);
	MyMapStringObj *data = my_mapStringObj_newCaseSensitive();
	const char *line;
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(dataTxt, 1));
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *name = my_newString_string(my_tokenizer_nextToken(tk));
		struct S_DatosVideo *datos = MY_MALLOC(1, struct S_DatosVideo);
		datos->newId = my_newString_string(my_tokenizer_nextToken(tk));
		if (my_tokenizer_hasNext(tk))
			datos->newFps = my_tokenizer_nextDouble(tk);
		my_mapStringObj_add(data, name, datos);
		my_tokenizer_releaseValidateEnd(tk);
	}
	my_lreader_close(reader, true);
	my_log_info("%"PRIi64" lines loaded\n", my_mapStringObj_size(data));
	return data;
}
//file format: name=>list of end frames
//line ej: vid1\t5,10.1,15.6
//for i in *.sam
//do
//  echo -en "${i/%sam/mpg}\t"
//  tail -n +3 $i | awk '{printf "%s,", $1}'
//  head -n 1 $i | awk '{printf "%s\n", $3}'
//done
static MyMapStringObj *priv_loadVideoLimits(const char *filename) {
	my_log_info("loading partitions from %s\n", filename);
	MyMapStringObj *data = my_mapStringObj_newCaseSensitive();
	const char *line;
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, 1));
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		char *name = my_newString_string(my_tokenizer_nextToken(tk));
		MyVectorDouble *limits = my_tokenizer_splitLineDouble(my_tokenizer_nextToken(tk),
				',');
		my_mapStringObj_add(data, name, limits);
		my_tokenizer_releaseValidateEnd(tk);
	}
	my_lreader_close(reader, true);
	my_log_info("%"PRIi64" lines loaded\n", my_mapStringObj_size(data));
	return data;
}

#define FILETYPE_AUTO 1
#define FILETYPE_IMAGE 2
#define FILETYPE_AUDIO 3
#define FILETYPE_VIDEO 4

static MyVectorString *removeFilesNotInType(MyVectorString *files,
		int64_t fileType) {
	MyVectorString *files_audio = my_vectorString_new();
	MyVectorString *files_image = my_vectorString_new();
	MyVectorString *files_video = my_vectorString_new();
	int64_t i, cont_unknown = 0;
	for (i = 0; i < my_vectorString_size(files); ++i) {
		char *fname = my_vectorString_get(files, i);
		if (is_audio_ext(fname)) {
			my_vectorString_add(files_audio, fname);
		} else if (is_image_ext(fname)) {
			my_vectorString_add(files_image, fname);
		} else if (is_video_ext(fname)) {
			my_vectorString_add(files_video, fname);
		} else {
			cont_unknown++;
			MY_FREE(fname);
			continue;
		}
	}
	if (fileType == FILETYPE_AUTO) {
		if (my_vectorString_size(files_image) >= my_vectorString_size(files_audio)
				&& my_vectorString_size(files_image)
						>= my_vectorString_size(files_video)) {
			my_log_info("processing images...\n");
			fileType = FILETYPE_IMAGE;
		} else if (my_vectorString_size(files_video)
				>= my_vectorString_size(files_audio)) {
			my_log_info("processing videos...\n");
			fileType = FILETYPE_VIDEO;
		} else {
			my_log_info("processing audio...\n");
			fileType = FILETYPE_AUDIO;
		}
	}
	if (fileType == FILETYPE_IMAGE) {
		my_vectorString_release(files_audio, true);
		my_vectorString_release(files_video, true);
		return files_image;
	} else if (fileType == FILETYPE_AUDIO) {
		my_vectorString_release(files_image, true);
		my_vectorString_release(files_video, true);
		return files_audio;
	} else if (fileType == FILETYPE_VIDEO) {
		my_vectorString_release(files_image, true);
		my_vectorString_release(files_audio, true);
		return files_video;
	}
	return NULL;
}
#endif

void db_create_db(CmdParams *cmd_params, const char *argOption) {
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"   -db path_DB_new                            Mandatory\n");
		my_log_info(
				"    [-append]                                  Optional\n");
		my_log_info(
				"    [-reversion (camcording|pip_in|pip_out)]+  One or More. Optional\n");
		my_log_info(
				"    [-tr transformation]+                      One or More. Optional. Ej. SKIP or CROP\n");
		my_log_info(
				"    [-createNew idSuffix transformation]       One or More. Optional. Ej. _f ROTATE_VFLIP\n");
		my_log_info(
				"    [-renameFiles newNames.txt]                Optional\n");
		my_log_info(
				"    [-divideVideos limits.txt]                 Optional\n");
		my_log_info(
				"    [-fileType image|video|audio]              Optional. default=auto\n");
		my_log_info(
				"    [-recursiveDirs]                           Optional\n");
		my_log_info(
				"    [-readFromTxtFiles]                        Optional\n");
		my_log_info(
				"    [-checkVideoFrames]                        Optional\n");
		my_log_info(
				"    [dirs o filenames...]                      Mandatory. One or More. Files or directories.\n");
		return;
	}
	int64_t fileType = FILETYPE_AUTO;
	uchar recursiveDirs = 0, append = 0, readFromTxtFiles = 0;
	const char *pathNewDB = NULL;
	MyVectorString *files = my_vectorString_new();
	struct S_Params params = { 0 };
	params.transformations = my_vectorString_new();
	params.newSuffixes = my_vectorString_new();
	params.newTransfs = my_vectorString_new();
	params.noRev = 1;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-db")) {
			pathNewDB = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-append")) {
			append = 1;
		} else if (isNextParam(cmd_params, "-reversion")) {
			params.noRev = 0;
			if (isNextParam(cmd_params, "camcording"))
				params.revCamcording = 1;
			else if (isNextParam(cmd_params, "pip_in"))
				params.revPipIn = 1;
			else if (isNextParam(cmd_params, "pip_out"))
				params.revPipOut = 1;
			else
				my_log_error("unknown rev %s\n", nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-tr")) {
			my_vectorStringConst_add(params.transformations, nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-createNew")) {
			my_vectorStringConst_add(params.newSuffixes, nextParam(cmd_params));
			my_vectorStringConst_add(params.newTransfs, nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-renameFiles")) {
			params.renameFiles = priv_loadNewFilenames(nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-divideVideos")) {
			params.divideVideos = priv_loadVideoLimits(nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-recursiveDirs")) {
			recursiveDirs = 1;
		} else if (isNextParam(cmd_params, "-checkVideoFrames")) {
			params.checkVideoFrames = 1;
		} else if (isNextParam(cmd_params, "-fileType")) {
			if (isNextParam(cmd_params, "image"))
				fileType = FILETYPE_IMAGE;
			else if (isNextParam(cmd_params, "audio"))
				fileType = FILETYPE_AUDIO;
			else if (isNextParam(cmd_params, "video"))
				fileType = FILETYPE_VIDEO;
			else
				my_log_error("unknown fileType %s\n", nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-readFromTxtFiles")) {
			readFromTxtFiles = 1;
		} else {
			const char *filename = nextParam(cmd_params);
			if (readFromTxtFiles && my_string_endsWith(filename, ".txt")) {
				my_log_info("loading %s...\n", filename);
				MyLineReader *reader = my_lreader_open(
						my_io_openFileRead1(filename, 1));
				const char *line;
				while ((line = my_lreader_readLine(reader)) != NULL)
					my_vectorString_add(files, my_newString_string(line));
				my_lreader_close(reader, true);
			} else
				my_vectorStringConst_add(files, filename);
		}
	}
	if (pathNewDB == NULL)
		my_log_error("-db is mandatory\n");
	if (my_vectorString_size(files) == 0)
		my_log_error("no files to process\n");
	DB *newDB = loadDB(pathNewDB, false, true);
	if (!append && !my_io_notExists(newDB->pathBase))
		my_log_error("already exists db %s (remove it or use -append)\n",
				newDB->pathBase);
	params.filesToAdd = my_io_listFilesInDirsRecursive(files, recursiveDirs);
	params.filesToAdd = removeFilesNotInType(params.filesToAdd, fileType);
	if (my_vectorString_size(params.filesToAdd) == 0)
		my_log_error("no files to process\n");
	my_io_createDir(newDB->pathBase, 0);
	set_logfile(newDB->pathLogs, cmd_params, "");
	params.newDB = newDB;
	my_parallel_incremental(my_vectorString_size(params.filesToAdd), &params,
			pre_procesarFile, "pre", NUM_CORES);
	if (params.out != NULL)
		fclose(params.out);
	if (!my_io_existsFile(newDB->pathFiles))
		my_log_info("error, no files added to db %s\n", newDB->pathBase);
	releaseDB(newDB);
	DB *db = loadDB(pathNewDB, true, true);
	my_log_info("%s contains %"PRIi64" files\n", pathNewDB, db->numFilesDb);
	releaseDB(db);
#endif
}
