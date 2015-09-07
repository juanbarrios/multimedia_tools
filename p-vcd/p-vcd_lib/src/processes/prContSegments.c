/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

static void cont_new(const char *segCode, const char *segParameters,
		void **out_state) {
}
static void seg_print_summary(DB *db, int64_t framesDb, int64_t segmentsDb,
		double secondsDb, double secondsSegm, double framesSegm) {
	if (framesDb == 0)
		return;
	char *st0 =
			(db == NULL) ?
					my_newString_string("TOTAL") :
					my_io_getFilename(db->pathBase);
	char *st1 = my_newString_int(framesDb);
	char *st2 = my_newString_int(segmentsDb);
	char *st3 = my_newString_hhmmss(secondsDb);
	char *st4 = my_newString_doubleDec(secondsSegm, 2);
	char *st5 = my_newString_doubleDec(framesSegm, 1);
	my_log_info(
			"  %-20s segments=%7s  %4s sec/segm  %4s frm/segm videos_length=%5s videos_frames=%5s\n",
			st0, st2, st4, st5, st3, st1);
	MY_FREE_MULTI(st0, st1, st2, st3, st4, st5);
}
static void print_segmentation(LoadSegmentation *segloader) {
	DB *db = loadSegmentation_getDb(segloader);
	int64_t framesDb = 0, segmentsDb = 0;
	double secondsDb = 0, secondsSegm = 0;
	for (int64_t j = 0; j < db->numFilesDb; ++j) {
		const struct Segmentation *seg = loadSegmentationFileDB(segloader,
				db->filesDb[j]);
		if (seg == NULL)
			continue;
		int64_t cont_frames = 0;
		double cont_seconds = 0;
		contFramesAndSecondsInSegmentation(seg, &cont_frames, &cont_seconds);
		segmentsDb += seg->num_segments;
		framesDb += cont_frames;
		secondsDb += cont_seconds;
		secondsSegm += (cont_seconds / seg->num_segments);
	}
	secondsSegm /= db->numFilesDb;
	double framesSegm = (segmentsDb > 0) ? framesDb / ((double) segmentsDb) : 0;
	seg_print_summary(db, framesDb, segmentsDb, secondsDb, secondsSegm,
			framesSegm);
}
static void cont_process_segmentation(LoadSegmentation *segloader, void *state) {
	print_segmentation(segloader);
}
static void cont_process_db(DB *db, void *state) {
	MyVectorString *dirnames = getAllSegmentationsInDb(db);
	for (int64_t k = 0; k < my_vectorString_size(dirnames); ++k) {
		char *segm_dir = my_vectorString_get(dirnames, k);
		my_log_info("%s:\n", segm_dir);
		LoadSegmentation *segloader = newLoadSegmentation(db, segm_dir);
		print_segmentation(segloader);
		releaseLoadSegmentation(segloader);
	}
	my_vectorString_release(dirnames, false);
}
static void cont_release(void *state) {
}

void proc_reg_cont() {
	addProcessorDefSegmentation("CONTSEGMENTATION", NULL, cont_new,
			cont_process_segmentation, cont_release);
	addProcessorDefDb("CONTALLSEGMENTATIONS", NULL, cont_new, cont_process_db,
			cont_release);
}
