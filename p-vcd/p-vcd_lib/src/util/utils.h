/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef UTILS_H
#define UTILS_H

#include "../pvcd.h"

void pvcd_restore_locale();

void set_logfile(const char *path, CmdParams *cmd_params, const char *suffix);
void close_logfile();

CmdParams *pvcd_system_start(int argc, char **argv);
int pvcd_system_exit_ok(CmdParams *cmd_params);
int pvcd_system_exit_error();
void pvcd_register_default_values();

void pvcd_system_setAfterInitFunction(void (*func_afterInit)());
void pvcd_system_setBeforeExitFunction(void (*func_beforeExit)());

int64_t nextParamInt(CmdParams *cmd_params);
double nextParamDouble(CmdParams *cmd_params);
double nextParamFraction(CmdParams *cmd_params);
double nextParamTime(CmdParams *cmd_params);
const char* nextParam(CmdParams *cmd_params);
bool isNextParam(CmdParams *cmd_params, char *value);
bool hasNextParam(CmdParams *cmd_params);
const char *getBinaryName(CmdParams *cmd_params);
void removeLastParam(CmdParams *cmd_params);
void resetParams(CmdParams *cmd_params);

char *get_command_line(CmdParams *cmd_params);

#endif
