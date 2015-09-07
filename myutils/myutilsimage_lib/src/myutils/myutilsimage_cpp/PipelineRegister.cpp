/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

 #include "Pipeline.hpp"

using namespace my;

bool PipelineRegister::hasInit = false;

void tra_reg_canny();
void tra_reg_crop();
void tra_reg_dog();
void tra_reg_filter();
void tra_reg_threshold();
void tra_reg_equalize();
void tra_reg_gray();
void tra_reg_histgray();
void tra_reg_resize();
void tra_reg_sobel();

void PipelineRegister::registerDefaultFactories() {
	if (hasInit)
		return;
	hasInit = true;
	tra_reg_canny();
	tra_reg_crop();
	tra_reg_dog();
	tra_reg_filter();
	tra_reg_threshold();
	tra_reg_equalize();
	tra_reg_gray();
	tra_reg_histgray();
	tra_reg_resize();
	tra_reg_sobel();
}
