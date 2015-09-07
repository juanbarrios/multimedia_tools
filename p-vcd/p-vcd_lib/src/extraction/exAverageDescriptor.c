/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
struct State_Average {
	Extractor *ex;
	DescriptorType tdEx;
	double *buff_averages;
	struct Quantization quant;
	void *descriptor;
};

static void ext_config_prom(const char *extCode, const char *extParameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(extParameters, '_');
	struct State_Average *es = MY_MALLOC(1, struct State_Average);
	es->quant = getQuantization(my_tokenizer_nextToken(tk));
	es->ex = getExtractor(my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	es->tdEx = getDescriptorType(es->ex);
	int64_t length = es->tdEx.array_length;
	es->buff_averages = MY_MALLOC(length, double);
	es->descriptor = newDescriptorQuantize(es->quant, length);
	*out_state = es;
	*out_td = descriptorType(es->quant.dtype, es->tdEx.array_length,
			es->tdEx.num_subarrays, es->tdEx.subarray_length);
	*out_useImgGray = false;
}
static void average_uchar(uchar *array, int64_t length, double *averages,
		int64_t cont) {
	for (int64_t i = 0; i < length; ++i)
		averages[i] += (array[i] - averages[i]) / cont;
}
static void average_float(float *array, int64_t length, double *averages,
		int64_t cont) {
	for (int64_t i = 0; i < length; ++i)
		averages[i] += (array[i] - averages[i]) / cont;
}
static void average_double(double *array, int64_t length, double *averages,
		int64_t cont) {
	for (int64_t i = 0; i < length; ++i)
		averages[i] += (array[i] - averages[i]) / cont;
}
static void *ext_segmento_frame_prom(struct Extractor_InitParams *ip,
		int64_t idSegment, void *state) {
	struct State_Average *es = state;
	int64_t dtype = es->tdEx.dtype;
	int64_t length = es->tdEx.array_length;
	struct VideoSegment s = ip->segmentation->segments[idSegment];
	MY_SETZERO(es->buff_averages, length, double);
	double *averages = es->buff_averages;
	int64_t cont = 0;
	for (int64_t k = s.start_frame; k <= s.end_frame; ++k) {
		if (!seekVideoToFrame(ip->video_frame, k)) {
			my_log_info("AVG: can't seek to frame %"PRIi64" in file %s\n", k,
					ip->fileDB->id);
			break;
		}
		IplImage *frame = getCurrentFrameOrig(ip->video_frame);
		void *descFrame = extractVolatileDescriptor(es->ex, frame);
		cont++;
		if (dtype == DTYPE_ARRAY_UCHAR) {
			average_uchar((uchar*) descFrame, length, averages, cont);
		} else if (dtype == DTYPE_ARRAY_FLOAT) {
			average_float((float*) descFrame, length, averages, cont);
		} else if (dtype == DTYPE_ARRAY_DOUBLE) {
			average_double((double*) descFrame, length, averages, cont);
		} else {
			my_log_error("unknown dtype %"PRIi64"\n", dtype);
		}
	}
	quantize(es->quant, averages, es->descriptor, length);
	return es->descriptor;
}
static void ext_release_prom(void *estado) {
	struct State_Average *es = estado;
	releaseExtractor(es->ex);
	MY_FREE(es->descriptor);
	MY_FREE(es->buff_averages);
	MY_FREE(es);
}
void ext_reg_average() {
	addExtractorSegmentDef("AVG", "(quant:R1U|1U|4F...)_[descriptor]",
			ext_config_prom, NULL, ext_segmento_frame_prom,
			NULL, ext_release_prom);
}
#endif
