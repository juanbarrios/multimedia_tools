/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "audio.h"

//ffmpeg -i file -f s16le -acodec pcm_s16le -ar 8192 -ac 1 -vn -sn audio.pcm
//vlc --demux=rawaud --rawaud-channels 1 --rawaud-samplerate 8192 audio.pcm
void loadAudioData_s16le_mono(FileDB *fdb, int64_t sampleRate,
		int64_t *out_numSamples, short **out_samples) {
	char *pathAudioTemp = my_newString_format("%s/audio_pcm_s16le_mono_%"PRIi64,
			fdb->db->pathBase, sampleRate);
	my_io_createDir(pathAudioTemp, 0);
	char *filenamePcm = my_newString_format("%s/%s.raw", pathAudioTemp, fdb->id);
	char *filenameLog = my_newString_format("%s/%s.log", pathAudioTemp, fdb->id);
	if (!my_io_existsFile(filenamePcm) || my_io_getFilesize(filenamePcm) == 0) {
		char *paramStart, *paramLength;
		if (fdb->secStartTime != 0 || fdb->secEndTime != fdb->lengthSec) {
			char *stStart = my_newString_hhmmssfff(fdb->secStartTime);
			paramStart = my_newString_format("-ss %s", stStart);
			paramLength = my_newString_format("-t %1.3lf",
					fdb->secEndTime - fdb->secStartTime);
			MY_FREE(stStart);
		} else {
			paramStart = my_newString_string("");
			paramLength = my_newString_string("");
		}
		char *cmd =
				my_newString_format(
						"-y %s -i \"%s\" %s -ac 1 -vn -sn -ar %"PRIi64" -f s16le -acodec pcm_s16le \"%s\" > \"%s\" 2>&1",
						paramStart, fdb->pathReal, paramLength, sampleRate,
						filenamePcm, filenameLog);
		pvcd_system_call_ffmpeg(cmd);
		MY_FREE_MULTI(cmd, paramStart, paramLength);
	}
	int64_t filesize = 0;
	void* bytes = my_io_loadFileBytes(filenamePcm, &filesize);
	MY_FREE_MULTI(pathAudioTemp, filenamePcm, filenameLog);
	*out_samples = (short*) bytes;
	*out_numSamples = filesize / 2;
}

void computeAudioLength(const char *filename, double *out_seconds,
		double *out_samplerate) {
	char *filenameLog = my_io_temp_createNewFile(".log");
	char *cmd = my_newString_format(
			"-show_streams -select_streams a:0 \"%s\" > \"%s\" 2>&1", filename,
			filenameLog);
	pvcd_system_call_ffprobe(cmd);
	*out_seconds = 0;
	*out_samplerate = 0;
	MyLineReader *reader = my_lreader_open(my_io_openFileRead1(filenameLog, 0));
	if (reader != NULL) {
		const char *line;
		while ((line = my_lreader_readLine(reader)) != NULL) {
			if (my_string_startsWith(line, "duration=")) {
				*out_seconds = my_parse_double_isubFromEnd(line, 9);
			} else if (my_string_startsWith(line, "sample_rate=")) {
				*out_samplerate = my_parse_double_isubFromEnd(line, 12);
			}
		}
		my_lreader_close(reader, true);
	}
	my_io_deleteFile(filenameLog, false);
	MY_FREE_MULTI(cmd, filenameLog);
}
