/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "saveSegmentation.h"

char *getFilenameSegmentationSeg(const char *segment_dir, const char *file_id) {
	return my_newString_format("%s/%s.seg", segment_dir, file_id);
}
char *getSegmentationDes(const char *segment_dir) {
	return my_newString_format("%s/segmentation.des", segment_dir);
}
struct SaveSegmentation {
	char *output_dir;
	char *segmentation_name;
};

SaveSegmentation *newSaveSegmentation(const char *output_dir,
		const char *segmentation_name) {
	SaveSegmentation *saver = MY_MALLOC(1, SaveSegmentation);
	saver->output_dir = my_newString_string(output_dir);
	saver->segmentation_name = my_newString_string(segmentation_name);
	return saver;
}
bool saveSegmentation_existsFile(SaveSegmentation *saver, const char *file_id) {
	char * fname = getFilenameSegmentationSeg(saver->output_dir, file_id);
	bool is = (my_io_existsFile(fname) && my_io_getFilesize(fname) > 0);
	free(fname);
	return is;
}
void saveSegmentation(SaveSegmentation *saver, const char *file_id,
		struct Segmentation *seg) {
	my_io_createDir(saver->output_dir, false);
	char *fname = getFilenameSegmentationSeg(saver->output_dir, file_id);
	FILE* out = my_io_openFileWrite1Config(fname, "PVCD", "FileSegmentation", 1,
			1);
	free(fname);
	char *sns = my_newString_int(seg->num_segments);
	char *sfps = my_newString_double(seg->video_fps);
	fprintf(out, "%s\t%s\n", sns, sfps);
	MY_FREE_MULTI(sns, sfps);
	for (int64_t j = 0; j < seg->num_segments; ++j) {
		struct VideoSegment *sel = seg->segments + j;
		char *fd = my_newString_int(sel->start_frame);
		char *fs = my_newString_int(sel->selected_frame);
		char *fh = my_newString_int(sel->end_frame);
		char *sd = my_newString_doubleDec(sel->start_second, 3);
		char *ss = my_newString_doubleDec(sel->selected_second, 3);
		char *sh = my_newString_doubleDec(sel->end_second, 3);
		fprintf(out, "%s\t%s\t%s\t%s\t%s\t%s\n", fd, fs, fh, sd, ss, sh);
		MY_FREE_MULTI(fd, fs, fh, sd, ss, sh);
	}
	fclose(out);
}
void releaseSaveSegmentation(SaveSegmentation *saver) {
	char *fname = getSegmentationDes(saver->output_dir);
	FILE *out = my_io_openFileWrite1Config(fname, "PVCD", "SegmentationData", 1,
			0);
	free(fname);
	fprintf(out, "segmentation=%s\n", saver->segmentation_name);
	fclose(out);
	MY_FREE_MULTI(saver->output_dir, saver);
}

char *print_VideoSegment(struct VideoSegment *s) {
	return my_newString_format(
			"%"PRIi64" (%"PRIi64"-%"PRIi64") %1.1lf (%1.1lf-%1.1lf)",
			s->selected_frame, s->start_frame, s->end_frame, s->selected_second,
			s->start_second, s->end_second);
}
