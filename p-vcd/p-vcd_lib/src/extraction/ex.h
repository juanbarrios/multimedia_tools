/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef EX_H
#define EX_H

#include "../pvcd.h"

DescriptorType descriptorType(int64_t dtype, int64_t array_length,
		int64_t num_subarrays, int64_t subarray_length);
const char *dtype2string(int64_t dtype);
int64_t string2dtype(const char *dtypeName);
void assertEqualDescriptorType(DescriptorType td1, DescriptorType td2);
int64_t getLengthBytesDescriptor(DescriptorType td);
void* cloneDescriptor(DescriptorType td, void *desSrc);
char *printDescriptor(DescriptorType td, void *descriptor);
void descriptor2doubleArray(DescriptorType td, void *descriptor,
		double *out_array);

//util_metricknn
MknnDataset *descriptors2MknnDataset(void **descriptors,
		int64_t num_descriptors, DescriptorType td);
MknnDataset *descriptors2MknnDataset_positions(void **descriptors,
		MyVectorInt *positions, DescriptorType td,
		double sampleFraction_localsPerFrame);
MknnDataset *descriptors2MknnDataset_samples(void **descriptors,
		int64_t num_descriptors, DescriptorType td,
		double sampleFractionSegments, double sampleFraction_localsPerFrame);

MknnDataset* getAlreadyComputedDescriptorsSample_singleFile(
		LoadDescriptors *loader, FileDB *fdb, double sampleFractionSegments,
		double sampleFractionPerFrame);
MknnDataset *getAlreadyComputedDescriptorsSample(LoadDescriptors *desloader,
		double sampleFractionGlobal, double sampleFractionFiles,
		double sampleFractionSegments, double sampleFractionPerFrame);

void pvcd_register_default_extractors();
void print_extractors_local();
void print_extractors();

#define QUANT_LINEAL 20
#define QUANT_ROUND 30

struct Quantization {
	int64_t dtype, quantizeType;
};
struct Quantization getQuantization(const char *name);
void *newDescriptorQuantize(struct Quantization quant, int64_t length);
void quantize(struct Quantization quant, double *src_array, void *descriptor,
		int64_t length);


/**********/

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

typedef void (*extract_func_new)(const char *code, const char *parameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray);
typedef void *(*extract_func_extract_image)(IplImage *img, void *state);
typedef void (*extract_func_release)(void *state);

void addExtractorGlobalDef(bool isLocal, const char *code, const char *help,
		extract_func_new func_new,
		extract_func_extract_image func_extract_image,
		extract_func_release func_release);

/**********/
struct Extractor_InitParams {
	VideoFrame *video_frame;
	const struct Segmentation *segmentation;
	FileDB *fileDB;
};

typedef void (*extract_func_init_video)(struct Extractor_InitParams *ip,
		void *state);
typedef void *(*extract_func_extract_segment)(struct Extractor_InitParams *ip,
		int64_t idSegment, void *state);
typedef void (*extract_func_end_video)(struct Extractor_InitParams *ip,
		void* state);

void addExtractorSegmentDef(const char *code, const char *help,
		extract_func_new func_new, extract_func_init_video func_init_video,
		extract_func_extract_segment func_extract_segment,
		extract_func_end_video func_end_video,
		extract_func_release func_release);

/****************/
typedef struct Extractor Extractor;

const char *getExtractorHelp(const char *extCode);
Extractor *getExtractor2(const char *extCode, const char *extParameters);
Extractor *getExtractor(const char *extCodeAndParameters);
DescriptorType getDescriptorType(Extractor *ex);
void* getExtractorState(Extractor *ex);
const char *getExtractorCodeAndParameters(Extractor *ex);
void *extractVolatileDescriptor(Extractor *ex, IplImage *image);
void releaseExtractor(Extractor *ex);

void extractPersistentDescriptors_seg(Extractor *ex, FileDB *fdb,
		const struct Segmentation *seg, int64_t first_segment,
		int64_t last_segmentNotIncluded, void **persistent_descriptors);
void extractPersistentDescriptors_threadedSegments(Extractor **exs,
		int64_t numExtractors, FileDB *fdb, const struct Segmentation *seg,
		int64_t first_segment, int64_t last_segmentNotIncluded,
		void **persistent_descriptors);
void releasePersistentDescriptors(void **descriptors, int64_t numDescriptors,
		DescriptorType td);

//util
MknnDataset* computeDescriptorsSample_singleFile(LoadSegmentation *loader,
		Extractor *ex, FileDB *fdb, double sampleFractionSegments,
		double sampleFractionPerFrame);
MknnDataset *computeDescriptorsSample(LoadSegmentation *segloader,
		Extractor **exs, int64_t numThreads, double sampleFractionGlobal,
		double sampleFractionFiles, double sampleFractionSegments,
		double sampleFractionPerFrame);
MknnDataset *computeDescriptorsSample_nameExtractor(LoadSegmentation *segloader,
		const char *nameExtractor, int64_t numThreads,
		double sampleFractionGlobal, double sampleFractionFiles,
		double sampleFractionSegments, double sampleFractionPerFrame);
//utilEx.c
double averageIntensity(IplImage *imgGray, int64_t startW, int64_t startH,
		int64_t endWNotIncluded, int64_t endHNotIncluded);
double *newKernel(double d00, double d01, double d10, double d11);

struct Zone {
	int64_t numZone;
	double prop_start_w, prop_end_w, prop_start_h, prop_end_h;
	int64_t pix_start_w, pix_end_w, pix_start_h, pix_end_h;
	CvRect rect;
};
struct Zoning {
	int64_t numZones;
	struct Zone **zones;
	int64_t currentW, currentH;
};
struct Zoning *parseZoning(const char *paramsZoning);
void processZoning(IplImage *image, struct Zoning *zoning,
		void (*func_processZone)(IplImage *image, struct Zone *z, void *state),
		void *state);
void releaseZoning(struct Zoning *zoning);

void applyPcaToLocalDescriptors(MknnPcaAlgorithm *pca,
		MyLocalDescriptors *src_descriptor, MyLocalDescriptors *dst_descriptor);

#endif
#endif
