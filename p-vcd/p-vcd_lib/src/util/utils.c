/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "utils.h"
#include <locale.h>
#include <time.h>

int64_t NUM_CORES = 1;
bool WITH_LOG_TO_FILE = true;

MY_MUTEX_NEWSTATIC(priv_mutex_locale);

void pvcd_restore_locale() {
	MY_MUTEX_LOCK(priv_mutex_locale);
	setlocale(LC_ALL, "POSIX");
	MY_MUTEX_UNLOCK(priv_mutex_locale);
}
static char *get_logfile_name(CmdParams *cmd_params, const char *suffix) {
	char *binName;
	if (my_string_endsWith_ignorecase(getBinaryName(cmd_params), ".exe")) {
		binName = my_subStringC_startLast(getBinaryName(cmd_params), '.');
	} else {
		binName = my_newString_string(getBinaryName(cmd_params));
	}
	resetParams(cmd_params);
	if (hasNextParam(cmd_params)) {
		const char *st = nextParam(cmd_params);
		char *newName = my_newString_format("%s,%s", binName,
				(my_string_startsWith(st, "-") ? st + 1 : st));
		MY_FREE(binName);
		binName = newName;
	}
	if (suffix != NULL) {
		char *newName = my_newString_format("%s%s", binName, suffix);
		MY_FREE(binName);
		binName = newName;
	}
	my_io_removeInvalidChars(binName);
	return binName;
}
void set_logfile(const char *path, CmdParams *cmd_params, const char *suffix) {
	close_logfile();
	if (!WITH_LOG_TO_FILE)
		return;
	char *logFilename = get_logfile_name(cmd_params, suffix);
	my_assert_notNull("path", path);
	my_io_createDir(path, 0);
	FILE *out = NULL;
	int64_t i;
	for (i = 0; i < 100 && out == NULL; ++i) {
		char *file = my_newString_format("%s/%s.%02"PRIi64".log", path,
				logFilename, i);
		if (!my_io_existsFile(file))
			out = my_io_openFileWrite1(file);
		MY_FREE(file);
	}
	if (out == NULL)
		my_log_error("cannot create log file %s in %s\n", logFilename, path);
	char *stCmd = get_command_line(cmd_params);
	fprintf(out, "%s\n", stCmd);
	MY_FREE_MULTI(logFilename, stCmd);
	my_log_setOutputFile(out);
}
void close_logfile() {
	my_log_closeOutputFile();
}
static void pvcd_print_legal() {
	printf("This file is part of P-VCD. http://p-vcd.org/\n");
 	printf(
			"P-VCD is made available under the terms of the BSD 2-Clause License.\n");
	printf(
			"This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law.\n");
#ifdef VERSION_NAME
#define STR(x) #x
#define STRX(x) STR(x)
	printf("Compiled version: " STRX(VERSION_NAME) "\n");
#endif
	printf("\n");
}

static void pvcd_system_exit_logerror() {
	pvcd_system_exit_error();
}
void pvcd_register_default_values() {
	my_log_setErrorFunction(pvcd_system_exit_logerror);
	NUM_CORES = my_env_getInt("PVCD_NUM_CORES", 0);
	if (NUM_CORES == 0)
		NUM_CORES = my_parallel_getNumberOfCores();
	my_io_temp_setTempDir(my_env_getString("PVCD_TEMP_DIR"));
	my_io_temp_setFilenamePrefix(my_env_getString("PVCD_TEMP_PREFIX"));
	pvcd_register_default_fileExtensions();
	pvcd_register_default_extractors();
	pvcd_register_default_processors();
	pvcd_register_default_segmentators();
	pvcd_register_default_transformations();
}
static void priv_load_cmd_parameters(CmdParams *cmd_params) {
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-num_cores")) {
			removeLastParam(cmd_params);
			NUM_CORES = nextParamInt(cmd_params);
			removeLastParam(cmd_params);
		} else if (isNextParam(cmd_params, "-random_seed")) {
			removeLastParam(cmd_params);
			int64_t RANDOM_SEED = nextParamInt(cmd_params);
			removeLastParam(cmd_params);
			my_random_setSeed(RANDOM_SEED);
		} else if (isNextParam(cmd_params, "-noLogFile")) {
			removeLastParam(cmd_params);
			WITH_LOG_TO_FILE = false;
		} else if (isNextParam(cmd_params, "-noLogStdout")) {
			removeLastParam(cmd_params);
			my_log_noOutputStdout();
		} else if (isNextParam(cmd_params, "-setAudioFileExtensions")) {
			removeLastParam(cmd_params);
			pvcd_register_audio_fileExtensions(nextParam(cmd_params));
			removeLastParam(cmd_params);
		} else if (isNextParam(cmd_params, "-setImageFileExtensions")) {
			removeLastParam(cmd_params);
			pvcd_register_image_fileExtensions(nextParam(cmd_params));
			removeLastParam(cmd_params);
		} else if (isNextParam(cmd_params, "-setVideoFileExtensions")) {
			removeLastParam(cmd_params);
			pvcd_register_video_fileExtensions(nextParam(cmd_params));
			removeLastParam(cmd_params);
		} else if (isNextParam(cmd_params, "-version")) {
			removeLastParam(cmd_params);
			pvcd_print_legal();
			pvcd_system_exit_ok(cmd_params);
		} else {
			nextParam(cmd_params);
		}
	}
	resetParams(cmd_params);
}

struct CmdParams {
	int64_t argc;
	char **argv;
	char *nombreBin;
	int64_t currentPos;
	MyVectorString *list;
	MyTimer *timerInit;
};

static void default_empty_function() {
}
static void (*func_external_afterInit)() = default_empty_function;
static void (*func_external_beforeExit)() = default_empty_function;

CmdParams *pvcd_system_start(int argc, char **argv) {
	CmdParams *cmd_params = MY_MALLOC(1, CmdParams);
	cmd_params->argc = argc;
	cmd_params->argv = argv;
	cmd_params->nombreBin = (argc > 0) ? my_io_getFilename(argv[0]) : NULL;
	cmd_params->currentPos = 0;
	cmd_params->list = my_vectorString_new();
	int64_t i, k;
	for (i = 1; i < argc; ++i) {
		char *s = my_newString_string(argv[i]);
		my_string_replaceChar(s, '\\', '/');
		for (k = strlen(s) - 1; k >= 0; --k) {
			char c = s[k];
			if (c == '\r' || c == '\n' || c < ' ' || (k > 1 && c == '/'))
				s[k] = '\0';
			else
				break;
		}
		my_vectorString_add(cmd_params->list, s);
	}
	cmd_params->timerInit = my_timer_new();
	pvcd_register_default_values();
	priv_load_cmd_parameters(cmd_params);
	pvcd_restore_locale();
	func_external_afterInit();
	return cmd_params;
}
int pvcd_system_exit_ok(CmdParams *cmd_params) {
	double ss = my_timer_getSeconds(cmd_params->timerInit);
	int64_t seconds = (int64_t) ss;
	int64_t hours = seconds / 3600;
	seconds = seconds % 3600;
	int64_t minutes = seconds / 60;
	seconds = seconds % 60;
	if (hours > 0) {
		my_log_info_time(
				"total time: %"PRIi64" hrs. %"PRIi64" mins. %"PRIi64" secs.\n",
				hours, minutes, seconds);
	} else if (minutes > 0) {
		my_log_info_time("total time: %"PRIi64" mins. %"PRIi64" secs.\n",
				minutes, seconds);
	} else if (seconds >= 10) {
		my_log_info_time("total time: %1.1lf secs.\n", ss);
	}
	my_timer_release(cmd_params->timerInit);
	MY_FREE(cmd_params->nombreBin);
	my_vectorString_release(cmd_params->list, true);
	MY_FREE(cmd_params);
	close_logfile();
	func_external_beforeExit();
	exit(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

MY_MUTEX_NEWSTATIC(priv_error_exit_mutex);

int pvcd_system_exit_error() {
	MY_MUTEX_LOCK(priv_error_exit_mutex);
	close_logfile();
	func_external_beforeExit();
	exit(EXIT_FAILURE);
	MY_MUTEX_UNLOCK(priv_error_exit_mutex);
	return EXIT_FAILURE;
}

void pvcd_system_setAfterInitFunction(void (*func_afterInit)()) {
	func_external_afterInit = func_afterInit;
}
void pvcd_system_setBeforeExitFunction(void (*func_beforeExit)()) {
	func_external_beforeExit = func_beforeExit;
}

int64_t nextParamInt(CmdParams *cmd_params) {
	return my_parse_int(nextParam(cmd_params));
}
double nextParamDouble(CmdParams *cmd_params) {
	return my_parse_double(nextParam(cmd_params));
}
double nextParamFraction(CmdParams *cmd_params) {
	return my_parse_fraction(nextParam(cmd_params));
}
double nextParamTime(CmdParams *cmd_params) {
	return my_parse_seconds(nextParam(cmd_params));
}
const char* nextParam(CmdParams *cmd_params) {
	if (!hasNextParam(cmd_params))
		my_log_error("error reading parameters\n");
	return my_vectorString_get(cmd_params->list, cmd_params->currentPos++);
}
bool isNextParam(CmdParams *cmd_params, char *value) {
	if (!hasNextParam(cmd_params))
		return 0;
	int64_t n = cmd_params->currentPos;
	const char *param = nextParam(cmd_params);
	if (my_string_equals(param, value))
		return true;
	cmd_params->currentPos = n;
	return false;
}
bool hasNextParam(CmdParams *cmd_params) {
	return (cmd_params->currentPos < my_vectorString_size(cmd_params->list));
}
const char *getBinaryName(CmdParams *cmd_params) {
	return cmd_params->nombreBin;
}
void removeLastParam(CmdParams *cmd_params) {
	if (cmd_params->currentPos > 0) {
		cmd_params->currentPos--;
		my_vectorString_remove(cmd_params->list, cmd_params->currentPos);
	}
}
void resetParams(CmdParams *cmd_params) {
	cmd_params->currentPos = 0;
}

char *get_command_line(CmdParams *cmd_params) {
	int64_t i;
	char **argv = cmd_params->argv;
	MyStringBuffer *sb = my_stringbuf_new();
	for (i = 0; i < cmd_params->argc; ++i) {
		if (i > 0)
			my_stringbuf_appendChar(sb, ' ');
		if (my_string_indexOf(argv[i], " ") >= 0) {
			my_stringbuf_appendString(sb, "\"");
			my_stringbuf_appendString(sb, argv[i]);
			my_stringbuf_appendString(sb, "\"");
		} else
			my_stringbuf_appendString(sb, argv[i]);
	}
	char *time_buff = my_timer_currentClock_newString();
	char *st = my_newString_format("[%s] %s", time_buff,
			my_stringbuf_getCurrentBuffer(sb));
	MY_FREE(time_buff);
	my_stringbuf_release(sb);
	return st;
}
