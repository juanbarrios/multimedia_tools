/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "io_util.hpp"
#include "../myutils_c.h"
#include <fstream>

using namespace my;

static std::vector<std::string> TOV(MyVectorString *list) {
	std::vector<std::string> vector;
	for (long long i = 0; i < my_vectorString_size(list); ++i) {
		char *value = my_vectorString_get(list, i);
		if (value != NULL)
			vector.push_back(std::string(value));
	}
	my_vectorString_release(list, true);
	return vector;
}
static MyVectorString *TOMY(std::vector<std::string> vector) {
	MyVectorString *list = my_vectorString_new();
	for (size_t i = 0; i < vector.size(); ++i) {
		std::string value = vector[i];
		my_vectorString_add(list, (char*) value.c_str());
	}
	return list;
}
static std::string TOS(char *string) {
	std::string s(string);
	free(string);
	return s;
}

std::string io::getFilename(std::string filePath) {
	return TOS(my_io_getFilename(filePath.c_str()));
}
std::string io::getFilenameWithoutExtension(std::string filePath) {
	return TOS(my_io_getFilenameWithoutExtension(filePath.c_str()));
}
std::string io::getFilenameExtension(std::string filePath) {
	return TOS(my_io_getFilenameExtension(filePath.c_str()));
}

std::string io::getDirname(std::string filePath) {
	return TOS(my_io_getDirname(filePath.c_str()));
}
std::string io::getAbsolutPath(std::string relativePath) {
	return TOS(my_io_getAbsolutPath(relativePath.c_str()));
}

bool io::deleteFile(std::string filename, bool fail) {
	return my_io_deleteFile(filename.c_str(), fail);
}
bool io::moveFile(std::string filenameOrig, std::string filenameDest, bool fail) {
	return my_io_moveFile(filenameOrig.c_str(), filenameDest.c_str(), fail);
}
void io::copyFile(std::string filenameOrig, std::string filenameDest) {
	my_io_copyFile(filenameOrig.c_str(), filenameDest.c_str());
}

bool io::existsFile(std::string filename) {
	return my_io_existsFile(filename.c_str());
}
bool io::existsDir(std::string dirname) {
	return my_io_existsDir(dirname.c_str());
}
bool io::notExists(std::string filename) {
	return my_io_notExists(filename.c_str());
}

long long io::getFilesize(std::string filename) {
	return my_io_getFilesize(filename.c_str());
}

void io::createDir(std::string path, bool fail) {
	my_io_createDir(path.c_str(), fail);
}
void io::createParentDir(std::string file_path) {
	my_io_createParentDir(file_path.c_str());
}

std::vector<std::string> io::listFilesInDir(std::string path_dir) {
	return TOV(my_io_listFilesInDir(path_dir.c_str()));
}

std::vector<std::string> io::listFilesWithSuffix(std::string path_dir,
		std::string suffix) {
	return TOV(my_io_listFilesWithSuffix(path_dir.c_str(), suffix.c_str()));
}

void io::loadFileBytes(std::string filename, std::vector<char> &file_bytes) {
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();
	file_bytes.reserve(pos);
	ifs.seekg(0, std::ios::beg);
	ifs.read(&file_bytes[0], pos);
}
void io::loadLinesFile(std::string filename,
		std::vector<std::string> &file_lines, bool ignoreComments) {
	std::ifstream ifs(filename);
	for (std::string line; std::getline(ifs, line);) {
		if (!ignoreComments || !my::string::startsWith(line, "#"))
			file_lines.push_back(line);
	}
}
void io::saveLinesFile(std::string filename,
		const std::vector<std::string> &lines) {
	MyVectorString *list = TOMY(lines);
	my_io_saveLinesFile(filename.c_str(), list);
	my_vectorString_release(list, false);
}
void io::saveTextFile(std::string filename, const std::string &text) {
	my_io_saveTextFile(filename.c_str(), text.c_str());
}
void io::saveBytesFile(std::string filename, const void *buffer,
		long long num_bytes) {
	my_io_saveBytesFile(filename.c_str(), buffer, num_bytes);
}

int io::system(std::string command) {
	return my_io_system(command.c_str());
}
