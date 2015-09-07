/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

MknnDistanceParams *mknn_predefDistance_L1() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "L1");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_L2() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "L2");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_L2squared() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "L2SQUARED");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_Lmax() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "LMAX");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_Lp(double order) {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "LP");
	mknn_distanceParams_addDouble(parameters, "order", order);
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_Hamming() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "HAMMING");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_Chi2() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "CHI2");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_Hellinger() {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "HELLINGER");
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_CosineSimilarity(bool normalize_vectors) {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "COSINE_SIMILARITY");
	mknn_distanceParams_addBool(parameters, "normalize_vectors",
			normalize_vectors);
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_CosineDistance(bool normalize_vectors) {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "COSINE_DISTANCE");
	mknn_distanceParams_addBool(parameters, "normalize_vectors",
			normalize_vectors);
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_EMD(int64_t matrix_rows,
		int64_t matrix_cols, double *cost_matrix, bool normalize_vectors) {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "EMD");
	mknn_distanceParams_addInt(parameters, "rows", matrix_rows);
	mknn_distanceParams_addInt(parameters, "cols", matrix_cols);
	mknn_distanceParams_addObject(parameters, "matrix_ptrDouble", cost_matrix);
	mknn_distanceParams_addBool(parameters, "normalize_vectors",
			normalize_vectors);
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_DPF(double order,
		int64_t num_dims_discard, double pct_discard, double threshold_discard) {
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "DPF");
	mknn_distanceParams_addDouble(parameters, "order", order);
	mknn_distanceParams_addInt(parameters, "dims_discard", num_dims_discard);
	mknn_distanceParams_addDouble(parameters, "pct_discard", pct_discard);
	mknn_distanceParams_addDouble(parameters, "threshold_discard",
			threshold_discard);
	return parameters;
}
MknnDistanceParams *mknn_predefDistance_MultiDistance(int64_t num_subdistances,
		MknnDistance **subdistances, bool free_subdistances_on_release,
		double *normalization_values, double *ponderation_values,
		bool with_auto_config, MknnDataset *auto_config_dataset,
		double auto_normalize_alpha, bool auto_ponderation_maxrho,
		bool auto_ponderation_maxtau) {
	my_log_error("not implemented\n");
	MknnDistanceParams *parameters = mknn_distanceParams_newEmpty();
	mknn_distanceParams_setDistanceId(parameters, "MULTIDISTANCE");
	mknn_distanceParams_addInt(parameters, "num_subdistances",
			num_subdistances);
	mknn_distanceParams_addObject(parameters, "subdistances", subdistances);
	mknn_distanceParams_addBool(parameters, "free_subdistances_on_release",
			free_subdistances_on_release);
	mknn_distanceParams_addObject(parameters, "normalization_values",
			normalization_values);
	mknn_distanceParams_addObject(parameters, "ponderation_values",
			ponderation_values);
	mknn_distanceParams_addBool(parameters, "with_auto_config",
			with_auto_config);
	mknn_distanceParams_addObject(parameters, "auto_config_dataset",
			auto_config_dataset);
	mknn_distanceParams_addDouble(parameters, "auto_normalize_alpha",
			auto_normalize_alpha);
	mknn_distanceParams_addBool(parameters, "auto_ponderation_maxrho",
			auto_ponderation_maxrho);
	mknn_distanceParams_addBool(parameters, "auto_ponderation_maxtau",
			auto_ponderation_maxtau);
	return parameters;
}
