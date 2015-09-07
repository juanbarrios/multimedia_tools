/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "systemCall.h"

void pvcd_register_system_impl(int (*function_system)(const char *command_line)) {
	my_io_system_setSystemImpl(function_system);
	my_image_setSystemImpl(function_system);
}

static int internal_system_call(const char *binaryName, const char * parameters) {
	char *command_line = my_newString_format("%s %s", binaryName, parameters);
	int exitCode = my_io_system(command_line);
	free(command_line);
	//locale may reset to default
	pvcd_restore_locale();
	return exitCode;
}
static const char *internal_getBinaryName(const char *binaryEnvName,
		const char *binaryName_win, const char *binaryName_noWin) {
	const char *filename = my_env_getString(binaryEnvName);
	if (strlen(filename) == 0) {
		return IS_WINDOWS ? binaryName_win : binaryName_noWin;
	} else {
		my_assert_fileExists(filename);
		return filename;
	}
}
void pvcd_system_call(const char *binaryEnvName, const char *binaryName_win,
		const char *binaryName_noWin, const char *parameters) {
	const char *binaryName = internal_getBinaryName(binaryEnvName,
			binaryName_win, binaryName_noWin);
	internal_system_call(binaryName, parameters);
}
/****************************/
static const char *internal_getBinaryFFmpeg() {
	return internal_getBinaryName("PVCD_FFMPEG_BIN", "ffmpeg.exe", "ffmpeg");
}
static const char *internal_getBinaryFFprobe() {
	return internal_getBinaryName("PVCD_FFPROBE_BIN", "ffprobe.exe", "ffprobe");
}
static const char *internal_getBinaryVlc() {
	return internal_getBinaryName("PVCD_VLC_BIN", "vlc.exe", "vlc");
}

void pvcd_system_call_ffmpeg(const char *parameters) {
	const char *binaryName = internal_getBinaryFFmpeg();
	internal_system_call(binaryName, parameters);
}
void pvcd_system_call_ffprobe(const char *parameters) {
	const char *binaryName = internal_getBinaryFFprobe();
	internal_system_call(binaryName, parameters);
}
void pvcd_system_call_vlc(const char *parameters) {
	const char *binaryName = internal_getBinaryVlc();
	internal_system_call(binaryName, parameters);
}

char *getCommand_vlc_viewSegment(const char *filenameInput,
		const char *filenameExt, double seconds_start, double seconds_length) {
	const char *binaryName = internal_getBinaryVlc();
	char *s1 = my_newString_doubleDec(seconds_start, 3);
	char *s2 = my_newString_doubleDec(seconds_length, 3);
	char *ff = my_newString_format(
			"%s %s%s --start-time=%s --run-time=%s --play-and-pause",
			binaryName, filenameInput, (filenameExt == NULL) ? "" : filenameExt,
			s1, s2);
	MY_FREE_MULTI(s1, s2);
	return ff;
}
char *getCommand_ffmpeg_copySegment(const char *filenameInput,
		const char *filenameExt, double seconds_start, double seconds_length,
		const char *filenameOutput) {
	const char *binaryName = internal_getBinaryFFmpeg();
	char *s1 = my_newString_hhmmssfff(seconds_start);
	char *s2 = my_newString_hhmmssfff(seconds_length);
	char *ff = my_newString_format(
			"%s -ss %s -i %s%s -t %s -codec copy -map_metadata -1 %s%s",
			binaryName, s1, filenameInput,
			(filenameExt == NULL) ? "" : filenameExt, s2, filenameOutput,
			(filenameExt == NULL) ? "" : filenameExt);
	MY_FREE_MULTI(s1, s2);
	return ff;
}

/*
 Something to try:
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/wait.h>

 int __system(const char * cmd) {
 pid_t pid;
 int status;

 if ((pid = fork()) == 0) {
 execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
 return -1;
 } else if (pid > 0) {
 waitpid(pid, &status, 0);
 return WEXITSTATUS(status);
 } else
 return -1;

 }
 */
