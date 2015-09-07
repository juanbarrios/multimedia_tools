/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_TIMER_UTIL_HPP
#define MY_TIMER_UTIL_HPP

#include "../myutils_cpp.hpp"

namespace my {

class timer {
public:

	double getSeconds();
	double updateToNow();

	timer();
	~timer();

	static std::string currentClock();

private:
	class Impl;
	Impl *pimpl;
	timer(const timer&);
	timer& operator=(const timer&);

};

}

#endif
