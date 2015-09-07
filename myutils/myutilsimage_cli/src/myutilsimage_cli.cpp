/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include <iostream>
#include <myutils/myutils_cpp.hpp>
#include <myutils/myutilsimage_cpp.hpp>
#include <stdexcept>
#include <sstream>

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OpenFile {
public:
	OpenFile(std::string filename, int internal_id) :
			filename(filename), internal_id(internal_id), ended(false) {
		if (!my::string::startsWith_ignorecase(filename, "webcam")
				&& !my::io::existsFile(filename)) {
			throw std::runtime_error("no puedo abrir " + filename);
		}
		std::ostringstream os;
		os << "VIDEO" << internal_id << "_" << filename;
		window_name = os.str();

	}
	void addTransform(std::string tr) {
		pipeline.addPipeItem(tr);
	}
	void open() {
		if (my::string::startsWith_ignorecase(filename, "webcam")) {
			int id_webcam = my::parse::stringToInt(filename.erase(0, 6));
			std::cout << "abriendo camara " << id_webcam << std::endl;
			capture.open(id_webcam);
		} else {
			std::cout << "abriendo video " << filename << std::endl;
			capture.open(filename);
		}
		if (!capture.isOpened()) {
			throw std::runtime_error("no puedo abrir " + filename);
		}
	}
	cv::Mat &getFrame(bool &hasEnded) {
		if (ended) {
			hasEnded = true;
			return frame_transform;
		}
		if (!capture.grab() || !capture.retrieve(frame_original)) {
			hasEnded = ended = true;
			return frame_transform;
		}
		hasEnded = false;
		frame_transform = pipeline.transform(frame_original);
		return frame_transform;
	}
	std::string getWindowName() {
		return window_name;
	}
	void close() {
		capture.release();
	}
private:
	std::string filename;
	int internal_id;
	bool ended;
	std::string window_name;
	cv::Mat frame_original;
	cv::Mat frame_transform;
	cv::VideoCapture capture;
	my::Pipeline pipeline;
};

void process(const std::vector<std::string> &args) {
	std::vector<OpenFile> openFiles;
	for (size_t i = 1; i < args.size(); ++i) {
		if (args[i] == "-show") {
			my::PipeItemAbstractFactory::printAll();
			return;
		} else if (my::io::existsFile(args[i])) {
			OpenFile file(args[i], openFiles.size());
			openFiles.push_back(file);
		} else if (my::PipeItemAbstractFactory::existsPipeItem(args[i])) {
			if (openFiles.size() == 0)
				throw std::runtime_error("transformation without file");
			openFiles[openFiles.size() - 1].addTransform(args[i]);
		} else {
			throw std::runtime_error("unknown " + args[i]);
		}
	}
	for (size_t i = 0; i < openFiles.size(); ++i) {
		openFiles[i].open();
	}
	for (size_t i = 0; i < openFiles.size(); ++i) {
		cv::namedWindow(openFiles[i].getWindowName(), cv::WINDOW_AUTOSIZE);
	}
	int delay = 30;
	int last_delay = delay;
	for (;;) {
		bool allEnded = true;
		for (size_t i = 0; i < openFiles.size(); ++i) {
			bool hasEnded = false;
			cv::Mat frame = openFiles[i].getFrame(hasEnded);
			if (hasEnded)
				continue;
			allEnded = false;
			cv::imshow(openFiles[i].getWindowName(), frame);
		}
		if (allEnded) {
			cv::waitKey(0);
			break;
		}
		char c = cv::waitKey(delay);
		if (c == 27) {
			break;
		} else if (c == 32) {
			if (delay == 0) {
				delay = last_delay;
			} else {
				last_delay = delay;
				delay = 0;
			}
		}
	}
	for (size_t i = 0; i < openFiles.size(); ++i) {
		cv::destroyWindow(openFiles[i].getWindowName());
		openFiles[i].close();
	}
}
#endif

int main(int argc, char **argv) {
	std::vector<std::string> args = my::collection::args_to_vector(argc, argv);
	if (args.size() < 2) {
		std::cout << "Uso: " << args[0]
				<< " [video_filename | webcamID] [filters...] [video filters ...]"
				<< std::endl;
		std::cout << "Uso: " << args[0] << " -show" << std::endl;
		return EXIT_FAILURE;
	}
	try {
#ifndef NO_OPENCV
		process(args);
#else
		throw std::runtime_error("compiled without support to opencv");
#endif
	} catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR" << std::endl;
	}
	return EXIT_SUCCESS;
}
