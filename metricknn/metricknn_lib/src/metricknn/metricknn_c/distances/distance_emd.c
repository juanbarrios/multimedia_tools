/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#ifndef NO_OPENCV

#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>

// OpenCV documentation for EMD:
// http://docs.opencv.org/modules/imgproc/doc/histograms.html#emd

typedef void (*func_fill_signature)(void *object, CvMat *signature1,
		int64_t dims);

struct State_EMD_Dist {
	CvMat *cost_matrix;
	bool normalize_vectors;
};

struct State_EMD_Eval {
	int64_t dims_left, dims_right;
	void *last_object_left, *last_object_right;
	bool isZero_object_left, isZero_object_right;
	CvMat *signature_left, *signature_right;
	func_fill_signature fill_signature_left;
	func_fill_signature fill_signature_right;
	struct State_EMD_Dist *state_dist;
};

#define FILL_FUNCTIONS(typeVector) \
static void fill_signature_##typeVector(void *object, CvMat *signature, int64_t dimensions) { \
	typeVector *array = (typeVector*) object; \
	for (int64_t i = 0; i < dimensions; ++i) { \
		double weight = (double) array[i]; \
		cvSet2D(signature, i, 0, cvScalarAll(weight)); \
	} \
}

GENERATE_SINGLE_DATATYPE(FILL_FUNCTIONS)

static bool sum_normalize_signature(CvMat *signature, int64_t dimensions,
bool normalize_vectors) {
	bool all_zero = true;
	double sum = 0;
	int64_t i;
	for (i = 0; i < dimensions; ++i) {
		CvScalar w = cvGet2D(signature, i, 0);
		sum += w.val[0];
		if (w.val[0] != 0 && all_zero)
			all_zero = false;
	}
	if (normalize_vectors && sum > 0) {
		for (i = 0; i < dimensions; ++i) {
			CvScalar w = cvGet2D(signature, i, 0);
			cvSet2D(signature, i, 0, cvScalarAll(w.val[0] / sum));
		}
	}
	return all_zero;
}

static double emd_distanceEval_eval(void *state_distEval, void *object_left,
		void *object_right, double current_threshold) {
	struct State_EMD_Eval *state = state_distEval;
	if (object_left != state->last_object_left) {
		state->fill_signature_left(object_left, state->signature_left,
				state->dims_left);
		state->isZero_object_left = sum_normalize_signature(
				state->signature_left, state->dims_left,
				state->state_dist->normalize_vectors);
		state->last_object_left = object_left;
	}
	if (object_right != state->last_object_right) {
		state->fill_signature_right(object_right, state->signature_right,
				state->dims_right);
		state->isZero_object_right = sum_normalize_signature(
				state->signature_right, state->dims_right,
				state->state_dist->normalize_vectors);
		state->last_object_right = object_right;
	}
	if (state->isZero_object_left || state->isZero_object_right)
		return DBL_MAX;
	float dist = cvCalcEMD2(state->signature_left, state->signature_right,
			CV_DIST_USER,
			NULL, state->state_dist->cost_matrix, NULL, NULL, NULL);
	if (isnan(dist) || isinf(dist))
		return DBL_MAX;
	return dist;
}
static void emd_distanceEval_release(void *state_distEval) {
	struct State_EMD_Eval *state = state_distEval;
	cvReleaseMat(&state->signature_left);
	cvReleaseMat(&state->signature_right);
	free(state);
}
static struct MknnDistEvalInstance emd_distanceEval_new(void *state_distance,
		MknnDomain *domain_left, MknnDomain *domain_right) {
	struct State_EMD_Eval *state_eval = MY_MALLOC(1, struct State_EMD_Eval);
	state_eval->state_dist = state_distance;
	state_eval->dims_left = mknn_domain_vector_getNumDimensions(domain_left);
	state_eval->dims_right = mknn_domain_vector_getNumDimensions(domain_right);
	if (state_eval->dims_left != state_eval->state_dist->cost_matrix->rows
			|| state_eval->dims_right
					!= state_eval->state_dist->cost_matrix->cols)
		my_log_error(
				"Matrix dimensions (%ix%i) do not coincide with vector dimensions (%ix%i)\n",
				(int) state_eval->state_dist->cost_matrix->rows,
				(int) state_eval->state_dist->cost_matrix->cols,
				(int) state_eval->dims_left, (int) state_eval->dims_right);
	state_eval->signature_left = cvCreateMat(state_eval->dims_left, 1, CV_32F);
	state_eval->signature_right = cvCreateMat(state_eval->dims_right, 1,
	CV_32F);
	MknnDatatype datatype_l = mknn_domain_vector_getDimensionDataType(
			domain_left);
	MknnDatatype datatype_r = mknn_domain_vector_getDimensionDataType(
			domain_right);
	ASSIGN_SINGLE_DATATYPE(state_eval->fill_signature_left, datatype_l,
			fill_signature_)
	ASSIGN_SINGLE_DATATYPE(state_eval->fill_signature_right, datatype_r,
			fill_signature_)
	struct MknnDistEvalInstance di = { 0 };
	di.state_distEval = state_eval;
	di.func_distanceEval_eval = emd_distanceEval_eval;
	di.func_distanceEval_release = emd_distanceEval_release;
	return di;
}
static void fill_matrix(struct State_EMD_Dist *state_dist, double *array_d,
		float *array_f, int64_t rows, int64_t cols) {
	if (rows <= 0 || cols <= 0) {
		my_log_error("undefined rows and cols for matrix cost\n");
	}
	CvMat *cost_matrix = cvCreateMat(rows, cols, CV_32F);
	int64_t pos = 0;
	for (int64_t i = 0; i < rows; ++i) {
		for (int64_t j = 0; j < cols; ++j) {
			double val = (array_d != NULL) ? array_d[pos] : array_f[pos];
			cvSet2D(cost_matrix, i, j, cvScalarAll(val));
			pos++;
		}
	}
	state_dist->cost_matrix = cost_matrix;
}
static void parse_file(const char *filename_read, void *state_distance) {
	MknnDataset *dataset = mknn_datasetLoader_ParseVectorFile(filename_read,
			MKNN_DATATYPE_FLOATING_POINT_64bits);
	void *data = mknn_dataset_getCompactVectors(dataset);
	int64_t rows = mknn_dataset_getNumObjects(dataset);
	int64_t cols = mknn_domain_vector_getNumDimensions(
			mknn_dataset_getDomain(dataset));
	fill_matrix(state_distance, (double*) data, NULL, rows, cols);
	mknn_dataset_release(dataset);
}
static void emd_dist_save(void *state_distance, const char *id_dist,
		MknnDistanceParams *params_distance, const char *filename_write) {
	struct State_EMD_Dist *state_dist = state_distance;
	int rows = state_dist->cost_matrix->rows;
	int cols = state_dist->cost_matrix->cols;
	double *array = MY_MALLOC(2 + rows * cols, double);
	array[0] = state_dist->cost_matrix->rows;
	array[1] = state_dist->cost_matrix->cols;
	int i, j, pos = 2;
	for (i = 0; i < rows; ++i) {
		for (j = 0; j < cols; ++j) {
			CvScalar weight = cvGet2D(state_dist->cost_matrix, i, j);
			array[pos++] = weight.val[0];
		}
	}
	FILE *out = my_io_openFileWrite1(filename_write);
	fwrite(array, sizeof(double), 2 + rows * cols, out);
	fclose(out);
	free(array);
}
static void emd_dist_load(void *state_distance, const char *id_dist,
		MknnDistanceParams *params_distance, const char *filename_read) {
	int64_t filesize = 0;
	double *array = (double *) my_io_loadFileBytes(filename_read, &filesize);
	int rows = (int) array[0];
	int cols = (int) array[1];
	fill_matrix(state_distance, array + 2, NULL, rows, cols);
	free(array);
}
static void emd_dist_build(void *state_distance, const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_EMD_Dist *state_dist = state_distance;
	const char *costs_file = mknn_distanceParams_getString(params_distance,
			"costs_file");
	if (costs_file != NULL) {
		parse_file(costs_file, state_distance);
		return;
	}
	MyVectorInt *dims = my_tokenizer_splitLineInt(
			mknn_distanceParams_getString(params_distance, "matrix_dims"), 'x');
	if (dims == NULL) {
		my_log_error("either costs_file or matrix_dims is required\n");
	} else if (my_vectorInt_size(dims) < 2) {
		my_log_error(
				"matrix_dims format must contain two numbers, rows and cols\n");
	}
	int64_t rows = my_vectorInt_get(dims, 0);
	int64_t cols = my_vectorInt_get(dims, 0);
	double *array_d = (double*) mknn_distanceParams_getObject(params_distance,
			"matrix_ptrDouble");
	float *array_f = (float*) mknn_distanceParams_getObject(params_distance,
			"matrix_ptrFloat");
	const char *array_string = mknn_distanceParams_getString(params_distance,
			"matrix");
	if (array_d != NULL) {
		fill_matrix(state_dist, array_d, NULL, rows, cols);
	} else if (array_f != NULL) {
		fill_matrix(state_dist, NULL, array_f, rows, cols);
	} else if (array_string != NULL) {
		MyVectorDouble *arr = my_tokenizer_splitLineDouble(array_string, ';');
		if (my_vectorDouble_size(arr) != rows * cols) {
			my_log_error("matrix (%ix%i) contains %i elements\n", (int) rows,
					(int) cols, (int) my_vectorDouble_size(arr));
		}
		fill_matrix(state_dist, my_vectorDouble_array(arr), NULL, rows, cols);
		my_vectorDouble_release(arr);
	} else {
		my_log_info("matrix cost must be defined\n");
		mknn_predefDistance_helpPrintDistance(id_dist);
	}

}
static void emd_dist_release(void *state_distance) {
	struct State_EMD_Dist *state = state_distance;
	if (state->cost_matrix != NULL)
		cvReleaseMat(&state->cost_matrix);
	free(state);
}
static struct MknnDistanceInstance emd_dist_new(const char *id_dist,
		MknnDistanceParams *params_distance) {
	struct State_EMD_Dist *state_dist = MY_MALLOC(1, struct State_EMD_Dist);
	state_dist->normalize_vectors = mknn_distanceParams_getBool(params_distance,
			"normalize_vectors");
	struct MknnDistanceInstance df = { 0 };
	df.state_distance = state_dist;
	df.func_distance_build = emd_dist_build;
	df.func_distance_load = emd_dist_load;
	df.func_distance_save = emd_dist_save;
	df.func_distance_release = emd_dist_release;
	df.func_distanceEval_new = emd_distanceEval_new;
	return df;
}
static void emd_printHelp(const char *id_dist) {
	my_log_info(
			"    normalize_vectors=[true|false]    Optional. Normalizes to sum 1 prior to each computation.\n");
	my_log_info(
			"    costs_file=[filename]         The filename to read the matrix cost. Format: one row per line, values separated by spaces or tabs.\n");
	my_log_info(
			"    matrix_dims=RxC               The dimensions of the matrix, rows x cols, e.g. matrix_dims=3x3.\n");
	my_log_info(
			"    matrix=val1;...;valn          The cost matrix. A string with R*C numbers separated by a semicolon.\n");
}
#endif

void register_distance_emd() {
#ifndef NO_OPENCV
	mknn_register_distance(MKNN_GENERAL_DOMAIN_VECTOR, "EMD",
			"normalize_vectors=[true|false],costs_file=[filename],rows=[int],cols=[int],matrix=[val1;...;valn]",
			emd_printHelp, emd_dist_new);
#endif
}


