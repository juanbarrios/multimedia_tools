/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "io_lreader_util.h"

struct MyLineReader {
	FILE *inFile;
	char *buffer, *line;
	bool ignoreComments, eof;
	int64_t buffer_size, buffer_filled, line_size, pos_buffer;
};

MyLineReader *my_lreader_open_params(FILE *inFile, int64_t buffer_size) {
	if (inFile == NULL)
		return NULL;
	MyLineReader *reader = MY_MALLOC(1, MyLineReader);
	reader->inFile = inFile;
	reader->eof = false;
	reader->line_size = reader->buffer_size = buffer_size;
	reader->buffer = MY_MALLOC(reader->buffer_size, char);
	reader->line = MY_MALLOC(reader->line_size, char);
	reader->buffer_filled = reader->pos_buffer = 0;
	reader->ignoreComments = true;
	return reader;
}
MyLineReader *my_lreader_open(FILE *inFile) {
	return my_lreader_open_params(inFile, 8192);
}
static void verify_header(MyLineReader *reader, const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version) {
	const char *line = my_lreader_readLineOrComment(reader);
	char *expected = my_io_getConfigFileHeader(software_name, format_file,
			major_version, minor_version);
	if (line == NULL || !my_string_equals(line, expected))
		my_log_error("invalid file (incorrect format)\n'%s'!='%s'\n", line,
				expected);
	free(expected);
}
MyLineReader *my_lreader_config_open_params(FILE *inFile, int64_t buffer_size,
		const char *software_name, const char *format_file,
		int64_t major_version, int64_t minor_version) {
	if (inFile == NULL)
		return NULL;
	MyLineReader *reader = my_lreader_open_params(inFile, buffer_size);
	verify_header(reader, software_name, format_file, major_version,
			minor_version);
	return reader;
}
MyLineReader *my_lreader_config_open(FILE *inFile, const char *software_name,
		const char *format_file, int64_t major_version, int64_t minor_version) {
	if (inFile == NULL)
		return NULL;
	MyLineReader *reader = my_lreader_open(inFile);
	verify_header(reader, software_name, format_file, major_version,
			minor_version);
	return reader;
}
static void priv_loadBuffer(MyLineReader *reader) {
	reader->pos_buffer = 0;
	reader->buffer_filled = fread(reader->buffer, sizeof(char),
			reader->buffer_size, reader->inFile);
	if (reader->buffer_filled == 0)
		reader->eof = true;
}
static char priv_nextChar(MyLineReader *reader) {
	if (reader->eof)
		return '\0';
	else if (reader->pos_buffer >= reader->buffer_filled)
		priv_loadBuffer(reader);
	if (reader->eof)
		return '\0';
	char c = reader->buffer[reader->pos_buffer];
	reader->pos_buffer++;
	return c;
}
static void priv_loadNextLineFromBuffer(MyLineReader *reader) {
	int64_t pos_linea = 0;
	char c;
	while ((c = priv_nextChar(reader)) != '\0') {
		if (c == '\r') {
			c = priv_nextChar(reader);
			if (c != '\n' && c != '\0')
				reader->pos_buffer--;
			break;
		} else if (c == '\n') {
			c = priv_nextChar(reader);
			if (c != '\r' && c != '\0')
				reader->pos_buffer--;
			break;
		}
		if (pos_linea >= reader->line_size - 1) {
			reader->line_size++;
			MY_REALLOC(reader->line, reader->line_size, char);
		}
		reader->line[pos_linea] = c;
		pos_linea++;
	}
	reader->line[pos_linea] = '\0';
}
const char *my_lreader_readLine(MyLineReader *reader) {
	if (reader == NULL)
		return NULL;
	while (!reader->eof) {
		priv_loadNextLineFromBuffer(reader);
		if (!reader->ignoreComments
				|| (reader->ignoreComments && reader->line[0] != '\0'
						&& reader->line[0] != '#')) {
			return reader->line;
		}
	}
	return NULL;
}
const char *my_lreader_readLineOrComment(MyLineReader *reader) {
	bool prev = reader->ignoreComments;
	reader->ignoreComments = false;
	const char *line = my_lreader_readLine(reader);
	reader->ignoreComments = prev;
	return line;
}
char *my_lreader_readLine_newString(MyLineReader *reader) {
	return my_newString_string(my_lreader_readLine(reader));
}
void my_lreader_close(MyLineReader *reader, bool close_input_file) {
	if (reader == NULL)
		return;
	if (close_input_file)
		fclose(reader->inFile);
	MY_FREE(reader->buffer);
	MY_FREE(reader->line);
	MY_FREE(reader);
}
