/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

void db_create_db(CmdParams *cmd_params, const char *argOption);
void db_compute_segmentation(CmdParams *cmd_params, const char *argOption);
void db_compute_descriptors(CmdParams *cmd_params, const char *argOption);
void db_process_db(CmdParams *cmd_params, const char *argOption);

int pvcd_processDB(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s -new ...\n", getBinaryName(cmd_params));
		my_log_info("%s -segment ...\n", getBinaryName(cmd_params));
		my_log_info("%s -extract ...\n", getBinaryName(cmd_params));
		my_log_info("%s -process ...\n", getBinaryName(cmd_params));
		return pvcd_system_exit_error();
	}
	const char *option = nextParam(cmd_params);
	if (my_string_equals(option, "-new")) {
		db_create_db(cmd_params, option);
	} else if (my_string_equals(option, "-segment")) {
		db_compute_segmentation(cmd_params, option);
	} else if (my_string_equals(option, "-extract")) {
		db_compute_descriptors(cmd_params, option);
	} else if (my_string_equals(option, "-process")) {
		db_process_db(cmd_params, option);
	} else {
		my_log_error("unknown parameter %s\n", option);
	}
	return pvcd_system_exit_ok(cmd_params);
}
