/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "detection.h"

static FILE *openFileEmpty(const char *filename, struct DetPrinterOptions *opt) {
	if (filename == NULL)
		return stdout;
	if (my_io_existsFile(filename) && !opt->overwrite)
		my_log_error("file %s already exists. overwrite it with -overwrite\n",
				filename);
	return my_io_openFileWrite1(filename);
}
static void closeFile(FILE *out, struct DetPrinterOptions *opt) {
	if (out != NULL && out != stdout)
		fclose(out);
}

static FILE *newFileVlc(const char *filename, struct DetPrinterOptions *opt) {
	FILE *out = openFileEmpty(filename, opt);
	if (opt->useWindowsFormat) {
		fprintf(out, "%s\r\n",
				"set PATH=%PATH%;C:\\Program Files\\VideoLAN\\VLC");
	} else {
		fprintf(out, "#!/bin/bash\n");
		fprintf(out, "VLC=\"vlc\";\n");
		fprintf(out, "function showVlc {\n");
		fprintf(out, "   $VLC $1 &;\n");
		fprintf(out, "   local A=$!;\n");
		fprintf(out, "   $VLC $2 &;\n");
		fprintf(out, "   local B=$!;\n");
		fprintf(out, "   wait $A $B;\n");
		//fprintf(out, "read -p 'press any key...'\n");
		fprintf(out, "}\n");
	}
	return out;
}
static char *videoToStringVlc(struct SearchFile *video, double timeStart,
		double timeEnd) {
	FileDB *fdb = video->fdb;
	return my_newString_format(
			"\"%s\" --start-time=%1.1lf --run-time=%1.1lf --play-and-pause",
			fdb->filenameReal, timeStart + fdb->secStartTime,
			timeEnd - timeStart);
}
static void printDetectionVlc(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	char *vq = videoToStringVlc(d->videoQ, d->secStartQ, d->secEndQ);
	char *vr = videoToStringVlc(d->videoR, d->secStartR, d->secEndR);
	char *st = NULL;
	if (opt->useWindowsFormat) {
		char *pref = my_newString_format("echo %5.1lf &", d->score);
		char *vlc = "start /MIN vlc.exe";
		if (opt->printBothQR) {
			st = my_newString_format("%s %s %s & %s %s", pref, vlc, vq, vlc,
					vr);
		} else if (opt->printOnlyQ) {
			st = my_newString_format("%s %s %s", pref, vlc, vq);
		} else if (opt->printOnlyR) {
			st = my_newString_format("%s %s %s", pref, vlc, vr);
		}
		MY_FREE(pref);
	} else {
		if (opt->printBothQR) {
			st = my_newString_format("showVlc '%s' '%s'", vq, vr);
		} else if (opt->printOnlyQ) {
			st = my_newString_format("showVlc '%s'", vq);
		} else if (opt->printOnlyR) {
			st = my_newString_format("showVlc '%s'", vr);
		}
	}
	fprintf(out, "%s%s", st, opt->newline);
	MY_FREE_MULTI(vq, vr, st);
}
static char *videoToStringView(struct SearchFile *video, double timeStart,
		double timeEnd) {
	FileDB *fdb = video->fdb;
	double start = timeStart + fdb->secStartTime;
	double end = timeEnd + fdb->secStartTime;
	if (start > 0 || (end > 0 && end < fdb->secEndTime)) {
		char *sq = my_newString_hhmmssfff(start);
		char *eq = my_newString_hhmmssfff(end);
		char *st = my_newString_format("\"%s\" -start %s -end %s",
				fdb->pathReal, sq, eq);
		MY_FREE_MULTI(sq, eq);
		return st;
	} else {
		return my_newString_format("\"%s\"", fdb->pathReal);
	}
}
static void printDetectionView(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	char *vq = videoToStringView(d->videoQ, d->secStartQ, d->secEndQ);
	char *vr = videoToStringView(d->videoR, d->secStartR, d->secEndR);
	if (opt->printBothQR) {
		fprintf(out, "pvcd_view %s %s%s", vq, vr, opt->newline);
	} else if (opt->printOnlyQ) {
		fprintf(out, "pvcd_view %s%s", vq, opt->newline);
	} else if (opt->printOnlyR) {
		fprintf(out, "pvcd_view %s%s", vr, opt->newline);
	}
	MY_FREE_MULTI(vq, vr);
}
static char *videoToStringFFmpeg(struct SearchFile *video, double timeStart,
		double timeEnd) {
	FileDB *fdb = video->fdb;
	double start = timeStart + fdb->secStartTime;
	double end = timeEnd + fdb->secStartTime;
	double length = timeEnd - timeStart;
	char *nam = my_subStringC_startFirst(fdb->filenameReal, '.');
	char *ext = my_subStringC_lastEnd(fdb->filenameReal, '.');
	char *newname = my_newString_format("%s_%1.0lf_%1.0lf.%s", nam, start, end,
			ext);
	char *stStart = my_newString_hhmmssfff(start);
	char *st = my_newString_format("ffmpeg -y -ss %s -i %s -t %1.1lf %s\n",
			stStart, fdb->pathReal, length, newname);
	MY_FREE_MULTI(nam, ext, newname, stStart);
	return st;
}
static void printDetectionFFmpeg(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	char *vq = videoToStringFFmpeg(d->videoQ, d->secStartQ, d->secEndQ);
	char *vr = videoToStringFFmpeg(d->videoR, d->secStartR, d->secEndR);
	if (opt->printBothQR) {
		fprintf(out, "%s%s%s", vr, vq, opt->newline);
	} else if (opt->printOnlyQ) {
		fprintf(out, "%s%s", vq, opt->newline);
	} else if (opt->printOnlyR) {
		fprintf(out, "%s%s", vr, opt->newline);
	}
	MY_FREE_MULTI(vq, vr);
}
static FILE *newFileXls(const char *filename, struct DetPrinterOptions *opt) {
	FILE *out = openFileEmpty(filename, opt);
	fprintf(out,
			"startDate\tstartHour\tendHour\tblock\tqueryName\tstart_q\tend_q\tlength_q\trefName\tstart_r\tend_r\tlength_r\tscore\tVLCQ\tVLCR%s",
			opt->newline);
	return out;
}
static char *videoToStringXls(struct SearchFile *video, double timeStart,
		double timeEnd) {
	FileDB *fdb = video->fdb;
	double start = timeStart + fdb->secStartTime;
	double end = timeEnd + fdb->secStartTime;
	double length = timeEnd - timeStart;
	char *st = my_newString_format("%s\t%1.1lf\t%1.1lf\t%1.1lf",
			fdb->filenameReal, start, end, length);
	return st;
}
static void printDetectionXls(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	char *vq = videoToStringXls(d->videoQ, d->secStartQ, d->secEndQ);
	char *vr = videoToStringXls(d->videoR, d->secStartR, d->secEndR);
	char *vlcq = videoToStringVlc(d->videoQ, d->secStartQ, d->secEndQ);
	char *vlcr = videoToStringVlc(d->videoR, d->secStartR, d->secEndR);
	char *score_st = my_newString_doubleDec(d->score, 2);
	fprintf(out, "%s\t%s\t%s\tstart /MIN vlc.exe %s\tstart /MIN vlc.exe %s%s",
			vq, vr, score_st, vlcq, vlcr, opt->newline);
	MY_FREE_MULTI(vq, vr, score_st, vlcq, vlcr);
}
static FILE *newFileDB(const char *filename, struct DetPrinterOptions *opt) {
	if (filename == NULL) {
		return openFileEmpty(filename, opt);
	} else {
		DB *newDb = loadDB(filename, false, false);
		return newDBfile(newDb);
	}
}
static MyVectorString *printedList = NULL;

static void printDbFilesVideo(FILE *out, struct SearchFile *video,
		double timeStart, double timeEnd, struct DetPrinterOptions *opt) {
	if (printedList == NULL) {
		printedList = my_vectorString_new();
		my_vectorString_keepSortedCaseSensitive(printedList);
	}
	FileDB *fdb = video->fdb;
	double newStart = fdb->secStartTime + timeStart;
	double newEnd = fdb->secStartTime + timeEnd;
	char *start_st = my_newString_doubleDec(newStart, 0);
	char *end_st = my_newString_doubleDec(newEnd, 0);
	char *newId = my_newString_format("%s_%s_%s", fdb->id, start_st, end_st);
	if (my_vectorString_indexOf_binsearch(printedList, newId) < 0) {
		my_vectorString_add(printedList, my_newString_string(newId));
		double oldStart = fdb->secStartTime;
		double oldEnd = fdb->secEndTime;
		char *oldId = fdb->id;
		fdb->secStartTime = newStart;
		fdb->secEndTime = newEnd;
		fdb->id = newId;
		addFileDB_toDB(out, fdb);
		fdb->secStartTime = oldStart;
		fdb->secEndTime = oldEnd;
		fdb->id = oldId;
	}
	MY_FREE_MULTI(start_st, end_st, newId);
}
static void printDetectionDB(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	if (opt->printBothQR || opt->printOnlyQ)
		printDbFilesVideo(out, d->videoQ, d->secStartQ, d->secEndQ, opt);
	if (opt->printBothQR || opt->printOnlyR)
		printDbFilesVideo(out, d->videoR, d->secStartR, d->secEndR, opt);
}
static char *getTimeLNK(double seconds) {
	int n = my_math_round_int(seconds);
	int s = n % 60;
	int m = n / 60;
	return my_newString_format("%i.%i", m, s);
}

static void printDetectionLNK(FILE *out, struct Detection *d,
		struct DetPrinterOptions *opt) {
	char *start = getTimeLNK(d->secStartR);
	char *end = getTimeLNK(d->secEndR);
	char *rank = my_newString_int(d->scoreRankInQuery);
	char *score = my_newString_doubleDec(d->score, 4);
	fprintf(out, "%s Q0 %s %s %s %s %s RUN_ID%s", d->videoQ->name, d->videoR->name,
			start, end, rank, score, opt->newline);
	MY_FREE_MULTI(start, end, rank, score);
}

void register_default_detectionsPrinters() {
	register_detectionsPrinter("VLC", "VideoLan Player", newFileVlc,
			printDetectionVlc, closeFile);
	register_detectionsPrinter("VIEW", "Internal player", openFileEmpty,
			printDetectionView, closeFile);
	register_detectionsPrinter("FFMPEG", "Export with FFMPEG", openFileEmpty,
			printDetectionFFmpeg, closeFile);
	register_detectionsPrinter("TXT", "Text separated by tab", newFileXls,
			printDetectionXls, closeFile);
	register_detectionsPrinter("DB", "Internal database", newFileDB,
			printDetectionDB, closeFile);
	register_detectionsPrinter("LNK", "LNK", openFileEmpty, printDetectionLNK,
			closeFile);
}

