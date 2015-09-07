/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "dbs.h"

static int qsort_compare_fdb_id(const void *a, const void *b) {
	FileDB *o1 = *(FileDB**) a;
	FileDB *o2 = *(FileDB**) b;
	return strcmp(o1->id, o2->id);
}
static int qsort_compare_fdb_filenameReal(const void *a, const void *b) {
	FileDB *o1 = *(FileDB**) a;
	FileDB *o2 = *(FileDB**) b;
	return strcmp(o1->filenameReal, o2->filenameReal);
}
FileDB *findFileDB_byId(DB *db, const char *file_id, bool fail) {
	int64_t desde = 0, hasta = db->numFilesDb;
	while (desde < hasta) {
		int64_t medio = (desde + hasta) / 2;
		int64_t cmp = strcmp(file_id, db->filesDb_sortId[medio]->id);
		if (cmp > 0) {
			desde = medio + 1;
		} else if (cmp < 0) {
			hasta = medio;
		} else {
			return db->filesDb_sortId[medio];
		}
	}
	if (fail)
		my_log_error("can't find video id '%s'\n", file_id);
	return NULL;
}
//hacer free de lo retornado
MyVectorObj *findFilesDB_byFilenameReal(DB *db, const char *pathReal) {
	int64_t from = 0, to = db->numFilesDb;
	int64_t posFound = -1;
	while (from < to) {
		int64_t med = (from + to) / 2;
		int64_t cmp = strcmp(pathReal,
				db->filesDb_sortFilenameReal[med]->filenameReal);
		if (cmp > 0) {
			from = med + 1;
		} else if (cmp < 0) {
			to = med;
		} else {
			posFound = med;
			break;
		}
	}
	MyVectorObj *found = my_vectorObj_new();
	if (posFound < 0)
		return found;
	//may exists duplicates, locate the first one
	while (posFound > 0
			&& strcmp(pathReal,
					db->filesDb_sortFilenameReal[posFound - 1]->filenameReal)
					== 0)
		posFound--;
	//include all the duplicates
	while (posFound < db->numFilesDb
			&& strcmp(pathReal,
					db->filesDb_sortFilenameReal[posFound]->filenameReal) == 0) {
		my_vectorObj_add(found, db->filesDb_sortFilenameReal[posFound]);
		posFound++;
	}
	return found;
}
static FileDB *parseFileDB(const char *line) {
	MyTokenizer *tk = my_tokenizer_new(line, '\t');
	FileDB *fdb = MY_MALLOC(1, FileDB);
	if (my_tokenizer_isNext(tk, "V"))
		fdb->isVideo = true;
	else if (my_tokenizer_isNext(tk, "I"))
		fdb->isImage = true;
	else if (my_tokenizer_isNext(tk, "A"))
		fdb->isAudio = true;
	else
		my_log_error("error reading line %s\n", my_tokenizer_nextToken(tk));
	fdb->id = my_newString_string(my_tokenizer_nextToken(tk));
	fdb->pathReal = my_newString_string(my_tokenizer_nextToken(tk));
	fdb->filenameReal = my_io_getFilename(fdb->pathReal);
	fdb->filesize = my_tokenizer_nextInt0(tk);
	if (fdb->isVideo) {
		fdb->width = my_tokenizer_nextInt(tk);
		fdb->height = my_tokenizer_nextInt(tk);
		fdb->secStartTime = my_tokenizer_nextSeconds(tk);
		fdb->secEndTime = my_tokenizer_nextSeconds(tk);
		fdb->numObjsPerSecond = my_tokenizer_nextFraction(tk);
	} else if (fdb->isImage) {
		fdb->width = my_tokenizer_nextInt(tk);
		fdb->height = my_tokenizer_nextInt(tk);
		my_tokenizer_nextToken(tk); //start
		my_tokenizer_nextToken(tk); //end
		my_tokenizer_nextToken(tk); //fps
	} else if (fdb->isAudio) {
		my_tokenizer_nextToken(tk); //width
		my_tokenizer_nextToken(tk); //height
		fdb->secStartTime = my_tokenizer_nextSeconds(tk);
		fdb->secEndTime = my_tokenizer_nextSeconds(tk);
		fdb->numObjsPerSecond = my_tokenizer_nextFraction(tk);
	}
	fdb->lengthSec = fdb->secEndTime - fdb->secStartTime;
	fdb->numTransfs = my_tokenizer_nextInt0(tk);
	if (fdb->numTransfs > 0) {
		fdb->nameTransf = MY_MALLOC(fdb->numTransfs, char*);
		fdb->preprocessDataTransf = MY_MALLOC(fdb->numTransfs, char*);
		int64_t i;
		for (i = 0; i < fdb->numTransfs; ++i) {
			fdb->nameTransf[i] = my_newString_string(
					my_tokenizer_nextToken(tk));
			const char *st = my_tokenizer_nextToken(tk);
			if (st != NULL && !my_string_equals_ignorecase(st, "null"))
				fdb->preprocessDataTransf[i] = my_newString_string(st);
		}
	}
	my_tokenizer_release(tk);
	return fdb;
}
const char *getDbLoadOptions() {
	return "%start=[num];length=[num];idInclude=[id];idExclude=[id];fileInclude=[file];fileExclude=[file]";
}
struct ParametersLoadDb {
	int64_t loadStart;
	int64_t loadLength;
	MyVectorString *loadInclude;
	MyVectorString *loadExclude;
};
static struct ParametersLoadDb loadParametersDB(char *loadOptions) {
	struct ParametersLoadDb p = { 0 };
	p.loadStart = 0;
	p.loadLength = 0;
	p.loadInclude = NULL;
	p.loadExclude = NULL;
	if (loadOptions == NULL)
		return p;
	MyTokenizer *tk = my_tokenizer_new(loadOptions, ';');
	const char *token;
	while ((token = my_tokenizer_nextToken(tk)) != NULL) {
		if (my_string_startsWith(token, "start=")) {
			p.loadStart = my_parse_int_csubFirstEnd(token, '=');
			if (p.loadLength == 0)
				p.loadLength = 1;
		} else if (my_string_startsWith(token, "length=")) {
			p.loadLength = my_parse_int_csubFirstEnd(token, '=');
		} else if (my_string_startsWith(token, "idInclude=")) {
			if (p.loadInclude == NULL)
				p.loadInclude = my_vectorString_new();
			char *id = my_subStringC_firstEnd(token, '=');
			my_vectorString_add(p.loadInclude, id);
		} else if (my_string_startsWith(token, "idExclude=")) {
			if (p.loadExclude == NULL)
				p.loadExclude = my_vectorString_new();
			char *id = my_subStringC_firstEnd(token, '=');
			my_vectorString_add(p.loadExclude, id);
		} else if (my_string_startsWith(token, "fileInclude=")) {
			if (p.loadInclude == NULL)
				p.loadInclude = my_vectorString_new();
			char *filename = my_subStringC_firstEnd(token, '=');
			MyVectorString *lins = my_io_loadLinesFile(filename);
			my_log_info("%"PRIi64" include ids loaded from %s\n",
					my_vectorString_size(lins), filename);
			free(filename);
			my_vectorString_addAll(p.loadInclude, lins);
			my_vectorString_release(lins, false);
		} else if (my_string_startsWith(token, "fileExclude=")) {
			if (p.loadExclude == NULL)
				p.loadExclude = my_vectorString_new();
			char *filename = my_subStringC_firstEnd(token, '=');
			MyVectorString *lins = my_io_loadLinesFile(filename);
			my_log_info("%"PRIi64" exclude ids loaded from %s\n",
					my_vectorString_size(lins), filename);
			free(filename);
			my_vectorString_addAll(p.loadExclude, lins);
			my_vectorString_release(lins, false);
		} else {
			my_log_error("unknown db option %s. Help: %s\n", token,
					getDbLoadOptions());
		}
	}
	my_tokenizer_releaseValidateEnd(tk);
	if (p.loadInclude != NULL)
		my_vectorString_sortCaseSensitive(p.loadInclude);
	if (p.loadExclude != NULL)
		my_vectorString_sortCaseSensitive(p.loadExclude);
	return p;
}
static MyVectorObj *parseFiles(char *filenameDB, struct ParametersLoadDb *param) {
	MyVectorObj *files = my_vectorObj_new();
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filenameDB, 1), "PVCD", "FilesDB", 1, 0);
	const char *line;
	int64_t contLine = 0;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		contLine++;
		if (contLine <= param->loadStart)
			continue;
		FileDB *fdb = parseFileDB(line);
		if ((param->loadInclude != NULL
				&& my_vectorString_indexOf_binsearch(param->loadInclude,
						fdb->id) < 0)
				|| (param->loadExclude != NULL
						&& my_vectorString_indexOf_binsearch(param->loadExclude,
								fdb->id) >= 0)) {
			releaseFileDB(fdb);
			continue;
		}
		my_vectorObj_add(files, fdb);
		if (param->loadLength > 0
				&& my_vectorObj_size(files) >= param->loadLength)
			break;
	}
	my_lreader_close(reader, true);
	return files;
}
static void addFilesToDb(DB *db, MyVectorObj *files) {
	db->numFilesDb = my_vectorObj_size(files);
	my_assert_greaterInt("num files in db", db->numFilesDb, 0);
	db->filesDb = MY_MALLOC(db->numFilesDb, FileDB *);
	db->filesDb_sortId = MY_MALLOC(db->numFilesDb, FileDB *);
	db->filesDb_sortFilenameReal = MY_MALLOC(db->numFilesDb, FileDB *);
	int64_t i;
	for (i = 0; i < db->numFilesDb; ++i) {
		FileDB *fdb = my_vectorObj_get(files, i);
		if (i == 0) {
			if (fdb->isImage)
				db->isImageDb = true;
			else if (fdb->isAudio)
				db->isAudioDb = true;
			else if (fdb->isVideo)
				db->isVideoDb = true;
		}
		if ((fdb->isImage && !db->isImageDb) || (fdb->isAudio && !db->isAudioDb)
				|| (fdb->isVideo && !db->isVideoDb))
			my_log_error("Mixed file types in %s\n", db->pathBase);
		fdb->db = db;
		fdb->internal_id = i;
		db->filesDb[i] = db->filesDb_sortId[i] =
				db->filesDb_sortFilenameReal[i] = fdb;
	}
	qsort(db->filesDb_sortId, db->numFilesDb, sizeof(FileDB*),
			qsort_compare_fdb_id);
	qsort(db->filesDb_sortFilenameReal, db->numFilesDb, sizeof(FileDB*),
			qsort_compare_fdb_filenameReal);
	for (i = 0; i < db->numFilesDb - 1; ++i) {
		FileDB *fdb = db->filesDb_sortId[i];
		FileDB *fdbNext = db->filesDb_sortId[i + 1];
		if (strcmp(fdb->id, fdbNext->id) == 0)
			my_log_error("duplicated id '%s' in %s\n", fdb->id, db->pathBase);
	}
}
DB *loadDB(const char *pathDB, bool fail, bool loadFiles) {
	char *dbPath = NULL, *loadOptions = NULL;
	if (my_string_indexOf(pathDB, "%") > 0) {
		dbPath = my_subStringC_startFirst(pathDB, '%');
		loadOptions = my_subStringC_firstEnd(pathDB, '%');
	} else {
		dbPath = my_newString_string(pathDB);
	}
	DB *db = MY_MALLOC(1, DB);
	if (my_io_existsDir(dbPath)) {
		db->pathBase = my_io_getAbsolutPath(dbPath);
	} else if (fail) {
		my_log_error("db '%s' not found\n", dbPath);
	} else {
		db->pathBase = my_newString_string(dbPath);
	}
	db->pathFiles = my_newString_format("%s/files.txt", db->pathBase);
	db->pathLogs = my_newString_format("%s/logs", db->pathBase);
	db->pathSegmentations = my_newString_format("%s/segmentations",
			db->pathBase);
	db->pathDescriptors = my_newString_format("%s/descriptors", db->pathBase);
	db->pathClustering = my_newString_format("%s/clustering", db->pathBase);
	if (!loadFiles || !my_io_existsFile(db->pathFiles))
		return db;
	struct ParametersLoadDb param = loadParametersDB(loadOptions);
	MY_FREE(loadOptions);
	MyVectorObj *files = parseFiles(db->pathFiles, &param);
	my_vectorString_release(param.loadInclude, true);
	my_vectorString_release(param.loadExclude, true);
	if (my_vectorObj_size(files) == 0) {
		if (fail)
			my_log_error("db %s does not contain any file\n", pathDB);
		return NULL;
	}
	addFilesToDb(db, files);
	my_vectorObj_release(files, 0);
	return db;
}
FileDB *loadFileDB(const char *nameDB, const char *idFile, bool fail) {
	DB *db = loadDB(nameDB, false, false);
	if (!my_io_existsFile(db->pathFiles)) {
		if (fail)
			my_log_error("can't find db '%s'\n", nameDB);
		else
			return NULL;
	}
	FileDB *fdb = NULL;
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(db->pathFiles, 1), "PVCD", "FilesDB", 1, 0);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		MyTokenizer *tk = my_tokenizer_new(line, '\t');
		my_tokenizer_nextToken(tk); //type
		const char *id = my_tokenizer_nextToken(tk);
		if (my_string_equals_ignorecase(idFile, id))
			fdb = parseFileDB(line);
		my_tokenizer_release(tk);
		if (fdb != NULL)
			break;
	}
	my_lreader_close(reader, true);
	if (fdb == NULL && fail)
		my_log_error("can't find file '%s' in db '%s'\n", idFile, nameDB);
	if (fdb != NULL)
		fdb->db = db;
	return fdb;
}
void releaseDB(DB *db) {
	if (db == NULL)
		return;
	if (db->filesDb != NULL) {
		int64_t i;
		for (i = 0; i < db->numFilesDb; ++i) {
			FileDB *fdb = db->filesDb[i];
			releaseFileDB(fdb);
		}
		MY_FREE_MULTI(db->filesDb, db->filesDb_sortId,
				db->filesDb_sortFilenameReal);
	}
	MY_FREE_MULTI(db->pathBase, db->pathFiles, db->pathLogs,
			db->pathSegmentations, db->pathDescriptors, db);
}
void releaseFileDB(FileDB *fdb) {
	if (fdb->numTransfs > 0) {
		int64_t j;
		for (j = 0; j < fdb->numTransfs; ++j)
			MY_FREE_MULTI(fdb->nameTransf[j], fdb->preprocessDataTransf[j]);
		MY_FREE_MULTI(fdb->nameTransf, fdb->preprocessDataTransf);
	}
	MY_FREE_MULTI(fdb->id, fdb->pathReal, fdb->filenameReal, fdb);
}
FILE *newDBfile(DB *db) {
	my_io_createDir(db->pathBase, 0);
	FILE *out = my_io_openFileWrite1Config(db->pathFiles, "PVCD", "FilesDB", 1,
			0);
	fprintf(out,
			"#type\tid\tpath\tfilesize\twidth\theight\ttimeStart\ttimeEnd\tfps\tnum_transforms\ttransforms(name,preprocess)...\n");
	return out;
}
void addFileDB_toDB(FILE *out, FileDB *fdb) {
	const char *stType = NULL;
	char *stWidth = NULL, *stHeight = NULL, *stStart = NULL, *stEnd = NULL,
			*stFps = NULL;
	if (fdb->isVideo) {
		stType = "V";
		stWidth = my_newString_int(fdb->width);
		stHeight = my_newString_int(fdb->height);
		stStart = my_newString_double(fdb->secStartTime);
		stEnd = my_newString_double(fdb->secEndTime);
		stFps = my_newString_double(fdb->numObjsPerSecond);
	} else if (fdb->isImage) {
		stType = "I";
		stWidth = my_newString_int(fdb->width);
		stHeight = my_newString_int(fdb->height);
	} else if (fdb->isAudio) {
		stType = "A";
		stStart = my_newString_double(fdb->secStartTime);
		stEnd = my_newString_double(fdb->secEndTime);
		stFps = my_newString_double(fdb->numObjsPerSecond);
	} else {
		my_log_error("unknown file type\n");
	}
	char *line = my_newString_format(
			"%s\t%s\t%s\t%"PRIi64"\t%s\t%s\t%s\t%s\t%s", stType, fdb->id,
			fdb->pathReal, fdb->filesize, (stWidth == NULL) ? "-" : stWidth,
			(stHeight == NULL) ? "-" : stHeight,
			(stStart == NULL) ? "-" : stStart, (stEnd == NULL) ? "-" : stEnd,
			(stFps == NULL) ? "-" : stFps);
	MY_FREE_MULTI(stWidth, stHeight, stStart, stEnd, stFps);
	if (fdb->numTransfs > 0) {
		MyStringBuffer *sb = my_stringbuf_new();
		for (int64_t i = 0; i < fdb->numTransfs; ++i) {
			my_stringbuf_appendString(sb, "\t");
			my_stringbuf_appendString(sb, fdb->nameTransf[i]);
			my_stringbuf_appendString(sb, "\t");
			my_stringbuf_appendString(sb,
					(fdb->preprocessDataTransf[i] == NULL) ?
							"null" : fdb->preprocessDataTransf[i]);
		}
		fprintf(out, "%s\t%"PRIi64"%s\n", line, fdb->numTransfs,
				my_stringbuf_getCurrentBuffer(sb));
		my_stringbuf_release(sb);
	} else {
		fprintf(out, "%s\n", line);
	}
	fflush(out);
	free(line);
}
MyVectorString *getAllSegmentationsInDb(DB *db) {
	MyVectorString *dirnames = my_vectorString_new();
	my_vectorString_keepSortedCaseSensitive(dirnames);
	MyVectorString *ll = my_io_listDirsInDir(db->pathSegmentations);
	for (int64_t j = 0; j < my_vectorString_size(ll); ++j) {
		char *st = my_vectorString_get(ll, j);
		char *samp = my_io_getFilename(st);
		if (my_vectorString_indexOf_binsearch(dirnames, samp) < 0)
			my_vectorString_add(dirnames, my_newString_string(samp));
		free(samp);
	}
	my_vectorString_release(ll, true);
	return dirnames;
}

static MyVectorString *priv_exts_audio = NULL, *priv_exts_image = NULL,
		*priv_exts_video = NULL;

static void priv_reg_fileExtensions(const char *line, MyVectorString **arr) {
	if (*arr != NULL)
		my_vectorString_release(*arr, 1);
	*arr = my_vectorString_new();
	my_vectorString_keepSortedIgnoreCase(*arr);
	MyTokenizer *tk = my_tokenizer_new(line, ',');
	while (my_tokenizer_hasNext(tk)) {
		char *text = my_newString_string(my_tokenizer_nextToken(tk));
		if (!my_vectorString_add(*arr, text))
			MY_FREE(text);
	}
	my_tokenizer_release(tk);
}
void pvcd_register_audio_fileExtensions(const char *extensions) {
	priv_reg_fileExtensions(extensions, &priv_exts_audio);
}
void pvcd_register_image_fileExtensions(const char *extensions) {
	priv_reg_fileExtensions(extensions, &priv_exts_image);
}
void pvcd_register_video_fileExtensions(const char *extensions) {
	priv_reg_fileExtensions(extensions, &priv_exts_video);
}
void pvcd_register_default_fileExtensions() {
	pvcd_register_audio_fileExtensions("mp3,wav,ogg,mp2,wma,m4a");
	pvcd_register_image_fileExtensions("png,jpg,jpeg,gif,bmp,pgm,ppm");
	pvcd_register_video_fileExtensions("mp4,mpg,avi,mov,flv,mkv,wmv,webm,ogv");
}
static bool priv_has_extension(const char *filename, MyVectorString *exts) {
	if (exts == NULL)
		return 0;
	int64_t n = my_string_lastIndexOf(filename, ".");
	const char *ext = filename + n + 1;
	return (my_vectorString_indexOf_binsearch(exts, ext) < 0) ? 0 : 1;
}
bool is_audio_ext(const char *filename) {
	return priv_has_extension(filename, priv_exts_audio);
}
bool is_image_ext(const char *filename) {
	return priv_has_extension(filename, priv_exts_image);
}
bool is_video_ext(const char *filename) {
	return priv_has_extension(filename, priv_exts_video);
}
