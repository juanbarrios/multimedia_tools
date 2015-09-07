/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_ENV_UTIL_H
#define MY_ENV_UTIL_H

#include "../myutils_c.h"

/**
 * Retrieves a environment variable. Each value is stored in cached in an internal map.
 * This function supports concurrent.
 *
 * @param envVarName the name of the variable
 * @return the environment variable, "" if envVarName is not set (it does not return null).
 */
const char *my_env_getString(const char *envVarName);

/**
 *
 * @param envVarName
 * @param defaultVal
 * @return the integer stored in the environment, or the default value if it is not defined.
 */
int64_t my_env_getInt(const char *envVarName, int64_t defaultVal);

/**
 *
 * @param envEnvName
 * @param defaultVal_win
 * @param defaultVal_noWin
 * @return the environment of a default value if it is not defined
 */
const char *my_env_getString_def2(const char *envEnvName,
		const char *defaultVal_win, const char *defaultVal_noWin);
#endif
