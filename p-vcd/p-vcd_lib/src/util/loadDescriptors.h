/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef LOADDESCRIPTORS_H
#define LOADDESCRIPTORS_H

#include "../pvcd.h"

LoadDescriptors *newLoadDescriptors(DB *db, const char *descAlias);

DescriptorType loadDescriptors_getDescriptorType(LoadDescriptors *desloader);
const char* loadDescriptors_getSegmentation(LoadDescriptors *desloader);
const char* loadDescriptors_getDescriptor(LoadDescriptors *desloader);
const char* loadDescriptors_getDescriptorAlias(LoadDescriptors *desloader);
const char* loadDescriptors_getDataDir(LoadDescriptors *desloader);
bool loadDescriptors_getIsSingleFile(LoadDescriptors *desloader);
LoadSegmentation *loadDescriptors_getLoadSegmentation(
		LoadDescriptors *desloader);
DB *loadDescriptors_getDb(LoadDescriptors *desloader);

void releaseLoadDescriptors_allFileBytes(LoadDescriptors *desloader);
void releaseLoadDescriptors(LoadDescriptors *desloader);

struct DescriptorsFile {
	int64_t numDescriptors;
	void **descriptors;
	int64_t size_bytes;
	FileDB *fdb;
	LoadDescriptors *desloader;
};

struct DescriptorsFile* loadDescriptorsFileId(LoadDescriptors *desloader,
		const char *file_id);
struct DescriptorsFile* loadDescriptorsFileDB(LoadDescriptors *desloader,
		FileDB *fdb);
MknnDataset *loadDescriptorsFile_getMknnDataset(struct DescriptorsFile* df,
		double sampleFractionDescriptors, double sampleFractionPerFrame);
void releaseDescriptorsFile(struct DescriptorsFile *df);

struct DescriptorsDB {
	int64_t numFiles;
	struct DescriptorsFile **allFiles;
	int64_t size_bytes_total;
	LoadDescriptors *desloader;
};

struct DescriptorsDB* loadDescriptorsDB(LoadDescriptors *desloader);
void releaseDescriptorsDB(struct DescriptorsDB* ddb);

#endif
