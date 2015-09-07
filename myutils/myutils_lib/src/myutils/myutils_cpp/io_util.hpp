/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_IO_UTIL_HPP
#define MY_IO_UTIL_HPP

#include "../myutils_cpp.hpp"

namespace my {
class io {
public:
	static std::string getFilename(std::string filePath);
	static std::string getFilenameWithoutExtension(std::string filePath);
	static std::string getFilenameExtension(std::string filePath);

	static std::string getDirname(std::string filePath);
	static std::string getAbsolutPath(std::string relativePath);

	static bool deleteFile(std::string filename, bool fail);
	static bool moveFile(std::string filenameOrig, std::string filenameDest,
			bool fail);
	static void copyFile(std::string filenameOrig, std::string filenameDest);

	static bool existsFile(std::string filename);
	static bool existsDir(std::string dirname);
	static bool notExists(std::string filename);

	static long long getFilesize(std::string filename);

	static void createDir(std::string path, bool fail);
	static void createParentDir(std::string file_path);

	static std::vector<std::string> listFilesInDir(std::string path_dir);

	static std::vector<std::string> listFilesWithSuffix(std::string path_dir,
			std::string suffix);

	static void loadFileBytes(std::string filename,
			std::vector<char> &file_bytes);
	static void loadLinesFile(std::string filename,
			std::vector<std::string> &file_lines, bool ignoreComments = true);

	static void saveLinesFile(std::string filename,
			const std::vector<std::string> &lines);
	static void saveTextFile(std::string filename, const std::string &text);
	static void saveBytesFile(std::string filename, const void *buffer,
			long long num_bytes);

	static int system(std::string command);
};
}

#endif
