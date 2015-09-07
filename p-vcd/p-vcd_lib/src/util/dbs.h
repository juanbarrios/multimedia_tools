/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef DBS_H
#define DBS_H

#include "../pvcd.h"

FileDB *findFileDB_byId(DB *db, const char *id, bool fail);
MyVectorObj *findFilesDB_byFilenameReal(DB *db, const char *pathReal);
const char *getDbLoadOptions();
DB *loadDB(const char *dbName, bool fail, bool loadFiles);
FileDB *loadFileDB(const char *nameDB, const char *idFile, bool fail);
void releaseDB(DB *db);
void releaseFileDB(FileDB *fdb);
FILE *newDBfile(DB *db);
void addFileDB_toDB(FILE *out, FileDB *fdb);
MyVectorString *getAllSegmentationsInDb(DB *db);

void pvcd_register_audio_fileExtensions(const char *extensions);
void pvcd_register_image_fileExtensions(const char *extensions);
void pvcd_register_video_fileExtensions(const char *extensions);
void pvcd_register_default_fileExtensions();
bool is_image_ext(const char *filename);
bool is_video_ext(const char *filename);
bool is_audio_ext(const char *filename);

#endif
