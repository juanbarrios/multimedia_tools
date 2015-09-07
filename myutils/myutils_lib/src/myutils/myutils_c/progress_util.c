/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "progress_util.h"

struct MyProgress {
	pthread_t logger_id;
	pthread_mutex_t logger_mutex;
	pthread_cond_t logger_cond;
	char *logger_name;
	int64_t contCompleted, contMaximum;
	bool isUnknownMaximum, isInLongWait;
	int64_t currentActiveThreads, maxActiveThreads;
	MyTimer *startTimer;
};
static char *getProgressRateString(int64_t numObjCompleted,
		double elapsedSeconds) {
	double rate = 0;
	if (elapsedSeconds > 0)
		rate = numObjCompleted / elapsedSeconds;
	if (rate > 10000)
		return my_newString_format("%1.0lf objs/sec", rate);
	else if (rate > 1000)
		return my_newString_format("%1.1lf objs/sec", rate);
	else if (rate > 1)
		return my_newString_format("%1.2lf objs/sec", rate);
	else if (rate > 0)
		return my_newString_format("%1.1lf secs/obj", 1 / rate);
	return my_newString_format("%1.1lf secs", elapsedSeconds);
}
static char *getActiveThreadsSuffix(int64_t contThreads) {
	return my_newString_format(", %"PRIi64" active thread%s", contThreads,
			(contThreads == 1) ? "" : "s");
}
static double getSecondsLeft(double elapsedSeconds, int64_t numObjCompleted,
		int64_t numObjLeft) {
	if (numObjCompleted == 0)
		return 0;
	return (elapsedSeconds * numObjLeft) / ((double) numObjCompleted);
}
static char *getProgressString(MyProgress *lt, bool isFinished) {
	double elapsedSeconds = my_timer_getSeconds(lt->startTimer);
	if (elapsedSeconds < 5)
		return NULL;
	char *stRate = getProgressRateString(lt->contCompleted, elapsedSeconds);
	char *progress;
	if (isFinished) {
		char *totalTime = my_newString_hhmmss(elapsedSeconds);
		char *stThreads = getActiveThreadsSuffix(lt->maxActiveThreads);
		progress = my_newString_format("%"PRIi64" in %1.1lf seconds %s (%s%s)",
				lt->contCompleted, elapsedSeconds, totalTime, stRate,
				stThreads);
		MY_FREE_MULTI(totalTime, stThreads);
	} else if (lt->isUnknownMaximum) {
		char *stThreads = getActiveThreadsSuffix(lt->currentActiveThreads);
		progress = my_newString_format("%"PRIi64" (%s%s)", lt->contCompleted,
				stRate, stThreads);
		MY_FREE(stThreads);
	} else {
		double secLeft = getSecondsLeft(elapsedSeconds, lt->contCompleted,
				lt->contMaximum - lt->contCompleted);
		char *timeLeft = my_newString_hhmmss(secLeft);
		char *stThreads = getActiveThreadsSuffix(lt->currentActiveThreads);
		progress = my_newString_format(
				"%"PRIi64"/%"PRIi64" (%2.0lf%%) left %s (%s%s)",
				lt->contCompleted, lt->contMaximum,
				(100.0 * lt->contCompleted) / lt->contMaximum, timeLeft, stRate,
				stThreads);
		MY_FREE_MULTI(timeLeft, stThreads);
	}
	free(stRate);
	return progress;
}

#define SECONDS_SHORT_WAIT  30
#define SECONDS_LONG_WAIT  180

static void* logger_thread(void *params_threads) {
	MyProgress *lt = (MyProgress*) params_threads;
	MY_MUTEX_LOCK(lt->logger_mutex);
	for (;;) {
		if (lt->contCompleted >= lt->contMaximum)
			break;
		int64_t prev_completed = lt->contCompleted;
		int64_t prev_maximum = lt->contMaximum;
		MY_MUTEX_TIMEDWAIT(lt->logger_cond, lt->logger_mutex,
				SECONDS_SHORT_WAIT);
		if (lt->contCompleted >= lt->contMaximum) {
			break;
		} else if (prev_completed == lt->contCompleted
				&& prev_maximum == lt->contMaximum) {
			//nothing has changed
			lt->isInLongWait = true;
			MY_MUTEX_TIMEDWAIT(lt->logger_cond, lt->logger_mutex,
					SECONDS_LONG_WAIT);
			lt->isInLongWait = false;
		}
		if (lt->contCompleted >= lt->contMaximum)
			break;
		char *progress = getProgressString(lt, false);
		if (progress != NULL) {
			my_log_info_time("%s: %s\n", lt->logger_name, progress);
			free(progress);
		}
	}
	MY_MUTEX_UNLOCK(lt->logger_mutex);
	return NULL;
}
//contMaximum <= 0 for unknown length
MyProgress* my_progress_new(const char *logger_name, int64_t contMaximum,
		int64_t startingThreads) {
	MyProgress *lt = MY_MALLOC(1, MyProgress);
	lt->logger_name = my_newString_string(logger_name);
	if (contMaximum > 0) {
		lt->contMaximum = contMaximum;
	} else {
		lt->contMaximum = INT64_MAX;
		lt->isUnknownMaximum = true;
	}
	lt->currentActiveThreads = lt->maxActiveThreads = startingThreads;
	lt->startTimer = my_timer_new();
	MY_MUTEX_INIT(lt->logger_mutex);
	int ret = pthread_cond_init(&lt->logger_cond, NULL);
	my_assert_equalInt("pthread_cond_init", ret, 0);
	ret = pthread_create(&lt->logger_id, NULL, logger_thread, lt);
	my_assert_equalInt("pthread_create", ret, 0);
	return lt;
}
static void updated(MyProgress *lt, bool forced) {
	if (lt->contCompleted > lt->contMaximum)
		lt->contMaximum = lt->contCompleted;
	if (lt->isInLongWait || forced) {
		int est = pthread_cond_broadcast(&lt->logger_cond);
		my_assert_equalInt("pthread_cond_signal", est, 0);
	}
}
//thread-safe
void my_progress_addN(MyProgress *lt, int64_t n) {
	if (lt == NULL || n == 0)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	lt->contCompleted += n;
	updated(lt, false);
	MY_MUTEX_UNLOCK(lt->logger_mutex);
}
//thread-safe
void my_progress_add1(MyProgress *lt) {
	if (lt == NULL)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	lt->contCompleted += 1;
	updated(lt, false);
	MY_MUTEX_UNLOCK(lt->logger_mutex);
}
//thread-safe
void my_progress_setN(MyProgress *lt, int64_t n) {
	if (lt == NULL)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	if (n != lt->contCompleted) {
		lt->contCompleted = n;
		updated(lt, false);
	}
	MY_MUTEX_UNLOCK(lt->logger_mutex);
}
//thread-safe
void my_progress_substractFromMaximum(MyProgress *lt, int64_t n) {
	if (lt == NULL || n == 0)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	lt->contMaximum -= n;
	updated(lt, false);
	MY_MUTEX_UNLOCK(lt->logger_mutex);
}
//thread-safe
void my_progress_updateActiveThreads(MyProgress *lt, int64_t activeThreads) {
	if (lt == NULL)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	lt->currentActiveThreads = activeThreads;
	if (lt->currentActiveThreads > lt->maxActiveThreads)
		lt->maxActiveThreads = lt->currentActiveThreads;
	updated(lt, false);
	MY_MUTEX_UNLOCK(lt->logger_mutex);
}
void my_progress_release(MyProgress *lt) {
	if (lt == NULL)
		return;
	MY_MUTEX_LOCK(lt->logger_mutex);
	lt->contMaximum = lt->contCompleted;
	updated(lt, true);
	MY_MUTEX_UNLOCK(lt->logger_mutex);
	int ret = pthread_join(lt->logger_id, NULL);
	my_assert_equalInt("pthread_join", ret, 0);
	char *progress = getProgressString(lt, true);
	if (progress != NULL) {
		my_log_info_time("%s: %s\n", lt->logger_name, progress);
		free(progress);
	}
	ret = pthread_cond_destroy(&lt->logger_cond);
	my_assert_equalInt("pthread_cond_destroy", ret, 0);
	my_timer_release(lt->startTimer);
	MY_MUTEX_DESTROY(lt->logger_mutex);
	MY_FREE(lt->logger_name);
	free(lt);
}
