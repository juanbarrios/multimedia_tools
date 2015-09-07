/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

struct DetectionsPrinter {
	char *code, *description;
	func_detPrinter_openFile func_openFile;
	func_detPrinter_printDetection func_printDetection;
	func_detPrinter_closeFile func_closeFile;
};

static MyMapStringObj *printers = NULL;

void register_detectionsPrinter(const char *code, const char *description,
		func_detPrinter_openFile func_openFile,
		func_detPrinter_printDetection func_printDetection,
		func_detPrinter_closeFile func_closeFile) {
	if (printers == NULL)
		printers = my_mapStringObj_newIgnoreCase();
	struct DetectionsPrinter *pt = MY_MALLOC(1, struct DetectionsPrinter);
	pt->code = my_newString_string(code);
	pt->description = my_newString_string(description);
	pt->func_openFile = func_openFile;
	pt->func_printDetection = func_printDetection;
	pt->func_closeFile = func_closeFile;
	if (my_mapStringObj_get(printers, code) != NULL)
		my_log_error("duplicated %s\n", code);
	my_mapStringObj_add(printers, pt->code, pt);
}
static struct DetectionsPrinter *find_detections_printer(const char *code) {
	struct DetectionsPrinter *pt = my_mapStringObj_get(printers, code);
	if (pt == NULL)
		my_log_error("can't find format %s\n", code);
	return pt;
}
static char *getFormats() {
	MyStringBuffer *sb = my_stringbuf_new();
	for (int64_t i = 0; i < my_mapStringObj_size(printers); ++i) {
		const char *code = my_mapStringObj_getKeyAt(printers, i);
		if (i > 0)
			my_stringbuf_appendChar(sb, ',');
		my_stringbuf_appendString(sb, code);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
void loc_print_detections(CmdParams *cmd_params, char *argOption) {
	register_default_detectionsPrinters();
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info(
				"  -det detFile.txt                  Mandatory. The detection file to print.\n");
		my_log_info(
				"  -out filename                     Optional. Save result to filename. By default, results are printed to stdout.\n");
		my_log_info(
				"  -format string                    Mandatory. valid formats=%s\n",
				getFormats());
		my_log_info(
				"  -forceOS (WIN|NOWIN)              Optional. Force output to a specific OS (windows or not-windows).\n");
		my_log_info(
				"  -overwrite                        Optional. Output file can be overwrited.\n");
		my_log_info("  -minScore val                     Optional.\n");
		my_log_info(
				"  [ -printOnlyQ | -printOnlyR ]     Optional. The output only considers Q or R segments.\n");
		return;
	}
	struct DetPrinterOptions *opt = MY_MALLOC(1, struct DetPrinterOptions);
	opt->useWindowsFormat = IS_WINDOWS ? true : false;
	opt->printBothQR = true;
	struct DetectionsPrinter *printer = NULL;
	const char *detFilename = NULL, *outFilename = NULL;
	double minScore = 0;
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-det")) {
			detFilename = nextParam(cmd_params);
			my_assert_fileExists(detFilename);
		} else if (isNextParam(cmd_params, "-out")) {
			outFilename = nextParam(cmd_params);
		} else if (isNextParam(cmd_params, "-minScore")) {
			minScore = nextParamDouble(cmd_params);
		} else if (isNextParam(cmd_params, "-format")) {
			printer = find_detections_printer(nextParam(cmd_params));
		} else if (isNextParam(cmd_params, "-forceOS")) {
			if (isNextParam(cmd_params, "WIN")) {
				opt->useWindowsFormat = true;
			} else if (isNextParam(cmd_params, "NOWIN")) {
				opt->useWindowsFormat = false;
			} else {
				my_log_error("unknown os '%s'\n", nextParam(cmd_params));
			}
		} else if (isNextParam(cmd_params, "-overwrite")) {
			opt->overwrite = true;
		} else if (isNextParam(cmd_params, "-printOnlyQ")) {
			opt->printBothQR = opt->printOnlyR = false;
			opt->printOnlyQ = true;
		} else if (isNextParam(cmd_params, "-printOnlyR")) {
			opt->printBothQR = opt->printOnlyQ = false;
			opt->printOnlyR = true;
		} else {
			my_log_error("unknown option '%s'\n", nextParam(cmd_params));
		}
	}
	my_assert_notNull("-det", detFilename);
	my_assert_notNull("-format", printer);
	struct SearchProfile *profile = loadProfileFromFile(detFilename);
	struct DetectionsFile *df = loadDetectionsFile(detFilename, profile);
	opt->newline = (opt->useWindowsFormat) ? "\r\n" : "\n";
	FILE *out = printer->func_openFile(outFilename, opt);
	for (int64_t i = 0; i < my_vectorObj_size(df->allDetections); ++i) {
		struct Detection *d = my_vectorObj_get(df->allDetections, i);
		if (minScore > 0 && d->score < minScore)
			continue;
		printer->func_printDetection(out, d, opt);
	}
	printer->func_closeFile(out, opt);
	releaseDetectionsFile(df);
	releaseProfile(profile);
}
