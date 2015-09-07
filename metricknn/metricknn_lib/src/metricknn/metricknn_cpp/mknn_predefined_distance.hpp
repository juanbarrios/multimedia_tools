/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PREDEFINED_DISTANCE_HPP_
#define MKNN_PREDEFINED_DISTANCE_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 * MetricKnn provides a set of pre-defined distances.
 *
 * The generic way for instantiating a predefined distance is to use the method
 * Distance::newPredefined, which requires the ID and parameters of the distance.
 *
 * The complete list of predefined distances can be listed by calling
 * Distance::helpListDistances. The parameters supported by each distance
 * can be listed by calling PredefDistance::helpPrintDistance.
 *
 * This class contains some functions to ease the instantiation of some predefined distances.
 *
 */

class PredefDistance {
public:
	/**
	 * @name Help functions
	 * @{
	 */
	/**
	 * Lists to standard output all pre-defined distances.
	 */
	static void helpListDistances();

	/**
	 * Prints to standard output the help for a distance.
	 *
	 * @param id_dist the unique identifier of a pre-defined distance.
	 */
	static void helpPrintDistance(std::string id_dist);

	/**
	 * Tests whether the given string references a valid pre-defined distance.
	 *
	 * @param id_dist the unique identifier of a pre-defined distance.
	 * @return true whether @p id_dist corresponds to a pre-defined distance, and false otherwise.
	 */
	static bool testDistanceId(std::string id_dist);

	/**
	 * @}
	 */

	/**
	 * Creates an object for Manhattan or Taxi-cab distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{L1}(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \sum_{i=1}^{n} |x_i - y_i|
	 * \f]
	 *
	 * This distance satisfies the metric properties, therefore it can be used by Metric Indexes to obtain exact nearest neighbors.
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams L1();

	/**
	 * Creates an object for Euclidean distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{L2}(\{x_1,...,x_n\},\{y_1,...,y_n\}) =  \sqrt{ \sum_{i=1}^{n} (x_i - y_i)^2 }
	 * \f]
	 *
	 * This distance satisfies the metric properties, therefore it can be used by Metric Indexes to obtain exact nearest neighbors.
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams L2();

	/**
	 * Creates an object for L-max distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{Lmax}(\{x_1,...,x_n\},\{y_1,...,y_n\}) =  \max_{i \in \{1,...,n\}} |x_i - y_i|
	 * \f]
	 *
	 * This distance satisfies the metric properties, therefore it can be used by Metric Indexes to obtain exact nearest neighbors.
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams Lmax();

	/**
	 * Creates an object for Minkowski distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{Lp}(\{x_1,...,x_n\},\{y_1,...,y_n\}) =  \left( {\sum_{i=1}^n |x_i-y_i|^p } \right)^{\frac{1}{p}}
	 * \f]
	 *
	 * @note This distance satisfies the metric properties only when \f$ p \geq 1 \f$.
	 * When \f$ 0 < p < 1 \f$ the Metric Indexes may not obtain exact nearest neighbors.
	 *
	 * @param order the order \f$ p \f$ of the distance  \f$ p > 0 \f$.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams Lp(double order);

	/**
	 * Creates an object for Hamming distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{Hamming}(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \sum_{i=1}^n \bar{p}_i
	 * \f]
	 *
	 * where \f$ \bar{p}_i= \left\{ \begin{array}{ll} 0 & x_i = y_i\\ 1 & x_i \neq y_i\\ \end{array} \right. \f$ .
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams Hamming();

	/**
	 * Creates an object for Chi2 distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \chi^2(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \sum_{i=1}^n \frac{ (x_i - \bar{m}_i )^2 }{ \bar{m}_i }
	 * \f]
	 *
	 * where \f$ \bar{m}_i=\frac{x_i+y_i}{2} \f$ .
	 *
	 * @note This distance does not satisfy the metric properties.
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams Chi2();

	/**
	 * Creates an object for Hellinger distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{Hellinger}(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \sqrt { \frac { \sum_{i=1}^n ( \sqrt{x_i} -  \sqrt{y_i} )^2} { 2 } }
	 * \f]
	 *
	 * @note This distance does not satisfy the metric properties.
	 *
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams Hellinger();

	/**
	 * Creates an object for Cosine Similarity.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{cos}(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \frac { \sum_{i=1}^n x_i \cdot y_i } {\sqrt{ \sum_{i=1}^{n} {x_i}^2 } \cdot \sqrt{ \sum_{i=1}^{n} {y_i}^2 } }
	 * \f]
	 *
	 * @note This is a similarity function, therefore a search for the Farthest Neighbors is needed. See #CosineDistance
	 * for a distance version.
	 *
	 * @param normalize_vectors Computes the euclidean norm for each vector. If this is set to false, it assumes the vectors are already normalized
	 * thus the value \f$ \sqrt{ \sum_{i=1}^{n} {x_i}^2 } \cdot \sqrt{ \sum_{i=1}^{n} {y_i}^2 } \f$ is equal to 1.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams CosineSimilarity(bool normalize_vectors);

	/**
	 * Creates an object for Cosine Distance.
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{CosineDistance}(\vec{x},\vec{y}) = \sqrt{ 2 ( 1 - \cos(\vec{x},\vec{y})) }
	 * \f]
	 *
	 * where \f$ \cos(\vec{x},\vec{y}) \f$ is the cosine similarity between vectors \f$ \vec{x} \f$ and \f$ \vec{y} \f$ as defined in #CosineSimilarity.
	 *
	 * The nearest neighbors obtained by this distance are identical to the farthest neighbor obtained by cosine similarity (if vectors
	 * are normalized). Therefore, this distance can be used accelerate the search using cosine similarity.
	 *
	 * @param normalize_vectors The cosine similarity must normalize vectors to euclidean norm 1 prior to each computation.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams CosineDistance(bool normalize_vectors);

	/**
	 * Creates an object for Earth Mover's Distance.
	 *
	 * This function uses OpenCV's implementation, see http://docs.opencv.org/modules/imgproc/doc/histograms.html#emd .
	 *
	 * @note Depending on the cost_matrix this distance may or may not satisfy the metric properties. If the values in cost_matrix
	 * where computed by a metric distance, then the EMD will also be a metric distance.
	 *
	 * @param matrix_rows
	 * @param matrix_cols
	 * @param cost_matrix an array of length <tt>matrix_rows * matrix_cols</tt> with the
	 * cost for each pair of dimensions.
	 * @param normalize_vectors normalizes (sum 1) both vectors before computing the distance.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams EMD(long long matrix_rows, long long matrix_cols,
			double *cost_matrix, bool normalize_vectors);

	/**
	 * Creates an object for Dynamic Partial Function distance. See definition http://dx.doi.org/10.1109/ICIP.2002.1040021 .
	 *
	 * The distance between two n-dimensional vectors is defined as:
	 * \f[
	 *  \textrm{DPF}(\{x_1,...,x_n\},\{y_1,...,y_n\}) = \left( {\sum_{i \in \Delta_m} |x_i-y_i|^p } \right)^{\frac{1}{p}}
	 * \f]
	 *
	 * where \f$ \Delta_m \f$ is the subset of the \f$ m \f$ smallest values of \f$ |x_i-y_i| \f$.
	 *
	 * @param order the order \f$ p \f$ of the distance  \f$ p > 0 \f$.
	 * @param num_dims_discard fixed number of dimensions to discard <tt>0 @< num_dims_discard @< num_dimensions</tt>.
	 * @param pct_discard fixed number of dimensions to discard computed as a fraction of @p num_dimensions <tt>0 @< pct_discard @< 1</tt>.
	 * <tt>num_dims_discard = round(pct_discard * num_dimensions)</tt>
	 * @param threshold_discard discard all dimensions which difference is higher than @p threshold_discard.
	 * It produces a variable number of dimensions to discard.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams DPF(double order, long long num_dims_discard,
			double pct_discard, double threshold_discard);

	/**
	 * Defines a multi-distance, which is a weighted combination of distances.
	 *
	 * @warning Under construction.
	 *
	 * @param subdistances the distances to combine
	 * @param free_subdistances_on_release to release the subdistances together with this distance
	 * @param normalization_values the value to divide each distance.
	 * @param ponderation_values the value to weight each distance.
	 * @param with_auto_config run algorithms to automatically locate normalization or ponderation values.
	 * @param auto_config_dataset the data to be used by the algorithms.
	 * @param auto_normalize_alpha the value to be used by the alpha-normalization.
	 * @param auto_ponderation_maxrho run the automatic ponderation according to max rho criterium.
	 * @param auto_ponderation_maxtau run the automatic ponderation according to max tau criterium.
	 * @return parameters to create a distance (it must be deleted)
	 */
	static DistanceParams MultiDistance(
			const std::vector<Distance> &subdistances,
			bool free_subdistances_on_release,
			const std::vector<double> &normalization_values,
			const std::vector<double> &ponderation_values,
			bool with_auto_config, Dataset &auto_config_dataset,
			double auto_normalize_alpha, bool auto_ponderation_maxrho,
			bool auto_ponderation_maxtau);

};

}

#endif
