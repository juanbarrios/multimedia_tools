/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_TIMER_UTIL_H
#define MY_TIMER_UTIL_H

#include "../myutils_c.h"

MyTimer *my_timer_new();

double my_timer_getSeconds(MyTimer *timer);

double my_timer_updateToNow(MyTimer *timer);

void my_timer_release(MyTimer *timer);

char *my_timer_currentClock_newString();

#endif
