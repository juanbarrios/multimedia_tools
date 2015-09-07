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
#endif

struct MknnPcaAlgorithm {
	int64_t dimensions;
	double **transformation;
	double *avgs;
	double *eigenvalues;
	bool deleteStats_on_release;
	struct MyDataStatsCompute *stats;
};

MknnPcaAlgorithm *mknn_pca_new() {
	struct MknnPcaAlgorithm *kmeans = MY_MALLOC(1, struct MknnPcaAlgorithm);
	return kmeans;
}
struct Eigen {
	double eigenvalue;
	double *eigenvector;
};
static int compare_eigen(const void *a, const void *b) {
	double da = ((struct Eigen*) a)->eigenvalue;
	double db = ((struct Eigen*) b)->eigenvalue;
	return my_compare_double(db, da);
}
void mknn_pca_addDatasetToVectorStats(MknnPcaAlgorithm *pca, MknnDataset *dataset) {
	my_log_info_time("PCA: computing statistics in dataset, size %"PRIi64"\n",
			mknn_dataset_getNumObjects(dataset));
	if (pca->stats == NULL) {
		pca->stats = mknn_dataset_computeDataStats(dataset);
		pca->deleteStats_on_release = true;
	} else {
		mknn_dataset_computeStatsVectors(dataset, pca->stats);
	}
}
void mknn_pca_setVectorStats(MknnPcaAlgorithm *pca,
		struct MyDataStatsCompute *stats, bool deleteStats_on_release) {
	pca->stats = stats;
	pca->deleteStats_on_release = deleteStats_on_release;
}
void mknn_pca_computeTransformationMatrix(MknnPcaAlgorithm *pca) {
#ifndef NO_OPENCV
	pca->dimensions = my_math_computeStats_getNumDimensions(pca->stats);
	my_log_info_time("PCA: computing principal components, dim %"PRIi64"\n",
			pca->dimensions);
	pca->avgs = MY_MALLOC(pca->dimensions, double);
	for (int64_t i = 0; i < pca->dimensions; ++i) {
		struct MyDataStats vals = { 0 };
		my_math_computeStats_getStats(pca->stats, i, &vals);
		pca->avgs[i] = vals.average;
	}
	CvMat* covMat = cvCreateMat(pca->dimensions, pca->dimensions, CV_32FC1);
	for (int64_t i = 0; i < pca->dimensions; i++) {
		for (int64_t j = 0; j <= i; j++) {
			double cov = my_math_computeStats_getCovariance(pca->stats, i, j);
			cvSetReal2D(covMat, i, j, cov);
			cvSetReal2D(covMat, j, i, cov);
		}
	}
	CvMat* evects = cvCreateMat(pca->dimensions, pca->dimensions, CV_32FC1);
	CvMat* evals = cvCreateMat(pca->dimensions, 1, CV_32FC1);
	cvEigenVV(covMat, evects, evals, DBL_EPSILON, -1, -1);
	struct Eigen *eigens = MY_MALLOC(pca->dimensions, struct Eigen);
	for (int64_t i = 0; i < pca->dimensions; ++i) {
		struct Eigen *e = eigens + i;
		e->eigenvalue = cvGetReal2D(evals, i, 0);
		e->eigenvector = MY_MALLOC_NOINIT(pca->dimensions, double);
		for (int64_t j = 0; j < pca->dimensions; ++j)
			e->eigenvector[j] = cvGetReal2D(evects, i, j);
	}
	qsort(eigens, pca->dimensions, sizeof(struct Eigen), compare_eigen);
	pca->eigenvalues = MY_MALLOC_NOINIT(pca->dimensions, double);
	pca->transformation = MY_MALLOC_NOINIT(pca->dimensions, double*);
	for (int64_t i = 0; i < pca->dimensions; ++i) {
		pca->eigenvalues[i] = eigens[i].eigenvalue;
		pca->transformation[i] = eigens[i].eigenvector;
	}
	free(eigens);
	cvReleaseMat(&evects);
	cvReleaseMat(&evals);
	cvReleaseMat(&covMat);
	my_log_info_time("PCA: completed ok\n");
#else
	my_log_error("ERROR, PCA was disabled during compilation\n");
#endif
}
void mknn_pca_save(MknnPcaAlgorithm *pca, const char *filename_write,
bool include_debug) {
	FILE *out = my_io_openFileWrite1Config(filename_write, "MetricKnn",
			"PcaState", 1, 0);
	fprintf(out, "pca.num_dimensions=%"PRIi64"\n", pca->dimensions);
	fprintf(out, "--\n\n");
	fwrite(pca->avgs, sizeof(double), pca->dimensions, out);
	for (int64_t i = 0; i < pca->dimensions; ++i) {
		fwrite(pca->transformation[i], sizeof(double), pca->dimensions, out);
	}
	fwrite(pca->eigenvalues, sizeof(double), pca->dimensions, out);
	fclose(out);
	if (include_debug) {
		MknnDataset *dat = mknn_datasetLoader_PointerCompactVectors(pca->avgs,
		false, 1, pca->dimensions, MKNN_DATATYPE_FLOATING_POINT_64bits);
		char *fname2 = my_newString_format("%s.averages.txt", filename_write);
		mknn_dataset_printObjectsTextFile(dat, fname2);
		free(fname2);
		mknn_dataset_release(dat);
	}
	if (include_debug) {
		double total = 0, sum = 0;
		for (int64_t i = 0; i < pca->dimensions; ++i)
			total += pca->eigenvalues[i];
		char *fname2 = my_newString_format("%s.eigenvalues.txt",
				filename_write);
		FILE *out = my_io_openFileWrite1(fname2);
		fprintf(out, "#dimension\teigenvalue\taccumulated_proportion\n");
		for (int64_t i = 0; i < pca->dimensions; ++i) {
			sum += pca->eigenvalues[i];
			char *num = my_newString_int(i + 1);
			char *val = my_newString_double(pca->eigenvalues[i]);
			char *pct = my_newString_double(sum / total);
			fprintf(out, "%s\t%s\t%s\n", num, val, pct);
			MY_FREE_MULTI(num, val, pct);
		}
		fclose(out);
		free(fname2);
	}
	if (include_debug) {
		MknnDataset *dat = mknn_datasetLoader_PointerArray(
				(void**) pca->transformation, pca->dimensions,
				mknn_domain_newVector(pca->dimensions,
						MKNN_DATATYPE_FLOATING_POINT_64bits), false, false,
				true);
		char *fname2 = my_newString_format("%s.transformation.txt",
				filename_write);
		mknn_dataset_printObjectsTextFile(dat, fname2);
		free(fname2);
		mknn_dataset_release(dat);
	}
	if (include_debug) {
		char *fname2 = my_newString_format("%s.stats.txt", filename_write);
		FILE *out = my_io_openFileWrite1(fname2);
		free(fname2);
		for (int64_t i = 0; i < pca->dimensions; ++i) {
			struct MyDataStats dat = { 0 };
			my_math_computeStats_getStats(pca->stats, i, &dat);
			char *st = my_math_statsDetail_newString(&dat, "", "\t", "\n");
			fprintf(out, "DIMENSION_%"PRIi64"\n%s\n", i, st);
			free(st);
		}
		double **mat = MY_MALLOC_MATRIX(pca->dimensions, pca->dimensions,
				double);
		my_math_computeStats_fillCovarianceMatrix(pca->stats, mat);
		fprintf(out, "\nCOVARIANCES\n");
		for (int64_t i = 0; i < pca->dimensions; ++i) {
			char *st = my_newString_arrayDouble(mat[i], pca->dimensions, '\t');
			fprintf(out, "%s\n", st);
			free(st);
		}
		fclose(out);
		MY_FREE_MATRIX(mat, pca->dimensions);
	}
}
void mknn_pca_restore(MknnPcaAlgorithm *pca, const char *filename_read) {
	FILE *input = my_io_openFileRead1(filename_read, true);
	MyLineReader *reader = my_lreader_config_open_params(input, 1, "MetricKnn",
			"PcaState", 1, 0);
	int64_t num_dimensions = -1;
	for (;;) {
		const char *line = my_lreader_readLine(reader);
		if (line == NULL || my_string_equals(line, "--"))
			break;
		int64_t pos = my_string_indexOf(line, "=");
		if (my_string_startsWith_ignorecase(line, "pca.num_dimensions=")) {
			num_dimensions = my_parse_int(line + pos + 1);
		} else {
			my_log_error("invalid line %s\n", line);
		}
	}
	my_lreader_close(reader, false);
	if (num_dimensions < 0)
		my_log_error("invalid format %s\n", filename_read);
	pca->dimensions = num_dimensions;
	pca->avgs = MY_MALLOC_NOINIT(pca->dimensions, double);
	pca->eigenvalues = MY_MALLOC_NOINIT(pca->dimensions, double);
	pca->transformation = MY_MALLOC_NOINIT(pca->dimensions, double*);
	int64_t expected_size = pca->dimensions * sizeof(double);
	my_io_readBytesFile(input, pca->avgs, expected_size, false);
	for (int64_t i = 0; i < pca->dimensions; ++i) {
		pca->transformation[i] = MY_MALLOC_NOINIT(pca->dimensions, double);
		my_io_readBytesFile(input, pca->transformation[i], expected_size,
		false);
	}
	my_io_readBytesFile(input, pca->eigenvalues, expected_size, true);
	fclose(input);
}
int64_t mknn_pca_getInputDimension(MknnPcaAlgorithm *pca) {
	return pca->dimensions;
}

#define TRANSFORM_FUNCTIONS(typeVector1, typeVector2) \
static void transformVector_##typeVector1##_##typeVector2(MknnPcaAlgorithm *pca, void *vector1, \
void *vector2, int64_t dim_vector2) { \
	int64_t dim_vector1 = pca->dimensions; \
	double *averages = pca->avgs; \
	typeVector1 *v_in = vector1; \
	typeVector2 *v_out = vector2; \
	for (int64_t i = 0; i < dim_vector2; ++i) { \
		double *matcolumn = pca->transformation[i]; \
		double sum = 0; \
		for (int64_t j = 0; j < dim_vector1; ++j) { \
			sum += (v_in[j] - averages[j]) * matcolumn[j]; \
		} \
		v_out[i] = sum; \
	} \
}
GENERATE_DOUBLE_DATATYPE(TRANSFORM_FUNCTIONS)

mknn_pca_func_transformVector mknn_pca_getTransformVectorFunction(
		MknnDatatype dtype_vector_src, MknnDatatype dtype_vector_dst) {
	mknn_pca_func_transformVector transformFunc;
	ASSIGN_DOUBLE_DATATYPE(transformFunc, dtype_vector_src, dtype_vector_dst,
			transformVector_)
	return transformFunc;
}

void mknn_pca_transform_dataset(MknnPcaAlgorithm *pca, MknnDataset *dataset_in,
		MknnDataset *dataset_out) {
	MknnDomain *dom_in = mknn_dataset_getDomain(dataset_in);
	MknnDomain *dom_out = mknn_dataset_getDomain(dataset_out);
	my_assert_isTrue("vector", mknn_domain_isGeneralDomainVector(dom_in));
	my_assert_isTrue("vector", mknn_domain_isGeneralDomainVector(dom_out));
	int64_t dims_in = mknn_domain_vector_getNumDimensions(dom_in);
	int64_t dims_out = mknn_domain_vector_getNumDimensions(dom_out);
	my_assert_equalInt("dimension input", dims_in, pca->dimensions);
	my_assert_lessEqualInt("dimension output", dims_out, pca->dimensions);
	int64_t numvec_in = mknn_dataset_getNumObjects(dataset_in);
	int64_t numvec_out = mknn_dataset_getNumObjects(dataset_out);
	my_assert_equalInt("dataset size", numvec_out, numvec_in);
	MknnDatatype dtype_in = mknn_domain_vector_getDimensionDataType(dom_in);
	MknnDatatype dtype_out = mknn_domain_vector_getDimensionDataType(dom_out);
	mknn_pca_func_transformVector transformFunc =
			mknn_pca_getTransformVectorFunction(dtype_in, dtype_out);
	for (int64_t i = 0; i < numvec_in; ++i) {
		void *input = mknn_dataset_getObject(dataset_in, i);
		void *output = mknn_dataset_getObject(dataset_out, i);
		transformFunc(pca, input, output, dims_out);
	}
}

void mknn_pca_release(MknnPcaAlgorithm *pca) {
	if (pca == NULL)
		return;
	MY_FREE_MATRIX(pca->transformation, pca->dimensions);
	MY_FREE(pca->avgs);
	MY_FREE(pca->eigenvalues);
	if (pca->deleteStats_on_release)
		my_math_computeStats_release(pca->stats);
	free(pca);
}
