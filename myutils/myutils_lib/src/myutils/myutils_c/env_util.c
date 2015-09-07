/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "env_util.h"

MY_MUTEX_NEWSTATIC(priv_mutex_getenv);
static MyMapStringObj *mapEnv = NULL;

const char *my_env_getString(const char *envVarName) {
	MY_MUTEX_LOCK(priv_mutex_getenv);
	if (mapEnv == NULL)
		mapEnv = my_mapStringObj_newIgnoreCase();
	const char *value = my_mapStringObj_get(mapEnv, envVarName);
	if (value == NULL) {
		const char *env_val = getenv(envVarName);
		if (env_val == NULL)
			env_val = "";
		my_mapStringObj_add(mapEnv, my_newString_string(envVarName),
				my_newString_string(env_val));
		value = my_mapStringObj_get(mapEnv, envVarName);
	}
	MY_MUTEX_UNLOCK(priv_mutex_getenv);
	return value;
}
int64_t my_env_getInt(const char *envVarName, int64_t defaultVal) {
	const char *value = my_env_getString(envVarName);
	if (value == NULL || strlen(value) == 0)
		return defaultVal;
	int64_t n = my_parse_int(value);
	return n;
}

const char *my_env_getString_def2(const char *envEnvName,
		const char *defaultVal_win, const char *defaultVal_noWin) {
	const char *value = my_env_getString(envEnvName);
	if (value == NULL || strlen(value) == 0)
		return IS_WINDOWS ? defaultVal_win : defaultVal_noWin;
	return value;
}
