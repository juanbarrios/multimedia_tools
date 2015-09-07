/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef SAVEDESCRIPTORS_H
#define SAVEDESCRIPTORS_H

#include "../pvcd.h"

char *getFilenameBin(const char *descriptors_dir, const char *file_id);
char *getFilenameTxt(const char *descriptors_dir, const char *file_id);
char *getFilenameSingleBin(const char *descriptors_dir);
char *getFilenameSinglePos(const char *descriptors_dir);
char *getFilenameSingleTxt(const char *descriptors_dir);
char *getFilenameDes(const char *descriptors_dir);

SaveDescriptors *newSaveDescriptors(const char *output_dir, DescriptorType td,
bool saveBinaryFormat, bool saveTxtFormat,
bool useSingleFile, const char *descriptor, const char *segmentation);
bool saveDescriptors_existsFile(SaveDescriptors *dessaver, const char *file_id);
void saveDescriptors(SaveDescriptors *dessaver, const char *file_id,
		int64_t numDescriptors, void **descriptors,
		const struct Segmentation *seg);
void releaseSaveDescriptors(SaveDescriptors *dessaver);

#endif
