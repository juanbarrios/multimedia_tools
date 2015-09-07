/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef SAVESEGMENTATION_H
#define SAVESEGMENTATION_H

#include "../pvcd.h"

char *getFilenameSegmentationSeg(const char *segment_dir, const char *file_id);
char *getSegmentationDes(const char *segment_dir);

SaveSegmentation *newSaveSegmentation(const char *output_dir,
		const char *segmentation_name);
bool saveSegmentation_textFileExist(SaveDescriptors *saver, const char *file_id);
void saveSegmentation(SaveSegmentation *saver, const char *file_id,
		struct Segmentation *seg);
void releaseSaveSegmentation(SaveSegmentation *saver);

char *print_VideoSegment(struct VideoSegment *s);

#endif
