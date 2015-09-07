/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef WAV_H
#define WAV_H

#include "../pvcd.h"

void loadAudioData_s16le_mono(FileDB *fdb, int64_t sampleRate,
		int64_t *out_numSamples, short **out_samples);
void computeAudioLength(const char *filename, double *out_seconds,
		double *out_samplerate);

#endif
