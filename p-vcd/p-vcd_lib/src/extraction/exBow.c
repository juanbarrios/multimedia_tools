/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"
#include <myutils/myutils_c.h>

#ifndef NO_OPENCV
struct State_Bow {
	struct Quantization quant;
	struct Zoning *zoning;
	int64_t descriptorLength, numCentroids;
	MknnIndex *index;
	MknnResolver *resolver;
	Extractor *local_ext;
	double *bins;
	void *descriptor;
	char *localDescriptorAlias;
	struct DescriptorsFile *current_file;
};

static void ext_new_bow(const char *extCode, const char *extParameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(extParameters, '_');
	my_tokenizer_useBrackets(tk, '(', ')');
	struct State_Bow *es = MY_MALLOC(1, struct State_Bow);
	es->zoning = parseZoning(my_tokenizer_nextToken(tk));
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	char *filenameCentroids = my_tokenizer_nextToken_newString(tk);
	char *distanceName = my_tokenizer_nextToken_newString(tk);
	if (my_string_equals_ignorecase(extCode, "BOW"))
		es->local_ext = getExtractor(my_tokenizer_getCurrentTail(tk));
	if (my_string_equals_ignorecase(extCode, "BOWPRECOMPUTED"))
		es->localDescriptorAlias = my_newString_string(
				my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	MknnDataset *centroids = mknn_datasetLoader_ParseVectorFile(
			filenameCentroids, MKNN_DATATYPE_FLOATING_POINT_32bits);
	free(filenameCentroids);
	MknnDistance *distance = mknn_distance_newPredefined(
			mknn_distanceParams_newParseString(distanceName), true);
	free(distanceName);
	es->index = mknn_index_newPredefined(
			mknn_predefIndex_LinearScan_indexParams(), true, centroids, true,
			distance, true);
	es->resolver = mknn_index_newResolver(es->index,
			mknn_predefIndex_LinearScan_resolverExactNearestNeighbors(1, 0, 0),
			true);
	es->numCentroids = mknn_dataset_getNumObjects(centroids);
	es->descriptorLength = es->zoning->numZones * es->numCentroids;
	es->bins = MY_MALLOC_NOINIT(es->descriptorLength, double);
	es->descriptor = newDescriptorQuantize(es->quant, es->descriptorLength);
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->descriptorLength,
			es->zoning->numZones, es->numCentroids);
	*out_useImgGray = false;
}
struct ParamsZones {
	MyLocalDescriptors *ldes;
	MknnResult *result;
	struct State_Bow *es;
};
static void ext_func_processZone(IplImage *image, struct Zone *zone,
		void *state) {
	struct ParamsZones *p = state;
	double *current_bins = p->es->bins + zone->numZone * p->es->numCentroids;
	for (int64_t i = 0; i < my_localDescriptors_getNumDescriptors(p->ldes);
			++i) {
		struct MyLocalKeypoint kp = my_localDescriptors_getKeypoint(p->ldes, i);
		if (!(zone->pix_start_w <= kp.x && kp.x < zone->pix_end_w
				&& zone->pix_start_h <= kp.y && kp.y < zone->pix_end_h))
			continue;
		MknnResultQuery *res = mknn_result_getResultQuery(p->result, i);
		if (res->num_nns == 0)
			continue;
		int64_t id_centroid = res->nn_position[0];
		my_assert_indexRangeInt("id_centroid", id_centroid,
				p->es->numCentroids);
		current_bins[id_centroid]++;
	}
}
static void compute_bow(IplImage *frame, MyLocalDescriptors *ldes,
		struct State_Bow *es) {
	MY_SETZERO(es->bins, es->descriptorLength, double);
	if (my_localDescriptors_getNumDescriptors(ldes) == 0)
		return;
	//compute nearest centroids
	MknnDataset *query_dataset = my_localDescriptors_createMknnDataset_vectors(
			ldes, false);
	MknnResult *result = mknn_resolver_search(es->resolver, false,
			query_dataset, false);
	//aggregate centroids
	struct ParamsZones p = { 0 };
	p.result = result;
	p.ldes = ldes;
	p.es = es;
	processZoning(frame, es->zoning, ext_func_processZone, &p);
	my_math_normalizeNorm1_double(es->bins, es->descriptorLength);
	quantize(es->quant, es->bins, es->descriptor, es->descriptorLength);
	mknn_result_release(result);
	mknn_dataset_release(query_dataset);
}
static void *ext_extract_bow(IplImage *image, void *state) {
	struct State_Bow *es = state;
	MyLocalDescriptors *ldes = extractVolatileDescriptor(es->local_ext, image);
	compute_bow(image, ldes, es);
	return es->descriptor;
}
static void ext_release_bow(void *state) {
	struct State_Bow *es = state;
	mknn_resolver_release(es->resolver);
	mknn_index_release(es->index);
	releaseZoning(es->zoning);
	releaseExtractor(es->local_ext);
	MY_FREE_MULTI(es->localDescriptorAlias, es->bins, es->descriptor, es);
}
static void ext_func_init_video(struct Extractor_InitParams *ip, void *state) {
	struct State_Bow *es = state;
	LoadDescriptors *dd = newLoadDescriptors(ip->fileDB->db,
			es->localDescriptorAlias);
	if (loadDescriptors_getDescriptorType(dd).dtype != DTYPE_LOCAL_VECTORS)
		my_log_error("BOW requires local descriptors\n");
	es->current_file = loadDescriptorsFileDB(dd, ip->fileDB);
}
static void *ext_segment_bow(struct Extractor_InitParams *ip, int64_t idSegment,
		void *state) {
	struct State_Bow *es = state;
	seekVideoToFrame(ip->video_frame,
			ip->segmentation->segments[idSegment].selected_frame);
	IplImage *frame = getCurrentFrameOrig(ip->video_frame);
	MyLocalDescriptors *ldes = es->current_file->descriptors[idSegment];
	compute_bow(frame, ldes, es);
	return es->descriptor;
}
static void ext_func_end_video(struct Extractor_InitParams *ip, void* state) {
	struct State_Bow *es = state;
	releaseDescriptorsFile(es->current_file);
}

void ext_reg_bow() {
	addExtractorGlobalDef(false, "BOW",
			"(zones:1x1+2x2+.)_(quant:1U|4F|.)_[fileCentroidsTxt]_[distanceDesc]_[localDescriptor]",
			ext_new_bow, ext_extract_bow, ext_release_bow);
	addExtractorSegmentDef("BOWPRECOMPUTED",
			"(zones:1x1+2x2+.)_(quant:1U|4F|.)_[fileCentroidsTxt]_[distanceDesc]_[precomputedDescriptorAlias]",
			ext_new_bow, ext_func_init_video, ext_segment_bow,
			ext_func_end_video, ext_release_bow);
}
#endif
