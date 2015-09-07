/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MYUTILSIMAGE_PIPELINE_HPP
#define MYUTILSIMAGE_PIPELINE_HPP

#include "../myutilsimage_cpp.hpp"

#ifndef NO_OPENCV
#include <opencv2/core/core.hpp>
#endif

namespace my {

class PipeItem {
public:
	PipeItem();
#ifndef NO_OPENCV
	virtual cv::Mat &transform(cv::Mat &image)=0;
#endif
	virtual ~PipeItem()=0;
private:
	PipeItem(const PipeItem &);
	PipeItem& operator=(const PipeItem &);
};

class PipeItemFactory {
public:
	PipeItemFactory(std::string codename);
	std::string getCodename();
	virtual std::string getParametersHelp()=0;
	virtual PipeItem *newItem(std::string parameters)=0;
	virtual ~PipeItemFactory()=0;

private:
	PipeItemFactory(const PipeItemFactory &);
	PipeItemFactory& operator=(const PipeItemFactory &);
	std::string codename;
};

class PipeItemAbstractFactory {
public:
	static void registerFactory(PipeItemFactory *factory);
	static void printAll();
	static bool existsPipeItem(std::string parameters);
	static PipeItem *newPipeItem(std::string parameters);

private:
	static PipeItemFactory *getFactory(std::string codename,
			bool failIfNotFound);
	static std::vector<PipeItemFactory*> & getAllFactories();

	static std::vector<PipeItemFactory*> all_factories;
};

class PipelineRegister {
public:
	static void registerDefaultFactories();
private:
	static bool hasInit;
};

class Pipeline {
public:
	Pipeline();
	void addPipeItem(std::string parameters);
#ifndef NO_OPENCV
	cv::Mat &transform(cv::Mat &image);
#endif
	~Pipeline();
	Pipeline(const Pipeline &p);
private:
	Pipeline& operator=(const Pipeline &p);
	std::vector<std::string> args;
	std::vector<PipeItem*> items;
};

}

#endif
