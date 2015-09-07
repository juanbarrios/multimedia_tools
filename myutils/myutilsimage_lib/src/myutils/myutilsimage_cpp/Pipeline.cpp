/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "Pipeline.hpp"
#include <myutils/myutils_cpp.hpp>
#include <iostream>

#ifndef NO_OPENCV
using namespace my;

PipeItem::PipeItem() {
}
PipeItem::~PipeItem() {
}

std::vector<PipeItemFactory*> PipeItemAbstractFactory::all_factories;

PipeItemFactory::PipeItemFactory(std::string codename) :
		codename(codename) {
}
std::string PipeItemFactory::getCodename() {
	return this->codename;
}
PipeItemFactory::~PipeItemFactory() {
}

std::vector<PipeItemFactory*> & PipeItemAbstractFactory::getAllFactories() {
	PipelineRegister::registerDefaultFactories();
	return all_factories;
}

PipeItemFactory *PipeItemAbstractFactory::getFactory(std::string codename,
		bool failIfNotFound) {
	for (PipeItemFactory *fac : getAllFactories()) {
		if (fac->getCodename() == codename)
			return fac;
	}
	if (failIfNotFound)
		throw std::runtime_error("can't find " + codename);
	else
		return NULL;
}
void PipeItemAbstractFactory::printAll() {
	for (PipeItemFactory *fac : getAllFactories()) {
		std::string help = fac->getParametersHelp();
		if (help == "")
			std::cout << "  " << fac->getCodename() << std::endl;
		else
			std::cout << "  " << fac->getCodename() << "_" << help << std::endl;
	}
}
void PipeItemAbstractFactory::registerFactory(PipeItemFactory *factory) {
	PipeItemFactory *fac = PipeItemAbstractFactory::getFactory(
			factory->getCodename(), false);
	if (fac != NULL)
		throw std::runtime_error("duplicated " + factory->getCodename());
	getAllFactories().push_back(factory);
}
bool PipeItemAbstractFactory::existsPipeItem(std::string parameters) {
	std::string codename;
	if (string::indexOf(parameters, "_") < 0) {
		codename = parameters;
	} else {
		codename = subStringC::startFirst(parameters, '_');
	}
	PipeItemFactory *fac = PipeItemAbstractFactory::getFactory(codename, false);
	return (fac != NULL);
}
PipeItem *PipeItemAbstractFactory::newPipeItem(std::string parameters) {
	std::string codename, args;
	if (string::indexOf(parameters, "_") < 0) {
		codename = parameters;
		args = "";
	} else {
		codename = subStringC::startFirst(parameters, '_');
		args = subStringC::firstEnd(parameters, '_');
	}
	PipeItemFactory *fac = PipeItemAbstractFactory::getFactory(codename, true);
	return fac->newItem(args);
}

Pipeline::Pipeline() {
}
Pipeline::Pipeline(const Pipeline &p) {
	for (std::string arg : p.args) {
		addPipeItem(arg);
	}
}

void Pipeline::addPipeItem(std::string parameters) {
	PipeItem *item = PipeItemAbstractFactory::newPipeItem(parameters);
	this->args.push_back(parameters);
	this->items.push_back(item);
}
cv::Mat &Pipeline::transform(cv::Mat &image) {
	if (this->items.size() == 0) {
		return image;
	} else if (this->items.size() == 1) {
		return this->items[0]->transform(image);
	} else {
		cv::Mat out = image;
		for (size_t i = 0; i < this->items.size() - 1; ++i) {
			out = this->items[i]->transform(out);
		}
		return this->items[this->items.size() - 1]->transform(out);
	}
}
Pipeline::~Pipeline() {
	for (PipeItem *item : this->items) {
		delete item;
	}
}
#endif
