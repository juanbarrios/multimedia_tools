/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

void register_index_linearscan();
void register_index_laesa();
void register_index_snaketable();
void register_index_flann();

void mknn_register_default_indexes() {
	register_index_linearscan();
	register_index_laesa();
	register_index_snaketable();
	register_index_flann();
}
