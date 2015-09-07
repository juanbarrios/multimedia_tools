/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "loadSegmentation.h"

struct LoadSegmentation {
	char *segmentations_dir;
	char *segAlias, *segmentation;
	DB *db;
	struct Segmentation **loaded_segmentations_by_file;
	struct Segmentation *default_image_segmentation;
};

LoadSegmentation *newLoadSegmentation(DB *db, const char *segAlias) {
	LoadSegmentation *loader = MY_MALLOC(1, LoadSegmentation);
	loader->db = db;
	loader->loaded_segmentations_by_file = MY_MALLOC(db->numFilesDb,
			struct Segmentation *);
	if (segAlias != NULL) {
		loader->segmentations_dir = my_newString_format("%s/%s",
				db->pathSegmentations, segAlias);
		char *fname = getSegmentationDes(loader->segmentations_dir);
		MyMapStringObj *prop = my_io_loadProperties(fname, true, "PVCD",
				"SegmentationData", 1, 0);
		free(fname);
		loader->segAlias = my_newString_string(segAlias);
		loader->segmentation = my_newString_string(
				my_mapStringObj_get(prop, "segmentation"));
		my_mapStringObj_release(prop, true, true);
	}
	return loader;
}
void releaseLoadSegmentation(LoadSegmentation *loader) {
	for (int64_t i = 0; i < loader->db->numFilesDb; ++i) {
		if (loader->loaded_segmentations_by_file[i] != NULL)
			releaseSegmentation(loader->loaded_segmentations_by_file[i]);
	}
	if (loader->default_image_segmentation == NULL)
		releaseSegmentation(loader->default_image_segmentation);
	MY_FREE_MULTI(loader->segmentations_dir, loader->segAlias,
			loader->segmentation, loader);
}
DB *loadSegmentation_getDb(LoadSegmentation *loader) {
	return loader->db;
}
void validateSegmentation(const struct Segmentation *seg) {
	my_assert_notNull("segmentation", seg);
	my_assert_greaterInt("num_segments", seg->num_segments, 0);
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		struct VideoSegment *s = seg->segments + i;
		if (s->start_frame > s->selected_frame
				|| s->selected_frame > s->end_frame
				|| s->start_second > s->selected_second
				|| s->selected_second > s->end_second)
			my_log_error("invalid segment %s\n", print_VideoSegment(s));
		if (i == 0) {
			my_assert_greaterEqual_int("start segmentation", s->start_frame, 0);
			my_assert_greaterEqualDouble("start segmentation", s->start_second,
					0);
		} else {
			struct VideoSegment *sPrev = seg->segments + (i - 1);
			my_assert_greaterEqual_int("overlap frames", s->selected_frame,
					sPrev->selected_frame);
			my_assert_greaterEqualDouble("overlap seconds", s->selected_second,
					sPrev->selected_second);
			my_assert_greaterEqual_int("overlap frames", s->start_frame,
					sPrev->end_frame);
			my_assert_greaterEqualDouble("overlap seconds", s->start_second,
					sPrev->end_second);
		}
	}
}
void contFramesAndSecondsInSegmentation(const struct Segmentation *seg,
		int64_t *out_cont_frames, double *out_cont_seconds) {
	int64_t cont_frames = 0;
	double cont_seconds = 0;
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		struct VideoSegment *s = seg->segments + i;
		cont_frames += s->end_frame - s->start_frame + 1;
		cont_seconds += s->end_second - s->start_second;
	}
	*out_cont_frames = cont_frames;
	*out_cont_seconds = cont_seconds;
}

static struct Segmentation *parseSegmentationFile(const char *fileSegmentation) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(fileSegmentation, true), "PVCD",
			"FileSegmentation", 1, 1);
	if (reader == NULL)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(my_lreader_readLine(reader), '\t');
	int64_t total_segments = my_tokenizer_nextInt(tk);
	double fps = my_tokenizer_nextDouble(tk);
	my_tokenizer_release(tk);
	struct Segmentation *seg = createNewSegmentation(total_segments, fps);
	my_assert_greaterInt("total_segments", seg->num_segments, 0);
	for (int64_t i = 0; i < seg->num_segments; ++i) {
		const char *line = my_lreader_readLine(reader);
		MyTokenizer *tk2 = my_tokenizer_new(line, '\t');
		struct VideoSegment *s = seg->segments + i;
		s->num_segment = i;
		s->start_frame = my_tokenizer_nextInt(tk2);
		s->selected_frame = my_tokenizer_nextInt(tk2);
		s->end_frame = my_tokenizer_nextInt(tk2);
		s->start_second = my_tokenizer_nextDouble(tk2);
		s->selected_second = my_tokenizer_nextDouble(tk2);
		s->end_second = my_tokenizer_nextDouble(tk2);
		my_tokenizer_releaseValidateEnd(tk2);
	}
	validateSegmentation(seg);
	my_lreader_close(reader, true);
	return seg;
}
static struct Segmentation *createDefaultImageSegmentation() {
	struct Segmentation *seg = createNewSegmentation(1, 1);
	struct VideoSegment * s = seg->segments;
	s->start_second = 0;
	s->end_second = 1;
	s->selected_second = 0.5;
	s->start_frame = s->end_frame = s->selected_frame = 0;
	validateSegmentation(seg);
	return seg;
}
static struct Segmentation *createDefaultVideoSegmentation(FileDB *fdb) {
	struct Segmentation *seg = createNewSegmentation(1, 1);
	struct VideoSegment * s = seg->segments;
	s->start_second = fdb->secStartTime;
	s->end_second = fdb->secEndTime;
	s->selected_second = (s->start_second + s->selected_second) / 2;
	s->start_frame = my_math_round_int(s->start_second * fdb->numObjsPerSecond);
	s->end_frame = my_math_round_int(s->end_second * fdb->numObjsPerSecond);
	s->selected_frame = (s->start_frame + s->end_frame) / 2;
	validateSegmentation(seg);
	return seg;
}
const struct Segmentation* loadSegmentationFileId(LoadSegmentation *loader,
		const char *file_id) {
	FileDB *fdb = findFileDB_byId(loader->db, file_id, true);
	return loadSegmentationFileDB(loader, fdb);
}
const struct Segmentation* loadSegmentationFileDB(LoadSegmentation *loader,
		FileDB *fdb) {
	if (fdb->isImage) {
		if (loader->default_image_segmentation == NULL)
			loader->default_image_segmentation =
					createDefaultImageSegmentation();
		return loader->default_image_segmentation;
	}
	struct Segmentation *fb =
			loader->loaded_segmentations_by_file[fdb->internal_id];
	if (fb != NULL)
		return fb;
	if (loader->segmentations_dir == NULL) {
		struct Segmentation *ss = createDefaultVideoSegmentation(fdb);
		loader->loaded_segmentations_by_file[fdb->internal_id] = ss;
		return ss;
	}
	char *filenameSeg = getFilenameSegmentationSeg(loader->segmentations_dir,
			fdb->id);
	if (!my_io_existsFile(filenameSeg))
		my_log_error("can't find segmentation %s for file %s\n",
				loader->segmentations_dir, fdb->id);
	struct Segmentation *ss = parseSegmentationFile(filenameSeg);
	free(filenameSeg);
	loader->loaded_segmentations_by_file[fdb->internal_id] = ss;
	return ss;
}

struct Segmentation *createNewSegmentationFileDB(int64_t total_segments,
		FileDB *fdb) {
	return createNewSegmentation(total_segments, fdb->numObjsPerSecond);
}

struct Segmentation *createNewSegmentation(int64_t total_segments,
		double video_fps) {
	struct Segmentation *seg = MY_MALLOC(1, struct Segmentation);
	seg->num_segments = total_segments;
	seg->video_fps = video_fps;
	seg->segments = MY_MALLOC(seg->num_segments, struct VideoSegment);
	return seg;

}
void releaseSegmentation(struct Segmentation *seg) {
	if (seg == NULL)
		return;
	MY_FREE(seg->segments);
	MY_FREE(seg);
}
