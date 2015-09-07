/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "io_util.h"
#include <sys/stat.h>
#include <dirent.h>

//hay que hacer free de lo retornado
char *my_io_getFilename(const char* filePath) {
	int64_t a = my_string_lastIndexOf(filePath, "/");
	int64_t b = my_string_lastIndexOf(filePath, "\\");
	int64_t n = MAX(a, b);
	return my_subStringI_fromEnd(filePath, n + 1);
}
char *my_io_getFilenameWithoutExtension(const char *filePath) {
	char *name = my_io_getFilename(filePath);
	int64_t n = my_string_indexOf(name, ".");
	if (n < 0)
		return name;
	char *name2 = my_subStringI_fromTo(name, 0, n);
	free(name);
	return name2;
}
char *my_io_getFilenameExtension(const char *filePath) {
	char *name = my_io_getFilename(filePath);
	int64_t n = my_string_lastIndexOf(name, ".");
	if (n < 0)
		return my_newString_string("");
	char *name2 = my_subStringI_fromEnd(name, n + 1);
	free(name);
	return name2;
}

//hay que hacer free de lo retornado
char *my_io_getDirname(const char* filePath) {
	int64_t a = my_string_lastIndexOf(filePath, "/");
	int64_t b = my_string_lastIndexOf(filePath, "\\");
	int64_t n = MAX(a, b);
	if (n < 0)
		return my_newString_string(".");
	return my_subStringI_fromTo(filePath, 0, n);
}
char *my_io_normalizeFilenameToRead(const char *filename) {
	my_assert_notNull("filename", filename);
	char *fname = my_newString_string(filename);
	my_string_replaceChar(fname, '\\', '/');
	my_newString_replaceOne(fname, "//", "/");
#if IS_WINDOWS
	char *nn = NULL;
	if (my_string_startsWith_ignorecase(fname, "/c/")) {
		nn = my_newString_replaceOne(fname, "/c/", "C:/");
	} else if (my_string_startsWith_ignorecase(fname, "/d/")) {
		nn = my_newString_replaceOne(fname, "/d/", "D:/");
	} else if (my_string_startsWith_ignorecase(fname, "/cygdrive/c/")) {
		nn = my_newString_replaceOne(fname, "/cygdrive/c/", "C:/");
	} else if (my_string_startsWith_ignorecase(fname, "/cygdrive/d/")) {
		nn = my_newString_replaceOne(fname, "/cygdrive/d/", "D:/");
	}
	if (nn != NULL) {
		MY_FREE(fname);
		fname = nn;
	}
	my_string_replaceChar(fname, '/', '\\');
#endif
	int len = strlen(fname);
	if (len > 1 && (fname[len - 1] == '/' || fname[len - 1] == '\\'))
		fname[len - 1] = '\0';
	return fname;
}
void my_io_removeInvalidChars(char* string) {
	char c;
	for (int64_t i = 0; (c = string[i]) != '\0'; i++) {
		if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?'
				|| c == '|' || c == '&' || c == '!' || c == '"' || c == '\''
				|| c == '<' || c == '>' || c == '{' || c == '}' || c == '['
				|| c == ']' || c == '(' || c == ')')
			string[i] = ',';
	}
	if (string[0] == '.')
		string[0] = ',';
}
char *my_io_normalizeFilenameToCreate(const char *filename) {
	char *fname = my_io_normalizeFilenameToRead(filename);
	char *path = my_io_getDirname(fname);
	char *name = my_io_getFilename(fname);
	my_io_removeInvalidChars(name);
	char *fullpath = my_newString_format("%s/%s", path, name);
	MY_FREE_MULTI(fname, path, name);
	return fullpath;
}
//hay que hacer MY_FREE a lo retornado
char *my_io_getAbsolutPath(const char *relativePath) {
	char *relat = my_io_normalizeFilenameToRead(relativePath);
	struct stat st;
	if (stat(relat, &st) != 0)
		my_log_error("error retrieving absolute path for %s (does not exist)\n",
				relat);
#if IS_WINDOWS
	char *absolute = _fullpath(NULL, relat, 0);
#elif IS_LINUX
	char *absolute = realpath(relat, NULL );
#endif
	if (absolute == NULL) {
		my_log_error("error retrieving absolute path for %s (errno=%i)\n",
				relativePath,
				errno);
	}
	char *absol = my_io_normalizeFilenameToRead(absolute);
	MY_FREE_MULTI(relat, absolute);
	return absol;
}

static FILE *priv_openFile(const char *filename, char *mode, bool fail) {
	char *fname = NULL;
	if (my_string_equals(mode, "rb")) {
		fname = my_io_normalizeFilenameToRead(filename);
		if (fail && !my_io_existsFile(fname))
			my_log_error("can't open file '%s'\n", fname);
	} else if (my_string_equals(mode, "wb")) {
		fname = my_io_normalizeFilenameToCreate(filename);
		if (my_io_existsFile(fname)) {
			my_log_info("overwriting %s\n", fname);
		} else {
			my_log_info("writing %s\n", fname);
		}
	} else if (my_string_equals(mode, "ab")) {
		fname = my_io_normalizeFilenameToCreate(filename);
		if (my_io_existsFile(fname))
			my_log_info("appending %s\n", fname);
		else
			my_log_info("writing %s\n", fname);
	} else {
		my_log_error("unknown file mode %s\n", mode);
	}
	FILE* out = fopen(fname, mode);
	if (fail && out == NULL) {
		my_log_error("can't open file '%s' in mode %s (errno=%i)\n", fname,
				mode,
				errno);
	}
	MY_FREE(fname);
	return out;
}
FILE *my_io_openFileRead1(const char *filename, bool failOnError) {
	return priv_openFile(filename, "rb", failOnError);
}
FILE *my_io_openFileWrite1(const char *filename) {
	my_io_createParentDir(filename);
	return priv_openFile(filename, "wb", 1);
}
FILE *my_io_openFileAppend1(const char *filename) {
	return priv_openFile(filename, "ab", 1);
}

char *my_io_getConfigFileHeader(const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version) {
	return my_newString_format("#%s::%s=%"PRIi64".%"PRIi64"", software_name,
			format_file, major_version, minor_version);
}

FILE *my_io_openFileWrite1Config(const char *filename,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version) {
	my_io_createParentDir(filename);
	FILE *out = my_io_openFileWrite1(filename);
	char *header = my_io_getConfigFileHeader(software_name, format_file,
			major_version, minor_version);
	fprintf(out, "%s\n", header);
	free(header);
	return out;
}

FILE *my_io_openFileRead2(const char *path, const char *file, bool failOnError) {
	char *fname = my_newString_format("%s/%s", path, file);
	FILE *in = my_io_openFileRead1(fname, failOnError);
	MY_FREE(fname);
	return in;
}
FILE *my_io_openFileRead3(const char *path, const char *name,
		const char *extension,
		bool failOnError) {
	char *fname = my_newString_format("%s/%s%s", path, name, extension);
	FILE *in = my_io_openFileRead1(fname, failOnError);
	MY_FREE(fname);
	return in;
}
FILE *my_io_openFileWrite2(const char *path, const char *file) {
	char *fname = my_newString_format("%s/%s", path, file);
	FILE *out = my_io_openFileWrite1(fname);
	MY_FREE(fname);
	return out;
}
FILE *my_io_openFileWrite3(const char *path, const char *name,
		const char *extension) {
	char *fname = my_newString_format("%s/%s%s", path, name, extension);
	FILE *out = my_io_openFileWrite1(fname);
	MY_FREE(fname);
	return out;
}
FILE *my_io_openFileAppend2(const char *path, const char *file) {
	char *fname = my_newString_format("%s/%s", path, file);
	FILE *out = my_io_openFileAppend1(fname);
	MY_FREE(fname);
	return out;
}
FILE *my_io_openFileWrite2Config(const char *path, const char *file,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version) {
	char *fname = my_newString_format("%s/%s", path, file);
	FILE *out = my_io_openFileWrite1Config(fname, software_name, format_file,
			major_version, minor_version);
	MY_FREE(fname);
	return out;
}
char *my_io_detectFileConfig(const char *filename, const char *software_name) {
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, 1));
	const char *line = my_lreader_readLineOrComment(reader);
	char *format = NULL;
	char *header = my_newString_format("#%s::", software_name);
	if (line != NULL && my_string_startsWith(line, header)) {
		int64_t posStart = strlen(header);
		int64_t posEnd = my_string_indexOf(line, "=");
		format = my_subStringI_fromTo(line, posStart, posEnd);
	}
	MY_FREE(header);
	my_lreader_close(reader, true);
	return format;
}

void my_io_readBytesFile(FILE *in, void *buffer, int64_t numBytes,
bool validateEOF) {
	my_assert_greaterEqual_int("filesize", numBytes, 0);
	int64_t n = fread(buffer, sizeof(char), numBytes, in);
	if (n < 0 || n != numBytes)
		my_log_error(
				"error reading file (unexpected EOF, %"PRIi64" of %"PRIi64")\n",
				n, numBytes);
	if (validateEOF && fgetc(in) != EOF)
		my_log_error("error reading file (longer than expected)\n");
}
void* my_io_loadFileBytes(const char *filename, int64_t *out_filesize) {
	FILE *in = my_io_openFileRead1(filename, 1);
	int64_t filesize = my_io_getFilesize2(in);
	void *bytes = MY_MALLOC_NOINIT(filesize, char);
	my_io_readBytesFile(in, bytes, filesize, 1);
	fclose(in);
	if (out_filesize != NULL)
		*out_filesize = filesize;
	return bytes;
}

bool my_io_deleteFile(const char* filename, bool fail) {
	if (!my_io_existsFile(filename)) {
		if (fail)
			my_log_error("can't delete %s\n", filename);
		return false;
	} else if (remove(filename) != 0) {
		if (fail)
			my_log_error("can't delete %s\n", filename);
		return false;
	}
	return true;
}
bool my_io_moveFile(const char* filenameOrig, const char* filenameDest,
bool fail) {
	if (!my_io_existsFile(filenameOrig)) {
		if (fail)
			my_log_error("move, can't read %s\n", filenameOrig);
		return false;
	} else if (my_io_existsFile(filenameDest)) {
		if (fail)
			my_log_error("move, can't overwrite %s\n", filenameDest);
		return false;
	} else if (rename(filenameOrig, filenameDest) != 0) {
		if (fail)
			my_log_error("can't move %s to %s\n", filenameOrig, filenameDest);
		return false;
	}
	return true;
}
#define COPY_BUFF_SIZE 1024
void my_io_copyFile(const char* filenameOrig, const char* filenameDest) {
	if (!my_io_existsFile(filenameOrig)) {
		my_log_info("copy, can't read %s\n", filenameOrig);
		return;
	} else if (my_io_existsFile(filenameDest)) {
		my_log_info("copy, can't overwrite %s\n", filenameDest);
		return;
	}
	FILE *in = my_io_openFileRead1(filenameOrig, 1);
	FILE *out = my_io_openFileWrite1(filenameDest);
	char *buffer = MY_MALLOC_NOINIT(COPY_BUFF_SIZE, char);
	size_t n;
	while ((n = fread(buffer, sizeof(char), COPY_BUFF_SIZE, in)) > 0) {
		fwrite(buffer, sizeof(char), n, out);
	}
	fclose(in);
	fclose(out);
	free(buffer);
}
bool my_io_existsFile(const char* filename) {
	if (filename == NULL)
		return 0;
	char *fname = my_io_normalizeFilenameToRead(filename);
	struct stat st;
	if (stat(fname, &st) == 0)
		return S_ISREG(st.st_mode) ? true : false;
	free(fname);
	return 0;
}
bool my_io_existsFile2(const char *path, const char* file) {
	char *fname = my_newString_format("%s/%s", path, file);
	bool n = my_io_existsFile(fname);
	free(fname);
	return n;
}
bool my_io_existsDir(const char* dirname) {
	char *fname = my_io_normalizeFilenameToRead(dirname);
	struct stat st;
	if (stat(fname, &st) == 0)
		return S_ISDIR(st.st_mode) ? true : false;
	free(fname);
	return 0;
}
bool my_io_notExists(const char* filename) {
	char *fname = my_io_normalizeFilenameToRead(filename);
	struct stat st;
	bool is = (stat(filename, &st) == 0) ? false : true;
	free(fname);
	return is;
}
bool my_io_notExists2(const char *path, const char* file) {
	char *fname = my_newString_format("%s/%s", path, file);
	bool is = my_io_notExists(fname);
	free(fname);
	return is;
}
int64_t my_io_getFilesize(const char* filename) {
#if IS_WINDOWS
	struct stat64 st;
	if (stat64(filename, &st) == 0) {
#elif IS_LINUX
		struct stat st;
		if (stat(filename, &st) == 0) {
#endif
		int64_t size = st.st_size;
		if (size < 0)
			my_log_error("error, file too large %"PRIi64"\n", size);
		return size;
	}
	my_log_error("could not get filesize of %s\n", filename);
	return -1;
}
int64_t my_io_getFilesize2(FILE *file) {
	int64_t fd = fileno(file);
	struct stat st;
	if (fstat(fd, &st) == 0) {
		off_t as = st.st_size;
		int64_t size = as;
		if (size < 0)
			my_log_error("error, file too large %"PRIi64"\n", size);
		return size;
	}
	my_log_error("could not get filesize\n");
	return -1;
}
void my_io_createDir(const char *path, bool fail) {
	if (
#if IS_WINDOWS
	mkdir(path) == 0
#elif IS_LINUX
			mkdir(path, S_IRWXU) == 0
#endif
			) {
		my_log_info("creating directory: %s\n", path);
	} else if (fail) {
		bool exists = my_io_existsDir(path);
		my_log_error("cannot create directory '%s'%s\n", path,
				exists ? " (already exists)" : "");
	}
}
void my_io_createParentDir(const char *file_path) {
	char *parent = my_io_getDirname(file_path);
	if (!my_string_equals(parent, ".") && !my_string_equals(parent, "..")) {
		if (!my_io_existsDir(parent))
			my_io_createDir(parent, 0);
	}
	MY_FREE(parent);
}

static MyVectorString *priv_getAllFilenames(const char *path_dir) {
	MyVectorString *list = my_vectorString_new();
#if IS_WINDOWS
	DIR *dp = opendir(path_dir);
	my_assert_notNull("opendir", dp);
	struct dirent *dir_entry;
	while ((dir_entry = readdir(dp)) != NULL) {
		char *name = dir_entry->d_name;
		my_vectorString_add(list, my_newString_string(name));
	}
	closedir(dp);
#elif IS_LINUX
	struct dirent **namelist = NULL;
	int64_t len = scandir(path_dir, &namelist, NULL, NULL );
	my_assert_greaterEqual_int("scandir", len, 0);
	for (int64_t i = 0; i < len; ++i) {
		char *name = namelist[i]->d_name;
		my_vectorString_add(list, my_newString_string(name));
		MY_FREE(namelist[i]);
	}
	MY_FREE(namelist);
#endif
	my_vectorString_sortCaseSensitive(list);
	return list;
}
static void priv_listFiles(const char *path_dir, MyVectorString *fileList,
		MyVectorString *dirList) {
	if (!my_io_existsDir(path_dir))
		return;
	MyVectorString *allFiles = priv_getAllFilenames(path_dir);
	for (int64_t i = 0; i < my_vectorString_size(allFiles); ++i) {
		char *name = my_vectorString_get(allFiles, i);
		if (my_string_startsWith(name, "."))
			continue;
		char *fname = my_newString_format("%s/%s", path_dir, name);
		if (fileList != NULL && my_io_existsFile(fname)) {
			my_vectorString_add(fileList, fname);
			continue;
		} else if (dirList != NULL && my_io_existsDir(fname)) {
			my_vectorString_add(dirList, fname);
			continue;
		}
		MY_FREE(fname);
	}
	my_vectorString_release(allFiles, true);
}
MyVectorString *my_io_listFilesInDir(const char *path_dir) {
	MyVectorString *list = my_vectorString_new();
	priv_listFiles(path_dir, list, NULL);
	return list;
}
MyVectorString *my_io_listDirsInDir(const char *path_dir) {
	MyVectorString *list = my_vectorString_new();
	priv_listFiles(path_dir, NULL, list);
	return list;
}

MyVectorString *my_io_listFilesInDirsRecursive(MyVectorString *paths,
bool recursive) {
	MyVectorString *filesFound = my_vectorString_new();
	MyVectorString *dirsToVisit = recursive ? my_vectorString_new() : NULL;
	for (int64_t i = 0; i < my_vectorString_size(paths); ++i) {
		char *fname = my_vectorString_get(paths, i);
		if (my_io_existsFile(fname)) {
			my_vectorString_add(filesFound, fname);
		} else {
			priv_listFiles(fname, filesFound, dirsToVisit);
		}
	}
	if (recursive) {
		while (my_vectorString_size(dirsToVisit) > 0) {
			char *dir_path = my_vectorString_remove(dirsToVisit,
					my_vectorString_size(dirsToVisit) - 1);
			priv_listFiles(dir_path, filesFound, dirsToVisit);
			MY_FREE(dir_path);
		}
		my_vectorString_release(dirsToVisit, 0);
	}
	//sort and remove duplicates
	my_vectorString_sortFilenames(filesFound);
	MyVectorString *filenames = my_vectorString_new();
	char *prev = NULL;
	for (int64_t i = 0; i < my_vectorString_size(filesFound); ++i) {
		char *fname = my_vectorString_get(filesFound, i);
		if (my_string_equals(fname, prev)) {
			MY_FREE(fname);
		} else {
			my_vectorString_add(filenames, fname);
			prev = fname;
		}
	}
	my_vectorString_release(filesFound, false);
	return filenames;
}
MyVectorString *my_io_listFilesWithPrefix(const char *path_prefix) {
	char *path_dir = my_io_getDirname(path_prefix);
	MyVectorString *allFiles = my_io_listFilesInDir(path_dir);
	MyVectorString *subset = my_vectorString_new();
	for (int64_t i = 0; i < my_vectorString_size(allFiles); ++i) {
		char *fname = my_vectorString_get(allFiles, i);
		if (my_string_startsWith_ignorecase(fname, path_prefix))
			my_vectorString_add(subset, fname);
		else
			MY_FREE(fname);
	}
	my_vectorString_release(allFiles, false);
	return subset;
}
MyVectorString *my_io_listFilesWithSuffix(const char *path_dir,
		const char *suffix) {
	if (path_dir == NULL)
		path_dir = ".";
	MyVectorString *allFiles = my_io_listFilesInDir(path_dir);
	MyVectorString *subset = my_vectorString_new();
	for (int64_t i = 0; i < my_vectorString_size(allFiles); ++i) {
		char *fname = my_vectorString_get(allFiles, i);
		if (my_string_endsWith_ignorecase(fname, suffix))
			my_vectorString_add(subset, fname);
		else
			MY_FREE(fname);
	}
	my_vectorString_release(allFiles, false);
	return subset;
}
MyVectorString *my_io_loadLinesFile(const char *filename) {
	MyVectorString *fileLines = my_vectorString_new();
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filename, true));
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		my_vectorString_add(fileLines, my_newString_string(line));
	}
	my_lreader_close(reader, true);
	return fileLines;
}
MyVectorString *my_io_loadLinesFileConfig(const char *filename,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version) {
	MyVectorString *fileLines = my_vectorString_new();
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filename, true), software_name, format_file,
			major_version, minor_version);
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		my_vectorString_add(fileLines, my_newString_string(line));
	}
	my_lreader_close(reader, true);
	return fileLines;
}

MyMapStringObj *my_io_loadProperties(const char *filename, bool fail,
		const char *module_name, const char *format_file, int64_t major_version,
		int64_t minor_version) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filename, fail), module_name, format_file,
			major_version, minor_version);
	if (reader == NULL)
		return NULL;
	MyMapStringObj *map = my_mapStringObj_newIgnoreCase();
	const char *line;
	while ((line = my_lreader_readLine(reader)) != NULL) {
		int64_t n = my_string_indexOf(line, "=");
		if (n < 0)
			my_log_error("invalid file %s (line %s)\n", filename, line);
		char *key = my_subStringI_fromTo2(line, 0, n, true);
		char *value = my_subStringI_fromTo2(line, n + 1, -1, true);
		if (my_mapStringObj_add(map, key, value) != NULL)
			my_log_error("file %s has duplicated key '%s'\n", filename, key);
	}
	my_lreader_close(reader, true);
	return map;
}

void my_io_saveLinesFile(const char *filename, MyVectorString *lines) {
	FILE *out = my_io_openFileWrite1(filename);
	for (int64_t i = 0; i < my_vectorString_size(lines); ++i) {
		char *line = my_vectorString_get(lines, i);
		fprintf(out, "%s\n", line);
	}
	fclose(out);
}
void my_io_saveTextFile(const char *filename, const char *text) {
	FILE *out = my_io_openFileWrite1(filename);
	fprintf(out, "%s", text);
	fclose(out);
}
void my_io_saveBytesFile(const char *filename, const void *buffer,
		int64_t num_bytes) {
	FILE *out = my_io_openFileWrite1(filename);
	int64_t n = fwrite(buffer, 1, num_bytes, out);
	my_assert_equalInt("written bytes", n, num_bytes);
	fclose(out);
}
