/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "loadDescriptors.h"

struct GlobalEntry {
	char *file_id;
	int64_t offset;
	int64_t size;
};
struct LoadedDB {
	MyVectorObj *entryList;
	struct DescriptorsDB* ddb;
	char *file_bytes;
};
struct LoadedFile {
	struct DescriptorsFile *df;
	char *file_bytes_toRelease;
};
struct LoadDescriptors {
	char *descriptors_dir;
	char *descAlias, *descriptor, *segmentation;
	bool isSingleFile;
	DescriptorType td;
	DB *db;
	LoadSegmentation *seg_loader;
	struct LoadedDB loaded_db;
	struct LoadedFile *loaded_files;
};

LoadDescriptors *newLoadDescriptors(DB *db, const char *descAlias) {
	LoadDescriptors *desloader = MY_MALLOC(1, LoadDescriptors);
	desloader->descriptors_dir = my_newString_format("%s/%s",
			db->pathDescriptors, descAlias);
	char *fname = getFilenameDes(desloader->descriptors_dir);
	MyMapStringObj *prop = my_io_loadProperties(fname, true, "PVCD",
			"DescriptorData", 1, 2);
	free(fname);
	desloader->db = db;
	desloader->descAlias = my_newString_string(descAlias);
	desloader->descriptor = my_newString_string(
			my_mapStringObj_get(prop, "descriptor"));
	desloader->segmentation = my_newString_string(
			my_mapStringObj_get(prop, "segmentation"));
	desloader->td.dtype = string2dtype(
			my_mapStringObj_get(prop, "descriptor_type"));
	desloader->td.array_length = my_parse_int(
			my_mapStringObj_get(prop, "array_length"));
	desloader->td.num_subarrays = my_parse_int(
			my_mapStringObj_get(prop, "num_subarrays"));
	desloader->td.subarray_length = my_parse_int(
			my_mapStringObj_get(prop, "subarray_length"));
	desloader->isSingleFile = my_parse_bool(
			my_mapStringObj_get(prop, "single_file"));
	my_mapStringObj_release(prop, true, true);
	desloader->loaded_files = MY_MALLOC(db->numFilesDb, struct LoadedFile);
	return desloader;
}
void releaseLoadDescriptors_allFileBytes(LoadDescriptors *desloader) {
	if (desloader->loaded_db.ddb != NULL) {
		releaseDescriptorsDB(desloader->loaded_db.ddb);
		desloader->loaded_db.ddb = NULL;
	}
	for (int64_t i = 0; i < desloader->db->numFilesDb; ++i) {
		if (desloader->loaded_files[i].df != NULL) {
			releaseDescriptorsFile(desloader->loaded_files[i].df);
			desloader->loaded_files[i].df = NULL;
		}
	}
}
void releaseLoadDescriptors(LoadDescriptors *desloader) {
	if (desloader == NULL)
		return;
	releaseLoadDescriptors_allFileBytes(desloader);
	if (desloader->seg_loader != NULL)
		releaseLoadSegmentation(desloader->seg_loader);
	MY_FREE_MULTI(desloader->descriptors_dir, desloader->descAlias,
			desloader->descriptor, desloader->segmentation, desloader);
}
DescriptorType loadDescriptors_getDescriptorType(LoadDescriptors *desloader) {
	return desloader->td;
}
const char* loadDescriptors_getSegmentation(LoadDescriptors *desloader) {
	return desloader->segmentation;
}
const char* loadDescriptors_getDescriptor(LoadDescriptors *desloader) {
	return desloader->descriptor;
}
const char* loadDescriptors_getDescriptorAlias(LoadDescriptors *desloader) {
	return desloader->descAlias;
}
const char* loadDescriptors_getDataDir(LoadDescriptors *desloader) {
	return desloader->descriptors_dir;
}
bool loadDescriptors_getIsSingleFile(LoadDescriptors *desloader) {
	return desloader->isSingleFile;
}
LoadSegmentation *loadDescriptors_getLoadSegmentation(
		LoadDescriptors *desloader) {
	if (desloader->seg_loader == NULL)
		desloader->seg_loader = newLoadSegmentation(desloader->db,
				desloader->segmentation);
	return desloader->seg_loader;
}

DB *loadDescriptors_getDb(LoadDescriptors *desloader) {
	return desloader->db;
}

static int qsort_compare_GlobalEntry(const void *a, const void *b) {
	struct GlobalEntry *da = *(struct GlobalEntry**) a;
	struct GlobalEntry *db = *(struct GlobalEntry**) b;
	return strcmp(da->file_id, db->file_id);
}
static int binarySearch_compare_GlobalEntry(const void *search_object,
		const void *array_object) {
	const char *id_file = search_object;
	const struct GlobalEntry *db = array_object;
	return strcmp(id_file, db->file_id);
}
static MyVectorObj *parsePosFile(LoadDescriptors *desloader) {
	char *filePos = getFilenameSinglePos(desloader->descriptors_dir);
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filePos, 1), "PVCD", "GlobalPos", 1, 3);
	free(filePos);
	int64_t num_files = my_parse_int(my_lreader_readLine(reader));
	struct GlobalEntry **entries = MY_MALLOC(num_files, struct GlobalEntry*);
	for (int64_t i = 0; i < num_files; ++i) {
		MyTokenizer *tk = my_tokenizer_new(my_lreader_readLine(reader), '\t');
		struct GlobalEntry *entry = MY_MALLOC(1, struct GlobalEntry);
		entry->file_id = my_tokenizer_nextToken_newString(tk);
		entry->offset = my_tokenizer_nextInt(tk);
		entry->size = my_tokenizer_nextInt(tk);
		entries[i] = entry;
		my_tokenizer_release(tk);
	}
	my_lreader_close(reader, true);
	MyVectorObj * entryList = my_vectorObj_new_wrapper(num_files,
			(void**) entries,
			true);
	my_vectorObj_qsort(entryList, qsort_compare_GlobalEntry);
	return entryList;
}
static struct GlobalEntry *getGlobalEntry(LoadDescriptors *desloader,
		FileDB *fdb) {
	if (desloader->loaded_db.entryList == NULL)
		desloader->loaded_db.entryList = parsePosFile(desloader);
	int64_t pos = my_vectorObj_indexOf_binsearch(desloader->loaded_db.entryList,
			fdb->id, binarySearch_compare_GlobalEntry);
	if (pos < 0)
		my_log_error("can't find file %s at %s\n", fdb->id,
				desloader->descriptors_dir);
	struct GlobalEntry *entry = my_vectorObj_get(desloader->loaded_db.entryList,
			pos);
	return entry;
}
static struct DescriptorsFile *createDescriptorsFile(LoadDescriptors *desloader,
		FileDB *fdb, char *base_bytes, int64_t size_bytes,
		bool release_base_bytes) {
	int numDescriptors = 0;
	void **descriptors = NULL;
	if (desloader->td.dtype == DTYPE_LOCAL_VECTORS
			|| desloader->td.dtype == DTYPE_SPARSE_ARRAY) {
		int64_t pos = 0;
		while (pos < size_bytes) {
			int64_t numBytesDesc = 0;
			if (desloader->td.dtype == DTYPE_LOCAL_VECTORS)
				numBytesDesc = my_localDescriptors_deserializePredictReadBytes(
						base_bytes + pos);
			else if (desloader->td.dtype == DTYPE_SPARSE_ARRAY)
				numBytesDesc = my_sparseArray_deserializePredictReadBytes(
						base_bytes + pos);
			my_assert_greaterInt("invalid numBytesDesc", numBytesDesc, 0);
			numDescriptors++;
			pos += numBytesDesc;
			my_assert_lessEqualInt("file size", pos, size_bytes);
		}
		my_assert_equalInt("file size", pos, size_bytes);
		descriptors = MY_MALLOC(numDescriptors, void*);
		pos = 0;
		for (int64_t i = 0; i < numDescriptors; ++i) {
			int64_t numBytesDesc = 0;
			if (desloader->td.dtype == DTYPE_LOCAL_VECTORS) {
				descriptors[i] = my_localDescriptors_newEmpty();
				numBytesDesc = my_localDescriptors_deserialize(base_bytes + pos,
						descriptors[i]);
			} else if (desloader->td.dtype == DTYPE_SPARSE_ARRAY) {
				descriptors[i] = my_sparseArray_new();
				numBytesDesc = my_sparseArray_deserialize(base_bytes + pos,
						descriptors[i]);
			}
			pos += numBytesDesc;
			my_assert_lessEqualInt("invalid file size", pos, size_bytes);
		}
		my_assert_equalInt("file size", pos, size_bytes);
		if (release_base_bytes) {
			MY_FREE(base_bytes);
			release_base_bytes = false;
		}
	} else {
		int64_t numBytesDesc = getLengthBytesDescriptor(desloader->td);
		my_assert_equalInt("invalid file size", size_bytes % numBytesDesc, 0);
		numDescriptors = size_bytes / numBytesDesc;
		descriptors = MY_MALLOC(numDescriptors, void*);
		for (int64_t i = 0; i < numDescriptors; ++i)
			descriptors[i] = base_bytes + i * numBytesDesc;
	}
	struct DescriptorsFile* df = MY_MALLOC(1, struct DescriptorsFile);
	df->desloader = desloader;
	df->size_bytes = size_bytes;
	df->numDescriptors = numDescriptors;
	df->descriptors = descriptors;
	df->fdb = fdb;
	struct LoadedFile *fb = desloader->loaded_files + fdb->internal_id;
	fb->df = df;
	if (release_base_bytes)
		fb->file_bytes_toRelease = base_bytes;
	return df;
}

struct DescriptorsFile* loadDescriptorsFileId(LoadDescriptors *desloader,
		const char *file_id) {
	FileDB *fdb = findFileDB_byId(desloader->db, file_id, true);
	return loadDescriptorsFileDB(desloader, fdb);
}
struct DescriptorsFile* loadDescriptorsFileDB(LoadDescriptors *desloader,
		FileDB *fdb) {
	struct LoadedFile *fb = desloader->loaded_files + fdb->internal_id;
	if (fb->df != NULL)
		return fb->df;
	char *filebytes = NULL;
	int64_t filesize = 0;
	bool release_filebytes = false;
	if (desloader->isSingleFile) {
		struct GlobalEntry *entry = getGlobalEntry(desloader, fdb);
		if (desloader->loaded_db.file_bytes != NULL) {
			filebytes = desloader->loaded_db.file_bytes + entry->offset;
			filesize = entry->size;
			release_filebytes = false;
		} else {
			char *fileBin = getFilenameSingleBin(desloader->descriptors_dir);
			FILE *in = my_io_openFileRead1(fileBin, true);
			if (fseeko(in, entry->offset, SEEK_SET) != 0)
				my_log_error("can't seek %s to %"PRIi64"\n", fileBin,
						entry->offset);
			filesize = entry->size;
			filebytes = MY_MALLOC_NOINIT(filesize, char);
			my_io_readBytesFile(in, filebytes, filesize, false);
			fclose(in);
			release_filebytes = true;
		}
	} else {
		char *fileBin = getFilenameBin(desloader->descriptors_dir, fdb->id);
		filebytes = my_io_loadFileBytes(fileBin, &filesize);
		free(fileBin);
		my_assert_greaterInt("filesize", filesize, 0);
		release_filebytes = true;
	}
	struct DescriptorsFile* df = createDescriptorsFile(desloader, fdb,
			filebytes, filesize, release_filebytes);
	return df;
}
MknnDataset *loadDescriptorsFile_getMknnDataset(struct DescriptorsFile* df,
		double sampleFractionDescriptors, double sampleFractionPerFrame) {
	return descriptors2MknnDataset_samples(df->descriptors, df->numDescriptors,
			df->desloader->td, sampleFractionDescriptors,
			sampleFractionPerFrame);
}
void releaseDescriptorsFile(struct DescriptorsFile *df) {
	if (df == NULL)
		return;
	struct LoadedFile *fb = df->desloader->loaded_files + df->fdb->internal_id;
	if (fb->df != df)
		my_log_error("not matched\n");
	if (df->desloader->td.dtype == DTYPE_LOCAL_VECTORS) {
		for (int64_t i = 0; i < df->numDescriptors; ++i) {
			MyLocalDescriptors *ldes = df->descriptors[i];
			my_localDescriptors_release(ldes);
		}
	} else if (df->desloader->td.dtype == DTYPE_LOCAL_VECTORS) {
		for (int64_t i = 0; i < df->numDescriptors; ++i) {
			MySparseArray *sp = df->descriptors[i];
			my_sparseArray_release(sp);
		}
	}
	MY_FREE_MULTI(df->descriptors, df, fb->file_bytes_toRelease);
	fb->df = NULL;
	fb->file_bytes_toRelease = NULL;
}

struct DescriptorsDB* loadDescriptorsDB(LoadDescriptors *desloader) {
	if (desloader->loaded_db.ddb != NULL)
		return desloader->loaded_db.ddb;
	int64_t numFiles = desloader->db->numFilesDb;
	struct DescriptorsFile **allFiles = MY_MALLOC(numFiles,
			struct DescriptorsFile*);
	int64_t size_bytes_total = 0;
	MyProgress *lt = my_progress_new("loading descriptors", numFiles, 1);
	if (desloader->isSingleFile) {
		char *fileBin = getFilenameSingleBin(desloader->descriptors_dir);
		char *base_bytes = my_io_loadFileBytes(fileBin, &size_bytes_total);
		free(fileBin);
		desloader->loaded_db.file_bytes = base_bytes;
		for (int64_t j = 0; j < numFiles; ++j) {
			allFiles[j] = loadDescriptorsFileDB(desloader,
					desloader->db->filesDb[j]);
			my_progress_add1(lt);
		}
		if (desloader->td.dtype == DTYPE_LOCAL_VECTORS
				|| desloader->td.dtype == DTYPE_SPARSE_ARRAY) {
			MY_FREE(desloader->loaded_db.file_bytes);
			desloader->loaded_db.file_bytes = NULL;
		}
	} else {
		for (int64_t j = 0; j < numFiles; ++j) {
			allFiles[j] = loadDescriptorsFileDB(desloader,
					desloader->db->filesDb[j]);
			size_bytes_total += allFiles[j]->size_bytes;
			my_progress_add1(lt);
		}
	}
	my_progress_release(lt);
	struct DescriptorsDB* ddb = MY_MALLOC(1, struct DescriptorsDB);
	ddb->desloader = desloader;
	ddb->numFiles = numFiles;
	ddb->allFiles = allFiles;
	ddb->size_bytes_total = size_bytes_total;
	desloader->loaded_db.ddb = ddb;
	return ddb;
}
void releaseDescriptorsDB(struct DescriptorsDB *ddb) {
	if (ddb == NULL)
		return;
	if (ddb != ddb->desloader->loaded_db.ddb)
		my_log_error("unknown ddb\n");
	for (int64_t i = 0; i < ddb->numFiles; ++i)
		releaseDescriptorsFile(ddb->allFiles[i]);
	free(ddb->allFiles);
	if (ddb->desloader->loaded_db.file_bytes != NULL) {
		free(ddb->desloader->loaded_db.file_bytes);
		ddb->desloader->loaded_db.file_bytes = NULL;
	}
	if (ddb->desloader->loaded_db.entryList != NULL) {
		for (int64_t i = 0;
				i < my_vectorObj_size(ddb->desloader->loaded_db.entryList);
				++i) {
			struct GlobalEntry *entry = my_vectorObj_get(
					ddb->desloader->loaded_db.entryList, i);
			free(entry->file_id);
			free(entry);
		}
		my_vectorObj_release(ddb->desloader->loaded_db.entryList, false);
		ddb->desloader->loaded_db.entryList = NULL;
	}
	ddb->desloader->loaded_db.ddb = NULL;
	free(ddb);
}
