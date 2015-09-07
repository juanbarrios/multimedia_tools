/*
 * Copyright (C) 2014-2015, ORAND S.A. <http://www.orand.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include <iostream>

#ifndef NO_OPENCV

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/contrib/contrib.hpp>
#include "metricknn_cli.hpp"

class State {
public:
	MknnDataset *dataset;
	int image_size;
};

static void callback_method(MknnKmeansAlgorithm *kmeans, int64_t num_iteration,
bool is_last_iteration, void *state_pointer) {
	//if (!is_last_iteration)
	//return;
	State *state = reinterpret_cast<State*>(state_pointer);
	int64_t *assign = mknn_kmeans_getAssignations(kmeans, false);
	MknnDataset *centers = mknn_kmeans_getCentroids(kmeans, false);
	cv::Mat img2(state->image_size, state->image_size, CV_8UC3);
	cv::rectangle(img2, cv::Point(0, 0), cv::Point(img2.cols, img2.rows),
			cv::Scalar(0, 0, 66), -1);
	for (int i = 0; i < mknn_dataset_getNumObjects(state->dataset); ++i) {
		int *vec = reinterpret_cast<int*>(mknn_dataset_getObject(state->dataset,
				i));
		double val = (180.0 * assign[i]) / mknn_dataset_getNumObjects(centers);
		cv::Scalar col = cv::Scalar(std::round(val), 255, 255);
		if (assign[i] < 0)
			col = cv::Scalar(0, 0, 200);
		cv::Point point = cv::Point(vec[0], vec[1]);
		cv::circle(img2, point, 2, col, -1);
		if (false) {
			std::string ss = my::toString::intValue(i);
			int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
			double fontScale = 0.3;
			int thickness = 1;
			cv::putText(img2, ss, point, fontFace, fontScale, col, thickness);
		}
		if (false && i > 0) {
			int *pvec = reinterpret_cast<int*>(mknn_dataset_getObject(
					state->dataset, i - 1));
			cv::Point ppoint = cv::Point(pvec[0], pvec[1]);
			cv::line(img2, ppoint, point, cv::Scalar(30, 100, 200), 1);
		}
	}
	for (int i = 0; i < mknn_dataset_getNumObjects(centers); ++i) {
		float *vec =
				reinterpret_cast<float*>(mknn_dataset_getObject(centers, i));
		cv::Scalar col(30, 220, 255);
		cv::Point point = cv::Point(vec[0], vec[1]);
		cv::circle(img2, point, 5, col, -1);
		std::string ss = my::toString::intValue(i);
		int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
		double fontScale = 0.5;
		int thickness = 1;
		cv::putText(img2, ss, point, fontFace, fontScale, col, thickness);
	}
	cv::Mat img3;
	cv::cvtColor(img2, img3, CV_HSV2BGR);
	std::cout << (is_last_iteration ? "[END] " : "") << "iteration "
			<< num_iteration << std::endl;
	cv::imshow("iteration", img3);
	cv::waitKey(0);
}
static void showMatrixToScreen(std::string imgname, std::vector<double> matrix,
		int n, std::vector<int> &limits_clusters) {
	cv::Mat mdist(n, n, CV_64FC1, matrix.data());
	cv::Mat mdist_norm;
	cv::normalize(mdist, mdist_norm, 0, 255, cv::NORM_MINMAX, CV_8U);
	mdist_norm = 255 - mdist_norm;
	cv::Mat mdist_color;
	cv::applyColorMap(mdist_norm, mdist_color, cv::COLORMAP_JET);
	cv::Scalar black = cv::Scalar::all(0);
	for (int pos : limits_clusters) {
		cv::line(mdist_color, cv::Point(pos, 0),
				cv::Point(pos, mdist_color.rows), black, 2);
		cv::line(mdist_color, cv::Point(0, pos),
				cv::Point(mdist_color.cols, pos), black, 2);
	}
	cv::imshow(imgname, mdist_color);
	cv::waitKey(0);
}
static void printDistanceMatrix(MknnKmeansAlgorithm *kmeans) {
	MknnDataset *dataset = mknn_kmeans_getDataset(kmeans);
	MknnDistance *distance = mknn_kmeans_getDistance(kmeans);
	int64_t n = mknn_dataset_getNumObjects(dataset);
	std::vector<double> matrix(n * n);
	mknn_fillDistanceMatrix(dataset, distance, matrix.data(), 8);
	std::vector<int> limits_clusters;
	showMatrixToScreen("distances", matrix, n, limits_clusters);
	int64_t *assign = mknn_kmeans_getAssignations(kmeans, false);
	int64_t nc = mknn_kmeans_getNumCentroids(kmeans);
	std::vector<int> new_pos(n * n);
	int cont = 0;
	for (int j = 0; j < nc; j++) {
		limits_clusters.push_back(cont);
		for (int i = 0; i < n; i++) {
			if (assign[i] == j)
				new_pos[cont++] = i;
		}
	}
	my::assert::equalInt("n", cont, n);
	std::vector<double> matrix2(n * n);
	for (int row = 0; row < n; row++) {
		for (int col = 0; col < n; col++) {
			matrix2[row * n + col] = matrix[new_pos[row] * n + new_pos[col]];
		}
	}
	showMatrixToScreen("distances2", matrix2, n, limits_clusters);
}

static MknnDataset *generate_dataset(int image_size) {
	MknnDataset *dataset1 = mknn_datasetLoader_UniformRandomVectors(100, 2, 0,
			image_size, MKNN_DATATYPE_SIGNED_INTEGER_32bits);
	MknnDataset *dataset2 = mknn_datasetLoader_UniformRandomVectors(100, 2,
			image_size / 4, image_size / 2,
			MKNN_DATATYPE_SIGNED_INTEGER_32bits);
	MknnDataset *dataset3 = mknn_datasetLoader_UniformRandomVectors(100, 2,
			(image_size * 4) / 5, image_size,
			MKNN_DATATYPE_SIGNED_INTEGER_32bits);
	MknnDataset *dataset4 = mknn_datasetLoader_UniformRandomVectors(100, 2, 0,
			image_size / 5, MKNN_DATATYPE_SIGNED_INTEGER_32bits);
	MknnDataset *arr[] = { dataset1, dataset2, dataset3, dataset4 };
	MknnDataset *dataset_pre = mknn_datasetLoader_Concatenate(4, arr, true);
	MknnDataset *dataset = mknn_datasetLoader_reorderRandomPermutation(
			dataset_pre, true);
	return dataset;
}

int main_demo_kmeans(std::vector<std::string> &args, unsigned int i) {
	int image_size = 600;
	int num_centers = 2;
	if (args.size() <= i + 1) {
		std::cout << "Usage: " << my::collection::args_getBinaryName(args)
				<< std::endl;
		std::cout << "    -kmeans -demo" << std::endl;
		std::cout << "    -numCenters [num]           Default=" << num_centers
				<< std::endl;
		std::cout << "    -imageSize [pixels]        Default=" << image_size
				<< std::endl;
		return EXIT_FAILURE;
	}
	while (i < args.size()) {
		if (my::collection::is_next_arg_equal("-numCenters", args, i))
			num_centers = my::collection::next_arg_int(args, i);
		else if (my::collection::is_next_arg_equal("-image_size", args, i))
			image_size = my::collection::next_arg_int(args, i);
		else
			throw std::runtime_error(
					"unknown parameter " + my::collection::next_arg(args, i));
	}
	my::assert::greaterInt("numCenters", num_centers, 0);
	my::assert::greaterInt("imageSize", image_size, 0);
	MknnDataset *dataset1 = generate_dataset(image_size);
	MknnDistance *distance = mknn_distance_newPredefined(
			mknn_predefDistance_L2(), true);
	MknnDataset *dataset;
	if (false) {
		dataset = mknn_datasetLoader_reorderNearestNeighbor(dataset1, distance,
				-1, true);
	} else {
		dataset = dataset1;
	}
	MknnKmeansAlgorithm *kmeans = mknn_kmeans_new();
	mknn_kmeans_setDataset(kmeans, dataset);
	mknn_kmeans_setDistance(kmeans, distance);
	mknn_kmeans_setNumCentroids(kmeans, num_centers);
	State state;
	state.dataset = dataset;
	state.image_size = image_size;
	mknn_kmeans_setIterationCallBackFunction(kmeans, callback_method, &state);
	mknn_kmeans_perform(kmeans);
	printDistanceMatrix(kmeans);
	return EXIT_SUCCESS;
}
#endif
