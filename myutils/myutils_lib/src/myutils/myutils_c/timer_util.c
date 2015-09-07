/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "timer_util.h"
#include <time.h>

#if IS_WINDOWS
#include <windows.h>
#endif

struct MyTimer {
	int64_t start_seconds;
	int64_t start_ticks;
	bool reportedError;
};

static int64_t getTickCount() {
#if IS_WINDOWS
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (int64_t) counter.QuadPart;
#elif IS_LINUX
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return ((int64_t)tp.tv_sec)*1000000000 + tp.tv_nsec;
#else
	struct timeval tv;
	struct timezone tz;
	gettimeofday( &tv, &tz );
	return ((int64_t)tv.tv_sec)*1000000 + tv.tv_usec;
#endif
}

static int64_t getTickFrequency() {
#if IS_WINDOWS
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (int64_t) freq.QuadPart;
#elif IS_LINUX
	return 1000000000;
#else
	return 1000000;
#endif
}

MyTimer *my_timer_new() {
	MyTimer *timer = MY_MALLOC(1, MyTimer);
	timer->start_seconds = time(NULL);
	timer->start_ticks = getTickCount();
	timer->reportedError = false;
	return timer;
}
static double priv_getTimerSeconds(MyTimer *timer, bool updateToNow) {
	int64_t current_seconds = time(NULL);
	int64_t current_ticks = getTickCount();
	double wallClock_time = (current_seconds - timer->start_seconds);
	double cpu_time = (current_ticks - timer->start_ticks)
			/ ((double) getTickFrequency());
	double diff = fabs(wallClock_time - cpu_time);
	if (diff > 2 && !timer->reportedError) {
		my_log_info(
				"warning: wall-clock time and cpu time do not coincide %lf vs %lf.\n",
				wallClock_time, cpu_time);
		timer->reportedError = true;
	}
	if (updateToNow) {
		timer->start_seconds = current_seconds;
		timer->start_ticks = current_ticks;
		timer->reportedError = false;
	}
	return (timer->reportedError) ? wallClock_time : cpu_time;
}
double my_timer_getSeconds(MyTimer *timer) {
	return priv_getTimerSeconds(timer, false);
}
double my_timer_updateToNow(MyTimer *timer) {
	return priv_getTimerSeconds(timer, true);
}
void my_timer_release(MyTimer *timer) {
	if (timer != NULL)
		free(timer);
}
char *my_timer_currentClock_newString() {
	time_t l = time(NULL);
	char time_buff[40];
	ctime_r(&l, time_buff);
	return my_subStringC_startLast(time_buff, '\n');
}


#if 0
struct MyDate {
	int64_t year, month, day, hour, minute;
	double second;
};
static struct MyDate parseTimestamp(const char *timestamp) {
	//format: year_month_day(T)hour_minute_second
	MyTokenizer *tk = my_tokenizer_new(timestamp, '_');
	my_tokenizer_addDelimiter(tk, 'T');
	struct MyDate d;
	d.year = my_tokenizer_nextInt(tk);
	d.month = my_tokenizer_nextInt(tk);
	d.day = my_tokenizer_nextInt(tk);
	d.hour = my_tokenizer_nextInt(tk);
	d.minute = my_tokenizer_nextInt(tk);
	d.second = my_tokenizer_nextDouble(tk);
	my_tokenizer_release(tk);
	return d;
}
static int64_t getLastDayOfMonth(int64_t month) {
	switch (month) {
	case 2:
		return 28;
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;
	}
	return 30;
}
static struct MyDate addSeconds(struct MyDate d, double seconds) {
	d.second += seconds;
	while (d.second > 59) {
		d.second -= 60;
		d.minute += 1;
	}
	while (d.minute > 59) {
		d.minute -= 60;
		d.hour += 1;
	}
	while (d.hour > 23) {
		d.hour -= 24;
		d.day += 1;
	}
	while (d.day > getLastDayOfMonth(d.month)) {
		d.day -= getLastDayOfMonth(d.month);
		d.month += 1;
	}
	while (d.month > 12) {
		d.month -= 12;
		d.year += 1;
	}
	while (d.second < 0) {
		d.second += 60;
		d.minute -= 1;
	}
	while (d.minute < 0) {
		d.minute += 60;
		d.hour -= 1;
	}
	while (d.hour < 0) {
		d.hour += 24;
		d.day -= 1;
	}
	return d;
}
static char *toStringDate(struct MyDate d) {
	return my_newString_format("%s%"PRIi64"-%s%"PRIi64"-%"PRIi64, (d.day < 10) ? "0" : "",
			d.day, (d.month < 10) ? "0" : "", d.month, d.year);
}
#endif
