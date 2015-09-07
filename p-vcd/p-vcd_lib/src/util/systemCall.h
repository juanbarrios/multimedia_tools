/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "../pvcd.h"

void pvcd_register_system_impl(int (*function_system)(const char *command_line));

void pvcd_system_call(const char *binaryEnvName, const char *binaryName_win,
		const char *binaryName_noWin, const char *parameters);

void pvcd_system_call_ffmpeg(const char *parameters);
void pvcd_system_call_ffprobe(const char *parameters);
void pvcd_system_call_vlc(const char *parameters);

char *getCommand_vlc_viewSegment(const char *filenameInput,
		const char *filenameExt, double seconds_start, double seconds_length);
char *getCommand_ffmpeg_copySegment(const char *filenameInput,
		const char *filenameExt, double seconds_start, double seconds_length,
		const char *filenameOutput);

#endif
