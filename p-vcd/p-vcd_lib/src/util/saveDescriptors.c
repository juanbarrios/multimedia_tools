/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "saveDescriptors.h"

char *getFilenameBin(const char *descriptors_dir, const char *file_id) {
	return my_newString_format("%s/%s.bin", descriptors_dir, file_id);
}
char *getFilenameTxt(const char *descriptors_dir, const char *file_id) {
	return my_newString_format("%s/%s.txt", descriptors_dir, file_id);
}
char *getFilenameSingleBin(const char *descriptors_dir) {
	return my_newString_format("%s/descriptor.bin", descriptors_dir);
}
char *getFilenameSinglePos(const char *descriptors_dir) {
	return my_newString_format("%s/descriptor.pos", descriptors_dir);
}
char *getFilenameSingleTxt(const char *descriptors_dir) {
	return my_newString_format("%s/descriptor.txt", descriptors_dir);
}
char *getFilenameDes(const char *descriptors_dir) {
	return my_newString_format("%s/descriptor.des", descriptors_dir);
}

struct SaveDescriptors {
	bool saveBinaryFormat, saveTxtFormat, useSingleFile;
	char *output_dir, *segmentation, *descriptor;
	DescriptorType td;
	int64_t total_bytes, total_descriptors;
	pthread_mutex_t mutex;
	FILE *singleFile_out, *singleFile_outTxt;
	MyVectorString *singleFile_ids;
	MyVectorInt *singleFile_starts, *singleFile_sizes;
};
static void print_DescriptorFileTxt(FILE *outTxt, DescriptorType td,
		int64_t numDescriptors, void **descriptors,
		const struct Segmentation *seg) {
	for (int64_t i = 0; i < numDescriptors; ++i) {
		if (numDescriptors > 1) {
			if (seg != NULL) {
				struct VideoSegment *s = seg->segments + i;
				char *st = print_VideoSegment(s);
				fprintf(outTxt, "#%s\n", st);
				free(st);
			} else {
				fprintf(outTxt, "##%"PRIi64"\n", i);
			}
		}
		char *st = printDescriptor(td, descriptors[i]);
		fprintf(outTxt, "%s\n", st);
		free(st);
	}
}
static int64_t print_DescriptorFileBin(FILE *out, DescriptorType td,
		int64_t numDescriptors, void **descriptors) {
	int64_t write_size = 0;
	if (td.dtype == DTYPE_LOCAL_VECTORS || td.dtype == DTYPE_SPARSE_ARRAY) {
		char *buffer = NULL;
		for (int64_t i = 0; i < numDescriptors; ++i) {
			my_assert_notNull("descriptor", descriptors[i]);
			int64_t numBytes = 0;
			if (td.dtype == DTYPE_LOCAL_VECTORS) {
				MyLocalDescriptors *ldes = (MyLocalDescriptors *) descriptors[i];
				numBytes = my_localDescriptors_serializePredictSizeBytes(ldes);
				MY_REALLOC(buffer, numBytes, char);
				int64_t numBytes2 = my_localDescriptors_serialize(ldes, buffer);
				my_assert_equalInt("numBytes", numBytes2, numBytes);
			} else if (td.dtype == DTYPE_SPARSE_ARRAY) {
				MySparseArray *sparse = (MySparseArray*) descriptors[i];
				numBytes = my_sparseArray_serializePredictSizeBytes(sparse);
				MY_REALLOC(buffer, numBytes, char);
				int64_t numBytes2 = my_sparseArray_serialize(sparse, buffer);
				my_assert_equalInt("numBytes", numBytes2, numBytes);
			}
			int64_t wrote = fwrite(buffer, sizeof(char), numBytes, out);
			my_assert_equalInt("fwrite", wrote, numBytes);
			write_size += numBytes;
		}
		MY_FREE(buffer);
	} else {
		for (int64_t i = 0; i < numDescriptors; ++i) {
			my_assert_notNull("descriptor", descriptors[i]);
			int64_t numBytes = getLengthBytesDescriptor(td);
			int64_t wrote = fwrite(descriptors[i], sizeof(char), numBytes, out);
			my_assert_equalInt("fwrite", wrote, numBytes);
			write_size += numBytes;
		}
	}
	return write_size;
}
static void writeDescriptorsToNewFile(SaveDescriptors *dessaver,
		const char *file_id, int64_t numDescriptors, void **descriptors,
		const struct Segmentation *seg) {
	my_io_createDir(dessaver->output_dir, false);
	char *fname = getFilenameBin(dessaver->output_dir, file_id);
	char *fnameTxt = getFilenameTxt(dessaver->output_dir, file_id);
	my_io_deleteFile(fname, false);
	my_io_deleteFile(fnameTxt, false);
	int64_t write_size = 0;
	if (dessaver->saveBinaryFormat) {
		char *fnameTemp = my_newString_format("%s.temp", fname);
		FILE *out = my_io_openFileWrite1(fnameTemp);
		write_size = print_DescriptorFileBin(out, dessaver->td, numDescriptors,
				descriptors);
		fclose(out);
		my_io_moveFile(fnameTemp, fname, true);
		free(fnameTemp);
	}
	if (dessaver->saveTxtFormat) {
		char *fnameTxtTemp = my_newString_format("%s.temp", fnameTxt);
		FILE *outTxt = my_io_openFileWrite1(fnameTxtTemp);
		print_DescriptorFileTxt(outTxt, dessaver->td, numDescriptors,
				descriptors, seg);
		fclose(outTxt);
		my_io_moveFile(fnameTxtTemp, fnameTxt, true);
		free(fnameTxtTemp);
	}
	free(fname);
	free(fnameTxt);
	MY_MUTEX_LOCK(dessaver->mutex);
	dessaver->total_bytes += write_size;
	dessaver->total_descriptors += numDescriptors;
	MY_MUTEX_UNLOCK(dessaver->mutex);
}
static void openSingleFile(SaveDescriptors *dessaver) {
	my_io_createDir(dessaver->output_dir, false);
	if (dessaver->saveBinaryFormat) {
		char *fname = getFilenameSingleBin(dessaver->output_dir);
		dessaver->singleFile_out = my_io_openFileWrite1(fname);
		free(fname);
		dessaver->singleFile_ids = my_vectorString_new();
		dessaver->singleFile_starts = my_vectorInt_new();
		dessaver->singleFile_sizes = my_vectorInt_new();
	}
	if (dessaver->saveTxtFormat) {
		char *fnameTxt = getFilenameSingleTxt(dessaver->output_dir);
		dessaver->singleFile_outTxt = my_io_openFileWrite1(fnameTxt);
		free(fnameTxt);
	}
}
static void writeDescriptorsToSingleFile(SaveDescriptors *dessaver,
		const char *file_id, int64_t numDescriptors, void **descriptors,
		const struct Segmentation *seg) {
	MY_MUTEX_LOCK(dessaver->mutex);
	if (dessaver->singleFile_out == NULL)
		openSingleFile(dessaver);
	int64_t write_size = 0;
	if (dessaver->saveBinaryFormat) {
		write_size = print_DescriptorFileBin(dessaver->singleFile_out,
				dessaver->td, numDescriptors, descriptors);
		my_vectorString_add(dessaver->singleFile_ids,
				my_newString_string(file_id));
		my_vectorInt_add(dessaver->singleFile_starts, dessaver->total_bytes);
		my_vectorInt_add(dessaver->singleFile_sizes, write_size);
	}
	if (dessaver->saveTxtFormat) {
		fprintf(dessaver->singleFile_outTxt, "##%s\n", file_id);
		print_DescriptorFileTxt(dessaver->singleFile_outTxt, dessaver->td,
				numDescriptors, descriptors, seg);
	}
	dessaver->total_bytes += write_size;
	dessaver->total_descriptors += numDescriptors;
	MY_MUTEX_UNLOCK(dessaver->mutex);
}
static void closeSingleFile(SaveDescriptors *dessaver) {
	if (dessaver->singleFile_out != NULL)
		fclose(dessaver->singleFile_out);
	if (dessaver->singleFile_outTxt != NULL)
		fclose(dessaver->singleFile_outTxt);
	char *fname = getFilenameSinglePos(dessaver->output_dir);
	FILE *compt_pos = my_io_openFileWrite1Config(fname, "PVCD", "GlobalPos", 1,
			3);
	free(fname);
	fprintf(compt_pos, "%"PRIi64"\n",
			my_vectorString_size(dessaver->singleFile_ids));
	for (int64_t i = 0; i < my_vectorString_size(dessaver->singleFile_ids);
			++i) {
		char *idVideo = my_vectorString_get(dessaver->singleFile_ids, i);
		int64_t start = my_vectorInt_get(dessaver->singleFile_starts, i);
		int64_t size = my_vectorInt_get(dessaver->singleFile_sizes, i);
		fprintf(compt_pos, "%s\t%"PRIi64"\t%"PRIi64"\n", idVideo, start, size);
	}
	fclose(compt_pos);
	my_vectorString_release(dessaver->singleFile_ids, true);
	my_vectorInt_release(dessaver->singleFile_starts);
	my_vectorInt_release(dessaver->singleFile_sizes);
}

static void writeDescriptorDes(SaveDescriptors *dessaver) {
	char *fname = getFilenameDes(dessaver->output_dir);
	FILE *out = my_io_openFileWrite1Config(fname, "PVCD", "DescriptorData", 1,
			2);
	free(fname);
	fprintf(out, "descriptor=%s\n", dessaver->descriptor);
	if (dessaver->segmentation != NULL)
		fprintf(out, "segmentation=%s\n", dessaver->segmentation);
	fprintf(out, "descriptor_type=%s\n", dtype2string(dessaver->td.dtype));
	fprintf(out, "array_length=%"PRIi64"\n", dessaver->td.array_length);
	fprintf(out, "num_subarrays=%"PRIi64"\n", dessaver->td.num_subarrays);
	fprintf(out, "subarray_length=%"PRIi64"\n", dessaver->td.subarray_length);
	char *s = my_newString_bool(dessaver->useSingleFile);
	fprintf(out, "single_file=%s\n", s);
	free(s);
	fclose(out);
}

SaveDescriptors *newSaveDescriptors(const char *output_dir, DescriptorType td,
bool saveBinaryFormat, bool saveTxtFormat, bool useSingleFile,
		const char *descriptor, const char *segmentation) {
	SaveDescriptors *dessaver = MY_MALLOC(1, SaveDescriptors);
	dessaver->output_dir = my_newString_string(output_dir);
	dessaver->td = td;
	dessaver->saveBinaryFormat = saveBinaryFormat;
	dessaver->saveTxtFormat = saveTxtFormat;
	dessaver->useSingleFile = useSingleFile;
	dessaver->descriptor = my_newString_string(descriptor);
	dessaver->segmentation = my_newString_string(segmentation);
	MY_MUTEX_INIT(dessaver->mutex);
	return dessaver;
}
bool saveDescriptors_existsFile(SaveDescriptors *dessaver, const char *file_id) {
	char *fname = NULL;
	if (dessaver->useSingleFile) {
		fname = getFilenameDes(dessaver->output_dir);
	} else {
		fname = getFilenameBin(dessaver->output_dir, file_id);
	}
	bool is = (my_io_existsFile(fname) && my_io_getFilesize(fname) > 0);
	free(fname);
	return is;
}
void saveDescriptors(SaveDescriptors *dessaver, const char *file_id,
		int64_t numDescriptors, void **descriptors,
		const struct Segmentation *seg) {
	if (dessaver->useSingleFile) {
		writeDescriptorsToSingleFile(dessaver, file_id, numDescriptors,
				descriptors, seg);
	} else {
		writeDescriptorsToNewFile(dessaver, file_id, numDescriptors,
				descriptors, seg);
	}
}
void releaseSaveDescriptors(SaveDescriptors *dessaver) {
	if (dessaver->singleFile_out != NULL)
		closeSingleFile(dessaver);
	if (dessaver->total_bytes > 0)
		writeDescriptorDes(dessaver);
	char *fs = my_newString_diskSpace(dessaver->total_bytes);
	my_log_info("saved %"PRIi64" descriptors in %s (%s)\n",
			dessaver->total_descriptors, dessaver->output_dir, fs);
	free(fs);
	MY_MUTEX_DESTROY(dessaver->mutex);
	MY_FREE_MULTI(dessaver->output_dir, dessaver->descriptor,
			dessaver->segmentation, dessaver);
}
