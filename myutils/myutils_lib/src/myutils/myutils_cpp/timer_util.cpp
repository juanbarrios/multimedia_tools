/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "timer_util.hpp"
#include "../myutils_c.h"

using namespace my;

class timer::Impl {
public:
	MyTimer *t;
};

timer::timer() {
	this->pimpl = new timer::timer::Impl();
	this->pimpl->t = my_timer_new();
}
timer::~timer() {
	my_timer_release(this->pimpl->t);
	delete this->pimpl;
}
double timer::getSeconds() {
	return my_timer_getSeconds(this->pimpl->t);
}
double timer::updateToNow() {
	return my_timer_updateToNow(this->pimpl->t);
}
std::string timer::currentClock() {
	char *st = my_timer_currentClock_newString();
	std::string s(st);
	free(st);
	return s;
}

