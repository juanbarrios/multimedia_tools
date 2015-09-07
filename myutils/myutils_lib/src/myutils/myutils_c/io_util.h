/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_IO_UTIL_H
#define MY_IO_UTIL_H

#include "../myutils_c.h"

void my_io_removeInvalidChars(char *string);
char *my_io_getFilename(const char *filePath);
char *my_io_getFilenameWithoutExtension(const char *filePath);
char *my_io_getFilenameExtension(const char *filePath);

char *my_io_normalizeFilenameToRead(const char *filename);
char *my_io_normalizeFilenameToCreate(const char *filename);

char *my_io_getDirname(const char* filePath);
char *my_io_getAbsolutPath(const char *relativePath);

FILE *my_io_openFileRead1(const char *filename, bool failOnError);
FILE *my_io_openFileRead2(const char *path, const char *file, bool failOnError);
FILE *my_io_openFileRead3(const char *path, const char *name,
		const char *extension,
		bool failOnError);
FILE *my_io_openFileWrite1(const char *filename);
FILE *my_io_openFileWrite2(const char *path, const char *file);
FILE *my_io_openFileWrite3(const char *path, const char *name,
		const char *extension);
FILE *my_io_openFileWrite1Config(const char *filename,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version);
FILE *my_io_openFileWrite2Config(const char *path, const char *file,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version);
FILE *my_io_openFileAppend1(const char *filename);
FILE *my_io_openFileAppend2(const char *path, const char *file);

char *my_io_detectFileConfig(const char *filename, const char *software_name);

char *my_io_getConfigFileHeader(const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version);

void my_io_readBytesFile(FILE *in, void *buffer, int64_t numBytes,
bool validateEOF);
void* my_io_loadFileBytes(const char *filename, int64_t *out_filesize);
bool my_io_deleteFile(const char* filename, bool fail);
bool my_io_moveFile(const char* filenameOrig, const char* filenameDest, bool fail);
void my_io_copyFile(const char* filenameOrig, const char* filenameDest);
bool my_io_existsFile(const char* filename);
bool my_io_existsFile2(const char *ruta, const char* file);
bool my_io_existsDir(const char* dirname);
bool my_io_notExists(const char* filename);
bool my_io_notExists2(const char *path, const char* file);
int64_t my_io_getFilesize(const char* filename);
int64_t my_io_getFilesize2(FILE *file);
void my_io_createDir(const char *path, bool fail);
void my_io_createParentDir(const char *file_path);

MyVectorString *my_io_listFilesInDir(const char *path_dir);
MyVectorString *my_io_listDirsInDir(const char *path_dir);
MyVectorString *my_io_listFilesInDirsRecursive(MyVectorString *paths,
bool recursive);
MyVectorString *my_io_listFilesWithPrefix(const char *path_prefix);
MyVectorString *my_io_listFilesWithSuffix(const char *path_dir,
		const char *suffix);

MyVectorString *my_io_loadLinesFile(const char *filename);
MyVectorString *my_io_loadLinesFileConfig(const char *filename, const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version);

MyMapStringObj *my_io_loadProperties(const char *filename, bool fail,
		const char *module_name, const char *format_file, int64_t major_version,
		int64_t minor_version);

void my_io_saveLinesFile(const char *filename, MyVectorString *lines);
void my_io_saveTextFile(const char *filename, const char *text);
void my_io_saveBytesFile(const char *filename, const void *buffer, int64_t num_bytes);

#endif
