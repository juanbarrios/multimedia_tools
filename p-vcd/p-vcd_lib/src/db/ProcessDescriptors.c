/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "../pvcd.h"

void db_process_db(CmdParams *cmd_params, char *argOption) {
#ifndef NO_OPENCV
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"    -db path_DB                       Mandatory. One or More. Options: %s\n",
				getDbLoadOptions());
		my_log_info(
				"    [-seg segmentation]               Optional or Mandatory (depends on -proc). Stored segmentation alias to run process.\n");
		my_log_info(
				"    [-desc descriptor]                Optional or Mandatory (depends on -proc). Stored descriptor alias to run process.\n");
		my_log_info(
				"    -proc CODE                        Mandatory. The process to run on db, segmentation or descriptor.\n");
		my_log_info(
				"    -show                            Shows defined processors\n");
		return;
	}
	DB *db = NULL;
	LoadSegmentation *segloader = NULL;
	LoadDescriptors *desloader = NULL;
	Processor *proc = NULL;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-db")) {
			const char *name = nextParam(cmd_params);
			if (db != NULL)
				my_log_error("-db is duplicated\n");
			db = loadDB(name, true, true);
		} else if (isNextParam(cmd_params, "-seg")) {
			const char *nameseg = nextParam(cmd_params);
			if (db == NULL)
				my_log_error("-db is not defined\n");
			else if (segloader != NULL)
				my_log_error("-seg is duplicated\n");
			segloader = newLoadSegmentation(db, nameseg);
		} else if (isNextParam(cmd_params, "-desc")) {
			const char *namedesc = nextParam(cmd_params);
			if (db == NULL)
				my_log_error("-db is not defined\n");
			else if (desloader != NULL)
				my_log_error("-desc is duplicated\n");
			desloader = newLoadDescriptors(db, namedesc);
		} else if (isNextParam(cmd_params, "-proc")) {
			const char *pr = nextParam(cmd_params);
			if (proc != NULL)
				my_log_error("-proc is duplicated\n");
			proc = getProcessor(pr);
		} else if (isNextParam(cmd_params, "-show")) {
			print_processors();
			return;
		} else
			my_log_error("unknown parameter %s\n", nextParam(cmd_params));
	}
	my_assert_notNull("-db", db);
	my_assert_notNull("-proc", proc);
	set_logfile(db->pathLogs, cmd_params, "");
	if (desloader != NULL) {
		if (!runProcessDescriptor(proc, desloader))
			my_log_error("process is not defined for descriptors\n");
	} else if (segloader != NULL) {
		if (!runProcessSegmentation(proc, segloader))
			my_log_error("process is not defined for segmentations\n");
	} else {
		if (!runProcessDb(proc, db))
			my_log_error("process is not defined for database\n");
	}
	close_logfile();
#endif
}
