/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

void register_distance_lp();
void register_distance_hamming();
void register_distance_bits();
void register_distance_chi2();
void register_distance_hellinger();
void register_distance_dpf();
void register_distance_emd();
void register_distance_cosine();
void register_distance_multiDistance();

void mknn_register_default_distances() {
	register_distance_lp();
	register_distance_hamming();
	register_distance_bits();
	register_distance_chi2();
	register_distance_hellinger();
	register_distance_dpf();
	register_distance_emd();
	register_distance_cosine();
	register_distance_multiDistance();
}

