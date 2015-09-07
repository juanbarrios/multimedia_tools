/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

#ifndef NO_OPENCV
//KIM, VASUDEV, 2005
//ORDINAL MEASUREMENT
struct ZoneValue {
	double averageGray;
	uchar position;
};
struct State_OMD {
	int64_t numZonesW, numZonesH, totalNumZones;
	int64_t lastW, lastH, *limitsW, *limitsH;
	struct ZoneValue *zones;
	uchar *positions;
};

static void ext_config_omd(const char *code, const char *parameters,
		void **out_state, DescriptorType *out_td, bool *out_useImgGray) {
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	my_tokenizer_addDelimiter(tk, 'x');
	struct State_OMD *es = MY_MALLOC(1, struct State_OMD);
	es->numZonesW = my_tokenizer_nextInt(tk);
	es->numZonesH = my_tokenizer_nextInt(tk);
	my_tokenizer_releaseValidateEnd(tk);
	es->totalNumZones = es->numZonesW * es->numZonesH;
	my_assert_lessEqualInt("zones", es->totalNumZones, 256);
	es->positions = MY_MALLOC_NOINIT(es->totalNumZones, uchar);
	es->zones = MY_MALLOC_NOINIT(es->totalNumZones, struct ZoneValue);
	*out_state = es;
	*out_td = descriptorType(DTYPE_ARRAY_UCHAR, es->totalNumZones,
			es->numZonesW * es->numZonesH, 1);
	*out_useImgGray = true;
}

int qsort_compareZones(const void *a, const void *b) {
	struct ZoneValue *na = (struct ZoneValue*) a;
	struct ZoneValue *nb = (struct ZoneValue*) b;
	double da = na->averageGray;
	double db = nb->averageGray;
	if (da == db) {
		//stable sorting algorithm
		return na->position - nb->position;
	} else {
		return (da > db ? 1 : -1);
	}
}

static void *ext_extract_omd(IplImage *image, void *state) {
	struct State_OMD *es = state;
	if (es->lastW != image->width || es->lastH != image->height) {
		MY_FREE_MULTI(es->limitsW, es->limitsH);
		es->limitsW = my_math_partitionIntUB(es->numZonesW, image->width);
		es->limitsH = my_math_partitionIntUB(es->numZonesH, image->height);
		es->lastW = image->width;
		es->lastH = image->height;
	}
	int64_t i, j, pos = 0;
	for (j = 0; j < es->numZonesH; ++j) {
		int64_t startH = (j == 0) ? 0 : es->limitsH[j - 1];
		int64_t endH = es->limitsH[j];
		for (i = 0; i < es->numZonesW; ++i) {
			int64_t startW = (i == 0) ? 0 : es->limitsW[i - 1];
			int64_t endW = es->limitsW[i];
			double val = averageIntensity(image, startW, startH, endW, endH);
			//the value is rounded in order to reduce the impact of near-invisible artifacts
			val = my_math_round_uint8(val / 4);
			es->zones[pos].averageGray = val;
			es->zones[pos].position = pos;
			pos++;
		}
	}
	qsort(es->zones, es->totalNumZones, sizeof(struct ZoneValue),
			qsort_compareZones);
	for (i = 0; i < es->totalNumZones; ++i)
		es->positions[es->zones[i].position] = i;
	return es->positions;
}
static void ext_release_omd(void *state) {
	struct State_OMD *es = state;
	MY_FREE_MULTI(es->limitsW, es->limitsH, es->zones, es->positions, es);
}
void ext_reg_omd() {
	addExtractorGlobalDef(false, "OMD", "[zonesW]x[zonesH]", ext_config_omd,
			ext_extract_omd, ext_release_omd);
}
#endif
