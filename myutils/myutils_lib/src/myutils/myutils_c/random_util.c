/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "random_util.h"
#include <time.h>

#define MAX_SEEDED_THREADS 20

static int last_seeded = -1;
static int cont_seeded = 0;
static pthread_t seeded_threads[MAX_SEEDED_THREADS];

static bool was_seeded_thread(pthread_t thread) {
	if (cont_seeded == 0)
		return false;
	if (seeded_threads[last_seeded] == thread)
		return true;
	for (int i = 0; i < cont_seeded; ++i) {
		if (seeded_threads[i] == thread)
			return true;
	}
	return false;
}

static void add_new_seeded_thread(pthread_t thread) {
	last_seeded = (last_seeded + 1) % MAX_SEEDED_THREADS;
	seeded_threads[last_seeded] = thread;
	if (cont_seeded < MAX_SEEDED_THREADS)
		cont_seeded++;
}

static unsigned int current_seed = 0;

static void internal_ensure_seed() {
	//in linux seeds are stored per process
	//in windows seeds are stored per thread
	//therefore we seed every new thread that has not been seeded
	//we store a memory of the last MAX_SEEDED_THREADS
	pthread_t current_thread = pthread_self();
	if (was_seeded_thread(current_thread))
		return;
	if (current_seed == 0) {
		current_seed = (unsigned int) time(NULL);
		if (current_seed == 0)
			current_seed = 1;
	} else {
		current_seed += 10;
	}
	srand(current_seed);
	add_new_seeded_thread(current_thread);
}

#if (RAND_MAX >= 0x7FFFFFFF)

static int64_t MAX_RANDOM_31b = 0x7FFFFFFF;
static int64_t MAX_RANDOM_52b = 0xFFFFFFFFFFFFF;
static int64_t MAX_RANDOM_62b = 0x3FFFFFFFFFFFFFFF;

static int64_t internal_random_int_31b() {
	int64_t a0 = (rand() & 0x7FFFFFFF); //31-bits
	return a0;
}
static int64_t internal_random_int_52b() {
	int64_t a0 = (rand() & 0x3FFFFFF); //26-bits
	int64_t a1 = (rand() & 0x3FFFFFF);//26-bits
	return a0 | (a1 << 26);
}
static int64_t internal_random_int_62b() {
	int64_t a0 = (rand() & 0x7FFFFFFF); //31-bits
	int64_t a1 = (rand() & 0x7FFFFFFF);//31-bits
	return a0 | (a1 << 31);
}
static int64_t internal_random_int(int64_t maxValueNotIncluded) {
	my_assert_greaterInt("random_int maxValue", maxValueNotIncluded, 0);
	int64_t val = 0;
	if (maxValueNotIncluded <= MAX_RANDOM_31b)
	val = internal_random_int_31b();
	else if (maxValueNotIncluded <= MAX_RANDOM_62b)
	val = internal_random_int_62b();
	else
	my_log_error("random_int maxValue too large\n");
	//TODO: remove bias to small values
	int64_t rnd = val % maxValueNotIncluded;
	return rnd;
}
#elif (RAND_MAX >= 0x7FFF)

static int64_t MAX_RANDOM_15b = 0x7FFF;
static int64_t MAX_RANDOM_30b = 0x3FFFFFFF;
static int64_t MAX_RANDOM_45b = 0x1FFFFFFFFFFF;
static int64_t MAX_RANDOM_52b = 0xFFFFFFFFFFFFF;
static int64_t MAX_RANDOM_60b = 0xFFFFFFFFFFFFFFF;

static int64_t internal_random_int_15b() {
	int64_t a0 = (rand() & 0x7FFF); //15-bits
	return a0;
}
static int64_t internal_random_int_30b() {
	int64_t a0 = (rand() & 0x7FFF); //15-bits
	int64_t a1 = (rand() & 0x7FFF); //15-bits
	return a0 | (a1 << 15);
}
static int64_t internal_random_int_45b() {
	int64_t a0 = (rand() & 0x7FFF); //15-bits
	int64_t a1 = (rand() & 0x7FFF); //15-bits
	int64_t a2 = (rand() & 0x7FFF); //15-bits
	return a0 | (a1 << 15) | (a2 << 30);
}
static int64_t internal_random_int_52b() {
	int64_t a0 = (rand() & 0x1FFF); //13-bits
	int64_t a1 = (rand() & 0x1FFF); //13-bits
	int64_t a2 = (rand() & 0x1FFF); //13-bits
	int64_t a3 = (rand() & 0x1FFF); //13-bits
	return a0 | (a1 << 13) | (a2 << 26) | (a3 << 39);
}
static int64_t internal_random_int_60b() {
	int64_t a0 = (rand() & 0x7FFF); //15-bits
	int64_t a1 = (rand() & 0x7FFF); //15-bits
	int64_t a2 = (rand() & 0x7FFF); //15-bits
	int64_t a3 = (rand() & 0x7FFF); //15-bits
	return a0 | (a1 << 15) | (a2 << 30) | (a3 << 45);
}
static int64_t internal_random_int(int64_t maxValueNotIncluded) {
	my_assert_greaterInt("random_int maxValue", maxValueNotIncluded, 0);
	int64_t val = 0;
	if (maxValueNotIncluded <= MAX_RANDOM_15b)
		val = internal_random_int_15b();
	else if (maxValueNotIncluded <= MAX_RANDOM_30b)
		val = internal_random_int_30b();
	else if (maxValueNotIncluded <= MAX_RANDOM_45b)
		val = internal_random_int_45b();
	else if (maxValueNotIncluded <= MAX_RANDOM_60b)
		val = internal_random_int_60b();
	else
		my_log_error("random_int maxValue too large\n");
	//TODO: remove bias to small values
	int64_t rnd = val % maxValueNotIncluded;
	return rnd;
}
#endif

static double internal_random_double(double maxValueNotIncluded) {
	double rnd = ((double) internal_random_int_52b())
			/ ((double) MAX_RANDOM_52b);
	double ret = rnd * maxValueNotIncluded;
	if (ret >= maxValueNotIncluded)
		return internal_random_double(maxValueNotIncluded);
	return ret;
}

MY_MUTEX_NEWSTATIC(rnd_mutex);

void my_random_setSeed(int64_t seed) {
	MY_MUTEX_LOCK(rnd_mutex);
	current_seed = (unsigned int) seed;
	srand(current_seed);
	add_new_seeded_thread(pthread_self());
	MY_MUTEX_UNLOCK(rnd_mutex);
}

int64_t my_random_int(int64_t minValueIncluded, int64_t maxValueNotIncluded) {
	MY_MUTEX_LOCK(rnd_mutex);
	internal_ensure_seed();
	int64_t interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterInt("random interval", interval_size, 0);
	int64_t rnd = minValueIncluded + internal_random_int(interval_size);
	MY_MUTEX_UNLOCK(rnd_mutex);
	return rnd;
}
void my_random_intList(int64_t minValueIncluded, int64_t maxValueNotIncluded,
		int64_t *array, int64_t size) {
	MY_MUTEX_LOCK(rnd_mutex);
	internal_ensure_seed();
	int64_t interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterInt("random interval", interval_size, 0);
	for (int64_t i = 0; i < size; ++i) {
		array[i] = minValueIncluded + internal_random_int(interval_size);
	}
	MY_MUTEX_UNLOCK(rnd_mutex);
}
double my_random_double(double minValueIncluded, double maxValueNotIncluded) {
	MY_MUTEX_LOCK(rnd_mutex);
	internal_ensure_seed();
	double interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterDouble("random interval", interval_size, 0);
	double rnd = minValueIncluded + internal_random_double(interval_size);
	MY_MUTEX_UNLOCK(rnd_mutex);
	return rnd;
}
void my_random_doubleList(double minValueIncluded, double maxValueNotIncluded,
		double *array, int64_t size) {
	MY_MUTEX_LOCK(rnd_mutex);
	internal_ensure_seed();
	double interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterDouble("random interval", interval_size, 0);
	for (int64_t i = 0; i < size; ++i) {
		array[i] = minValueIncluded + internal_random_double(interval_size);
	}
	MY_MUTEX_UNLOCK(rnd_mutex);
}
int64_t *my_random_newPermutation(int64_t minValueIncluded,
		int64_t maxValueNotIncluded) {
	int64_t interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterInt("random interval", interval_size, 0);
	int64_t *array = MY_MALLOC(interval_size, int64_t);
	for (int64_t i = 0; i < interval_size; ++i)
		array[i] = minValueIncluded + i;
	MY_MUTEX_LOCK(rnd_mutex);
	internal_ensure_seed();
	for (int64_t i = 0; i < interval_size - 1; i++) {
		int64_t rnd = internal_random_int(interval_size - i);
		int64_t pos = i + rnd;
		int64_t swp = array[i];
		array[i] = array[pos];
		array[pos] = swp;
	}
	MY_MUTEX_UNLOCK(rnd_mutex);
	return array;
}
void my_random_intList_noRepetitions(int64_t minValueIncluded,
		int64_t maxValueNotIncluded, int64_t *array, int64_t sample_size) {
	int64_t interval_size = maxValueNotIncluded - minValueIncluded;
	my_assert_greaterInt("random interval", interval_size, 0);
	if (sample_size > interval_size)
		my_log_error(
				"cannot select %"PRIi64" values between %"PRIi64" possible values\n",
				sample_size, interval_size);
	if (sample_size > interval_size / 100) {
		int64_t *ids = my_random_newPermutation(minValueIncluded,
				maxValueNotIncluded);
		memcpy(array, ids, sample_size * sizeof(int64_t));
		MY_FREE(ids);
	} else {
		MY_MUTEX_LOCK(rnd_mutex);
		internal_ensure_seed();
		for (int64_t i = 0; i < sample_size; ++i) {
			array[i] = minValueIncluded + internal_random_int(interval_size);
			for (int64_t j = 0; j < i; ++j) {
				if (array[i] == array[j]) {
					i--;
					break;
				}
			}
		}
		MY_MUTEX_UNLOCK(rnd_mutex);
	}
}
MyVectorInt *my_random_sampleNoRepetitionsSorted(int64_t minValueIncluded,
		int64_t maxValueNotIncluded, double sampleSizeOrFraction) {
	int64_t max_size = maxValueNotIncluded - minValueIncluded;
	int64_t sample_size;
	if (sampleSizeOrFraction >= 1)
		sample_size = my_math_round_int(sampleSizeOrFraction);
	else if (sampleSizeOrFraction > 0)
		sample_size = my_math_round_int(sampleSizeOrFraction * max_size);
	else
		sample_size = max_size;
	sample_size = MIN(max_size, MAX(0, sample_size));
	int64_t *positions = MY_MALLOC(sample_size, int64_t);
	if (sample_size == max_size) {
		for (int64_t i = 0; i < max_size; ++i)
			positions[i] = minValueIncluded + i;
	} else {
		my_random_intList_noRepetitions(minValueIncluded, maxValueNotIncluded,
				positions, sample_size);
		my_qsort_int_array(positions, sample_size);
	}
	return my_vectorInt_new_wrapper(positions, sample_size, true);
}
static void validate_stats(double actualAvg, double expectedAvg) {
	double pctErrorAvg = 0.001;
	double expectedAvgDelta = expectedAvg * pctErrorAvg;
	//double errorAvg = MY_ABS_DOUBLE(expectedAvg - avg) / expectedAvg;
	my_log_info("expectedAverage=%lf +/- %lf\n", expectedAvg, expectedAvgDelta);
	if (MY_ABS_DOUBLE(expectedAvg - actualAvg) <= expectedAvgDelta)
		my_log_info("OK. average %lf inside range [%lf, %lf]\n", actualAvg,
				expectedAvg - expectedAvgDelta, expectedAvg + expectedAvgDelta);
	else
		my_log_info("ERROR. average %lf out of range [%lf, %lf]\n", actualAvg,
				expectedAvg - expectedAvgDelta, expectedAvg + expectedAvgDelta);
}
static void validate_frequencies(int64_t *array, int64_t minValueIncluded,
		int64_t maxValueNotIncluded, int64_t numSamples) {
	int64_t interval_size = maxValueNotIncluded - minValueIncluded;
	int64_t *frequencies = MY_MALLOC(interval_size, int64_t);
	for (int64_t i = 0; i < numSamples; ++i) {
		frequencies[array[i] - minValueIncluded]++;
	}
	int64_t expectedFreq = my_math_round_int(
			numSamples / (double) interval_size);
	int64_t expectedFreqDelta = my_math_round_int(expectedFreq * 0.01);
	my_log_info("expected frequency=%"PRIi64" +/- %"PRIi64"\n", expectedFreq,
			expectedFreqDelta);
	int64_t cont_errors = 0;
	for (int64_t i = 0; i < interval_size; ++i) {
		int64_t freq = frequencies[i];
		bool is_ok = (MY_ABS_INT(expectedFreq - freq) <= expectedFreqDelta);
		if (interval_size <= 100) {
			double freq_pct = 100.0 * freq / (double) numSamples;
			my_log_info("%3"PRIi64" =%7"PRIi64" (%1.2lf%%)%s\n",
					i + minValueIncluded, freq, freq_pct, is_ok ? "" : " *");
		}
		if (!is_ok)
			cont_errors++;
	}
	if (cont_errors == 0)
		my_log_info("OK. All frequencies inside range [%"PRIi64",%"PRIi64"]\n",
				expectedFreq - expectedFreqDelta,
				expectedFreq + expectedFreqDelta);
	else
		my_log_info(
				"ERROR. %"PRIi64"/%"PRIi64" frequencies out of range [%"PRIi64",%"PRIi64"]\n",
				cont_errors, interval_size, expectedFreq - expectedFreqDelta,
				expectedFreq + expectedFreqDelta);
	MY_FREE(frequencies);
}
void my_random_testInt(int64_t minValueIncluded, int64_t maxValueNotIncluded,
		int64_t numSamples) {
	my_log_info("\nrandomTestInt\n");
	int64_t *array = MY_MALLOC(numSamples, int64_t);
	my_random_intList(minValueIncluded, maxValueNotIncluded, array, numSamples);
	double actualAvg = my_math_averageIntArray(numSamples, array);
	double expectedAvg = (maxValueNotIncluded - 1 + minValueIncluded) / 2.0;
	char *st = my_newString_double(actualAvg);
	my_log_info("%"PRIi64" random int numbers in [%"PRIi64",%"PRIi64"): %s\n",
			numSamples, minValueIncluded, maxValueNotIncluded, st);
	MY_FREE(st);
	validate_stats(actualAvg, expectedAvg);
	validate_frequencies(array, minValueIncluded, maxValueNotIncluded,
			numSamples);
	MY_FREE(array);
}
void my_random_testDouble(double minValueIncluded, double maxValueNotIncluded,
		int64_t numSamples) {
	my_log_info("\nrandomTestDouble\n");
	double *array = MY_MALLOC_NOINIT(numSamples, double);
	my_random_doubleList(minValueIncluded, maxValueNotIncluded, array,
			numSamples);
	double actualAvg = my_math_averageDoubleArray(numSamples, array);
	char *st = my_newString_double(actualAvg);
	my_log_info("%"PRIi64" random double numbers in [%lf,%lf): %s\n",
			numSamples, minValueIncluded, maxValueNotIncluded, st);
	validate_stats(actualAvg, (maxValueNotIncluded + minValueIncluded) / 2.0);
	MY_FREE_MULTI(st, array);
}

