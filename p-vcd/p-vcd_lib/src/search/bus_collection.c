/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "bus.h"

struct SearchDB {
	DB *db;
	//list of LoadDescriptors*
	MyVectorObj *loaders;
	struct SearchFile **sfiles;
};

struct InternalData {
	struct SearchFile **allFiles_sortedByName;
	struct SearchDB **sdb_by_sfile;
	//list of struct SearchDB*
	MyVectorObj *searchDbs;
};
static struct SearchSegment *newSearchSegment(struct VideoSegment *segment,
		struct SearchFile *sfile) {
	struct SearchSegment *ssegment = MY_MALLOC(1, struct SearchSegment);
	ssegment->segment = segment;
	ssegment->sfile = sfile;
	return ssegment;
}
static struct SearchFile *newSearchFile(FileDB *fdb, LoadSegmentation* loadseg,
		struct SearchDB *sdb) {
	struct SearchFile *sfile = MY_MALLOC(1, struct SearchFile);
	sfile->fdb = fdb;
	sfile->name = my_newString_string(fdb->id);
	sfile->seg = loadSegmentationFileDB(loadseg, fdb);
	sfile->numSegments = sfile->seg->num_segments;
	sfile->ssegments = MY_MALLOC(sfile->seg->num_segments,
			struct SearchSegment *);
	for (int64_t j = 0; j < sfile->seg->num_segments; ++j) {
		struct VideoSegment *vsegment = sfile->seg->segments + j;
		sfile->ssegments[j] = newSearchSegment(vsegment, sfile);
	}
	return sfile;
}
static struct SearchDB *newSearchDB(char *dbName,
		MyVectorString *descriptorsAlias) {
	DB *db = loadDB(dbName, true, true);
	struct SearchDB *sdb = MY_MALLOC(1, struct SearchDB);
	sdb->db = db;
	sdb->sfiles = MY_MALLOC(db->numFilesDb, struct SearchFile*);
	sdb->loaders = my_vectorObj_new();
	LoadDescriptors *first = NULL;
	for (int64_t i = 0; i < my_vectorString_size(descriptorsAlias); ++i) {
		char *descAlias = my_vectorString_get(descriptorsAlias, i);
		LoadDescriptors *loader = newLoadDescriptors(db, descAlias);
		my_vectorObj_add(sdb->loaders, loader);
		if (i == 0)
			first = loader;
		//check compatible modalities (same segmentation)
		const char* s1 = loadDescriptors_getSegmentation(first);
		const char* s2 = loadDescriptors_getSegmentation(loader);
		if (s2 != NULL || s1 != NULL)
			my_assert_equalString("segmentation", s2, s1);
	}
	for (int64_t i = 0; i < db->numFilesDb; ++i) {
		FileDB *fdb = db->filesDb[i];
		LoadSegmentation* loadseg = loadDescriptors_getLoadSegmentation(first);
		sdb->sfiles[i] = newSearchFile(fdb, loadseg, sdb);
	}
	return sdb;
}
static void assignIds(struct SearchCollection *col) {
	col->totalSegments = 0;
	int64_t i;
	for (i = 0; i < col->numFiles; ++i) {
		struct SearchFile *sfile = col->sfiles[i];
		col->totalSegments += sfile->numSegments;
	}
	col->allSegments = MY_MALLOC_NOINIT(col->totalSegments,
			struct SearchSegment *);
	int64_t cont = 0;
	for (i = 0; i < col->numFiles; ++i) {
		struct SearchFile *sfile = col->sfiles[i];
		sfile->id_seq = i;
		sfile->id_seq_segment_first = cont;
		int64_t j;
		for (j = 0; j < sfile->numSegments; ++j) {
			struct SearchSegment *sseg = sfile->ssegments[j];
			my_assert_equalInt("num_segment", sseg->segment->num_segment, j);
			col->allSegments[cont] = sseg;
			cont++;
		}
	}
	my_assert_equalInt("cont", cont, col->totalSegments);
}
static int qsort_compare_SearchFile(const void *a, const void *b) {
	struct SearchFile *o1 = *(struct SearchFile**) a;
	struct SearchFile *o2 = *(struct SearchFile**) b;
	return strcmp(o1->name, o2->name);
}
static void joinFiles(struct SearchCollection *col) {
	struct InternalData *id = col->internalData;
	int64_t i, numFiles = 0;
	for (i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		numFiles += sdb->db->numFilesDb;
	}
	my_assert_greaterInt("numFiles", numFiles, 0);
	col->numFiles = numFiles;
	col->sfiles = MY_MALLOC(numFiles, struct SearchFile*);
	id->allFiles_sortedByName = MY_MALLOC(numFiles, struct SearchFile*);
	id->sdb_by_sfile = MY_MALLOC(numFiles, struct SearchDB*);
	int64_t pos = 0;
	for (i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		int64_t j;
		for (j = 0; j < sdb->db->numFilesDb; ++j) {
			struct SearchFile *sfile = sdb->sfiles[j];
			sfile->col = col;
			col->sfiles[pos] = id->allFiles_sortedByName[pos] = sfile;
			id->sdb_by_sfile[pos] = sdb;
			pos++;
		}
	}
	my_assert_equalInt("pos", pos, numFiles);
	qsort(id->allFiles_sortedByName, col->numFiles, sizeof(struct SearchFile*),
			qsort_compare_SearchFile);
	for (i = 1; i < col->numFiles; ++i) {
		char *name1 = id->allFiles_sortedByName[i - 1]->name;
		char *name2 = id->allFiles_sortedByName[i]->name;
		int cmp = strcmp(name1, name2);
		if (cmp == 0)
			my_log_error("duplicated video name %s\n", name1);
		if (cmp > 0)
			my_log_error("error at sorting %s > %s\n", name1, name2);
	}
}
static void mergeModalities(struct SearchCollection *col) {
	struct InternalData *id = col->internalData;
	struct SearchDB *sdb0 = my_vectorObj_get(id->searchDbs, 0);
	col->numModalities = my_vectorObj_size(sdb0->loaders);
	col->modalities = MY_MALLOC(col->numModalities, DescriptorType);
	for (int64_t i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		my_assert_equalInt("numModalities", my_vectorObj_size(sdb->loaders),
				col->numModalities);
		for (int64_t j = 0; j < col->numModalities; ++j) {
			LoadDescriptors *loader = my_vectorObj_get(sdb->loaders, j);
			DescriptorType td = loadDescriptors_getDescriptorType(loader);
			if (i == 0) {
				col->modalities[j] = td;
			} else {
				//check compatible modalities (identical type)
				assertEqualDescriptorType(col->modalities[j], td);
			}
		}
	}
}
struct SearchCollection *loadSearchCollection(
		struct LoadSearchCollectionParams *params) {
	MyVectorObj *searchDbs = my_vectorObj_new();
	for (int64_t i = 0; i < my_vectorString_size(params->dbsNames); ++i) {
		char *dbName = my_vectorString_get(params->dbsNames, i);
		struct SearchDB *sdb = newSearchDB(dbName, params->descriptorsAlias);
		my_vectorObj_add(searchDbs, sdb);
	}
	struct SearchCollection *col = MY_MALLOC(1, struct SearchCollection);
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < my_vectorObj_size(searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(searchDbs, i);
		char *name = my_subStringC_lastEnd(sdb->db->pathBase, '/');
		if (i > 0)
			my_stringbuf_appendString(sb, "+");
		my_stringbuf_appendString(sb, name);
		MY_FREE(name);
	}
	col->collectionName = my_stringbuf_releaseReturnBuffer(sb);
	struct InternalData *id = MY_MALLOC(1, struct InternalData);
	id->searchDbs = searchDbs;
	col->internalData = id;
	mergeModalities(col);
	joinFiles(col);
	assignIds(col);
	my_log_info(
			"%s: %"PRIi64" files, %"PRIi64" objects, %"PRIi64" modalities\n",
			col->collectionName, col->numFiles, col->totalSegments,
			col->numModalities);
	return col;
}
static void bindDescriptorsFile(struct SearchFile *sfile,
		struct DescriptorsFile *df, int64_t numModalities, int64_t idModality) {
	my_assert_equalInt("numDescriptors", sfile->numSegments,
			df->numDescriptors);
	for (int64_t k = 0; k < sfile->numSegments; ++k) {
		if (numModalities > 1) {
			if (idModality == 0)
				sfile->ssegments[k]->descriptor = MY_MALLOC(numModalities,
						void*);
			void **descriptors = sfile->ssegments[k]->descriptor;
			descriptors[idModality] = df->descriptors[k];
		} else {
			sfile->ssegments[k]->descriptor = df->descriptors[k];
		}
	}
}
static void unbindDescriptorsFile(struct SearchFile *sfile,
		int64_t numModalities) {
	int64_t k;
	for (k = 0; k < sfile->numSegments; ++k) {
		if (numModalities > 1)
			MY_FREE(sfile->ssegments[k]->descriptor);
		sfile->ssegments[k]->descriptor = NULL;
	}
}
static char *text_numDescriptorsLocalVectors(struct SearchSegment **segments,
		int64_t num_segments, int64_t num_modalities,
		DescriptorType *modalities) {
	int64_t total_descriptors = 0;
	int64_t total_vectors = 0;
	for (int64_t i = 0; i < num_segments; ++i) {
		for (int64_t j = 0; j < num_modalities; ++j) {
			void *descriptor = NULL;
			if (num_modalities == 1)
				descriptor = segments[i]->descriptor;
			else
				descriptor = ((void**) (segments[i]->descriptor))[j];
			if (modalities[j].dtype == DTYPE_LOCAL_VECTORS)
				total_vectors += my_localDescriptors_getNumDescriptors(
						descriptor);
			if (descriptor != NULL)
				total_descriptors++;
		}
	}
	if (total_vectors > 0)
		return my_newString_format(", %"PRIi64" vectors, %"PRIi64" descriptors",
				total_vectors, total_descriptors);
	else if (total_descriptors > 0)
		return my_newString_format(", %"PRIi64" descriptors", total_descriptors);
	else
		return my_newString_string("");
}

void loadDescriptorsInCollection(struct SearchCollection *col) {
	struct InternalData *id = col->internalData;
	my_log_info_time("loading descriptors in %s\n", col->collectionName);
	int64_t numBytes = 0;
	for (int64_t i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		for (int64_t j = 0; j < my_vectorObj_size(sdb->loaders); ++j) {
			LoadDescriptors *loader = my_vectorObj_get(sdb->loaders, j);
			struct DescriptorsDB *ddb = loadDescriptorsDB(loader);
			for (int64_t k = 0; k < sdb->db->numFilesDb; ++k) {
				struct SearchFile *sfile = sdb->sfiles[k];
				struct DescriptorsFile *df = ddb->allFiles[k];
				bindDescriptorsFile(sfile, df, col->numModalities, j);
			}
			numBytes += ddb->size_bytes_total;
		}
	}
	char *msg = text_numDescriptorsLocalVectors(col->allSegments,
			col->totalSegments, col->numModalities, col->modalities);
	char *sb = my_newString_diskSpace(numBytes);
	my_log_info_time("loaded descriptors in %s: %s%s\n", col->collectionName,
			sb, msg);
	MY_FREE_MULTI(msg, sb);
}
void unloadDescriptorsInCollection(struct SearchCollection *col) {
	struct InternalData *id = col->internalData;
	for (int64_t i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		for (int64_t k = 0; k < sdb->db->numFilesDb; ++k) {
			struct SearchFile *sfile = sdb->sfiles[k];
			unbindDescriptorsFile(sfile, col->numModalities);
		}
		for (int64_t k = 0; k < my_vectorObj_size(sdb->loaders); ++k) {
			LoadDescriptors *loader = my_vectorObj_get(sdb->loaders, k);
			releaseLoadDescriptors_allFileBytes(loader);
		}
	}
}
void loadDescriptorsInSearchFile(struct SearchFile *sfile) {
	struct SearchCollection *col = sfile->col;
	struct InternalData *id = col->internalData;
	struct SearchDB *sdb = id->sdb_by_sfile[sfile->id_seq];
	int64_t numBytes = 0;
	for (int64_t j = 0; j < my_vectorObj_size(sdb->loaders); ++j) {
		LoadDescriptors *loader = my_vectorObj_get(sdb->loaders, j);
		struct DescriptorsFile *df = loadDescriptorsFileDB(loader, sfile->fdb);
		bindDescriptorsFile(sfile, df, col->numModalities, j);
		numBytes += df->size_bytes;
	}
	char *msg = text_numDescriptorsLocalVectors(sfile->ssegments,
			sfile->numSegments, col->numModalities, col->modalities);
	char *sb = my_newString_diskSpace(numBytes);
	my_log_info("loaded descriptors in %s: %s%s\n", sfile->name, sb, msg);
	MY_FREE_MULTI(msg, sb);
}
void unloadDescriptorsInSearchFile(struct SearchFile *sfile) {
	struct SearchCollection *col = sfile->col;
	struct InternalData *id = col->internalData;
	struct SearchDB *sdb = id->sdb_by_sfile[sfile->id_seq];
	unbindDescriptorsFile(sfile, col->numModalities);
	for (int64_t k = 0; k < my_vectorObj_size(sdb->loaders); ++k) {
		LoadDescriptors *loader = my_vectorObj_get(sdb->loaders, k);
		releaseLoadDescriptors_allFileBytes(loader);
	}
}
//////////////////////////////
#if 0
static int64_t priv_verSegCompatible(Segmentation *seg1, Segmentation *seg2,
		int64_t minSegmentos, struct ColFileTxt *fuenteS1,
		struct ColFileTxt *fuenteS2) {
	int64_t i, numIncomp = 0;
	for (i = 0; i < minSegmentos; ++i) {
		VideoSegment s1 = seg1->ssegments[i];
		VideoSegment s2 = seg2->ssegments[i];
		if (!MY_INTERSECT(s1.seg_desde, s1.seg_hasta, s2.seg_desde, s2.seg_hasta)) {
			if (0)
			my_log_info(
					"incompatible segments: %1.2lf-%1.2lf vs %1.2lf-%1.2lf (%s %s)\n",
					s1.seg_desde, s1.seg_hasta, s2.seg_desde, s2.seg_hasta,
					fuenteS1->fdb->id, fuenteS2->fdb->id);
			numIncomp++;
		}
	}
	return numIncomp;
}
#endif

void reorderSearchFiles(struct SearchCollection* col, char *criterion) {
	my_log_info("reordering search files %s in %s\n", criterion,
			col->collectionName);
	struct SearchFile **newFileList = MY_MALLOC(col->numFiles,
			struct SearchFile*);
	if (my_string_equals(criterion, "RANDOM")) {
		int64_t i, *permutation = my_random_newPermutation(0, col->numFiles);
		for (i = 0; i < col->numFiles; ++i) {
			newFileList[i] = col->sfiles[permutation[i]];
		}
		MY_FREE(permutation);
	} else if (my_string_startsWith(criterion, "PERMUT_")) {
		char *file = my_subStringC_firstEnd(criterion, '_');
		MyLineReader *reader = my_lreader_open(my_io_openFileRead1(file, 1));
		int64_t numLine = 0;
		const char *line;
		while ((line = my_lreader_readLine(reader)) != NULL) {
			int64_t oldPos = my_parse_int(line);
			my_assert_indexRangeInt("position", oldPos, col->numFiles);
			my_assert_indexRangeInt("numLine", numLine, col->numFiles);
			newFileList[numLine] = col->sfiles[oldPos];
			numLine++;
		}
		my_lreader_close(reader, true);
		my_assert_equalInt("reordering", numLine, col->numFiles);
	} else if (my_string_startsWith(criterion, "NAMES_")) {
		char *file = my_subStringC_firstEnd(criterion, '_');
		MyLineReader *reader = my_lreader_open(my_io_openFileRead1(file, 1));
		int64_t numLine = 0;
		const char *line;
		while ((line = my_lreader_readLine(reader)) != NULL) {
			struct SearchFile *arc = findSearchFileInCollection(col, line, 1);
			my_assert_indexRangeInt("numLine", numLine, col->numFiles);
			newFileList[numLine] = arc;
			numLine++;
		}
		my_lreader_close(reader, true);
		my_assert_equalInt("reordering", numLine, col->numFiles);
	} else {
		my_log_error("unknown order %s\n", criterion);
	}
	MY_FREE(col->sfiles);
	col->sfiles = newFileList;
	assignIds(col);
}
void reorderSearchObjects(struct SearchCollection* col, char *criterion) {
	my_log_info("reordering search objects %s in %s\n", criterion,
			col->collectionName);
	struct SearchSegment **newObjectsList = MY_MALLOC(col->totalSegments,
			struct SearchSegment*);
	if (my_string_equals(criterion, "RANDOM")) {
		int64_t i, *permutation = my_random_newPermutation(0,
				col->totalSegments);
		for (i = 0; i < col->totalSegments; ++i) {
			newObjectsList[i] = col->allSegments[permutation[i]];
		}
		MY_FREE(permutation);
	} else if (my_string_startsWith(criterion, "PERMUT_")) {
		char *file = my_subStringC_firstEnd(criterion, '_');
		MyLineReader *reader = my_lreader_open(my_io_openFileRead1(file, 1));
		int64_t numLine = 0;
		const char *line;
		while ((line = my_lreader_readLine(reader)) != NULL) {
			int64_t oldPos = my_parse_int(line);
			my_assert_indexRangeInt("position", oldPos, col->totalSegments);
			my_assert_indexRangeInt("numLine", numLine, col->totalSegments);
			newObjectsList[numLine] = col->allSegments[oldPos];
			numLine++;
		}
		my_lreader_close(reader, true);
		my_assert_equalInt("reordering", numLine, col->totalSegments);
	} else if (my_string_startsWith(criterion, "NAMES_")) {
		char *file = my_subStringC_firstEnd(criterion, '_');
		MyLineReader *reader = my_lreader_open(my_io_openFileRead1(file, 1));
		int64_t numLine = 0;
		const char *line;
		while ((line = my_lreader_readLine(reader)) != NULL) {
			struct SearchSegment *kf = findSearchSegmentInCollection(col, line,
					1);
			my_assert_indexRangeInt("numLine", numLine, col->totalSegments);
			newObjectsList[numLine] = kf;
			numLine++;
		}
		my_lreader_close(reader, true);
		my_assert_equalInt("reordering", numLine, col->totalSegments);
	} else {
		my_log_error("unknown reorder %s\n", criterion);
	}
	MY_FREE(col->allSegments);
	col->allSegments = newObjectsList;
}
static void releaseSearchDB(struct SearchDB *sdb) {
	for (int64_t i = 0; i < sdb->db->numFilesDb; ++i) {
		struct SearchFile* sfile = sdb->sfiles[i];
		for (int64_t j = 0; j < sfile->numSegments; ++j) {
			struct SearchSegment *sseg = sfile->ssegments[j];
			MY_FREE(sseg);
		}
		MY_FREE_MULTI(sfile->ssegments, sfile->name, sfile);
	}
	MY_FREE(sdb->sfiles);
	for (int64_t i = 0; i < my_vectorObj_size(sdb->loaders); ++i) {
		LoadDescriptors *dd = my_vectorObj_get(sdb->loaders, i);
		releaseLoadDescriptors(dd);
	}
	my_vectorObj_release(sdb->loaders, 0);
}
void releaseCollection(struct SearchCollection* col) {
	struct InternalData *id = col->internalData;
	int64_t i;
	for (i = 0; i < my_vectorObj_size(id->searchDbs); ++i) {
		struct SearchDB *sdb = my_vectorObj_get(id->searchDbs, i);
		releaseSearchDB(sdb);
	}
	my_vectorObj_release(id->searchDbs, 0);
	MY_FREE_MULTI(id->sdb_by_sfile, id->allFiles_sortedByName, id);
	MY_FREE_MULTI(col->sfiles, col->allSegments, col->modalities,
			col->collectionName, col);
}
char *toString_SearchFile(struct SearchFile *sfile) {
	return my_newString_string(sfile->name);
}
char *toString_SearchSegmentNoFile(struct SearchSegment *sseg) {
	return my_newString_int(sseg->segment->num_segment);
}
char *toString_SearchSegmentAndFile(struct SearchSegment *sseg) {
	char *stArc = toString_SearchFile(sseg->sfile);
	if (sseg->sfile->numSegments == 1) {
		return stArc;
	} else {
		char *stSeg = toString_SearchSegmentNoFile(sseg);
		char *st = my_newString_format("%s@%s", stArc, stSeg);
		MY_FREE_MULTI(stArc, stSeg);
		return st;
	}
}
static int compare_SearchFile_string(const void *search_object,
		const void *array_object) {
	const char *ssfile_string = search_object;
	const struct SearchFile *obj2 = array_object;
	return strcmp(ssfile_string, obj2->name);
}
struct SearchFile *findSearchFileInCollection(struct SearchCollection *col,
		const char *ssfile_string, bool fail) {
	if (ssfile_string == NULL) {
		if (fail)
			my_assert_notNull("name", ssfile_string);
		return NULL;
	}
	struct InternalData *id = col->internalData;
	int64_t pos = -1;
	if (!my_binsearch_objArr(ssfile_string, (void**) id->allFiles_sortedByName,
			col->numFiles, compare_SearchFile_string, &pos)) {
		if (fail)
			my_log_error("can't find video '%s'\n", ssfile_string);
		return NULL;
	}
	my_assert_indexRangeInt("pos", pos, col->numFiles);
	return id->allFiles_sortedByName[pos];
}

struct SearchSegment *findSearchSegmentInFile(struct SearchFile* ssfile,
		const char *sseg_string, bool fail) {
	if ((sseg_string == NULL || strlen(sseg_string) == 0)
			&& ssfile->numSegments == 1)
		return ssfile->ssegments[0];
	int64_t num_segment = my_parse_int(sseg_string);
	if (fail)
		my_assert_indexRangeInt("num_segment", num_segment,
				ssfile->numSegments);
	if (MY_BETWEEN(num_segment, 0, ssfile->numSegments - 1))
		return ssfile->ssegments[num_segment];
	return NULL;
}

struct SearchSegment *findSearchSegmentInCollection(
		struct SearchCollection *col, const char *sseg_string, bool fail) {
	if (sseg_string == NULL) {
		if (fail)
			my_assert_notNull("name", sseg_string);
		return NULL;
	}
	int64_t pos1 = my_string_indexOf(sseg_string, "|");
	if (pos1 > 0) {
		char *name = my_subStringI_fromTo(sseg_string, 0, pos1);
		struct SearchSegment *sseg = findSearchSegmentInCollection(col, name,
				fail);
		MY_FREE(name);
		return sseg;
	}
	int64_t pos2 = my_string_indexOf(sseg_string, "@");
	if (pos2 < 0) {
		struct SearchFile *ssfile = findSearchFileInCollection(col, sseg_string,
				fail);
		if (ssfile == NULL)
			return NULL;
		if (ssfile->numSegments != 1)
			my_log_error("video '%s' must contain only one segment\n",
					sseg_string);
		return ssfile->ssegments[0];
	} else {
		char *sfile_name = my_subStringI_fromTo(sseg_string, 0, pos2);
		char *sseg_name = my_subStringI_fromEnd(sseg_string, pos2 + 1);
		struct SearchFile *sfile = findSearchFileInCollection(col, sfile_name,
				fail);
		struct SearchSegment *sseg = NULL;
		if (sfile != NULL)
			sseg = findSearchSegmentInFile(sfile, sseg_name, fail);
		MY_FREE_MULTI(sfile_name, sseg_name);
		return sseg;
	}
}
