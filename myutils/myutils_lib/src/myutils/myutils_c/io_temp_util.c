/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "io_util.h"

MY_MUTEX_NEWSTATIC(filetemp_mutex);

static char *temp_dirname = NULL;
static char *temp_filename_prefix = NULL;

void my_io_temp_setTempDir(const char* dirname_temp) {
	MY_MUTEX_LOCK(filetemp_mutex);
	if (dirname_temp == NULL || strlen(dirname_temp) == 0) {
		MY_FREE(temp_dirname);
		temp_dirname = NULL;
	} else if (!my_io_existsDir(dirname_temp)) {
		my_log_error("dir '%s' not found\n", dirname_temp);
	} else if (!my_string_endsWith(dirname_temp, "/")) {
		temp_dirname = my_newString_format("%s/", dirname_temp);
	} else {
		temp_dirname = my_newString_string(dirname_temp);
	}
	MY_MUTEX_UNLOCK(filetemp_mutex);
}

void my_io_temp_setFilenamePrefix(const char* filename_prefix) {
	MY_MUTEX_LOCK(filetemp_mutex);
	if (filename_prefix == NULL || strlen(filename_prefix) == 0) {
		MY_FREE(temp_filename_prefix);
		temp_filename_prefix = NULL;
	} else {
		temp_filename_prefix = my_newString_string(filename_prefix);
	}
	MY_MUTEX_UNLOCK(filetemp_mutex);
}

char* my_io_temp_createNewFile(const char* suffix) {
	MY_MUTEX_LOCK(filetemp_mutex);
	const char *dirname = temp_dirname;
	if (dirname == NULL)
		dirname = "";
	const char *name_prefix = temp_filename_prefix;
	if (name_prefix == NULL)
		name_prefix = "temp_";
	if (suffix == NULL)
		suffix = "";
	char *fname = NULL;
	int64_t cont = 0;
	for (;;) {
		char *ff = my_newString_format("%s%s%03"PRIi64"%s", dirname,
				name_prefix, cont, suffix);
		if (my_io_notExists(ff)) {
			fname = ff;
			break;
		}
		free(ff);
		cont++;
		if (cont >= 10000)
			my_log_error(
					"could not create a new temp file in '%s' (no available names)\n",
					dirname);
	}
	if (fname != NULL) {
		FILE* out = fopen(fname, "w");
		if (out == NULL)
			my_log_error("error creating new temp file %s\n", fname);
		fclose(out);
		//char *abs_name = my_io_getAbsolutPath(fname);
		//free(fname);
		//fname = abs_name;
	}
	MY_MUTEX_UNLOCK(filetemp_mutex);
	return fname;
}
