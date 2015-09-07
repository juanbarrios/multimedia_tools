/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef PROCESSES_H
#define PROCESSES_H

#include "../pvcd.h"

void pvcd_register_default_processors();
void print_processors();

typedef void (*process_func_new)(const char *code, const char *parameters,
		void **out_state);
typedef void (*process_func_process_db)(DB *db, void *state);
typedef void (*process_func_process_segmentation)(LoadSegmentation *segloader,
		void *state);
typedef void (*process_func_process_descriptor)(LoadDescriptors *desloader,
		void *state);
typedef void (*process_func_release)(void *state);

void addProcessorDefDb(const char *code, const char *help,
		process_func_new func_new, process_func_process_db func_process_dbs,
		process_func_release func_release);
void addProcessorDefSegmentation(const char *code, const char *help,
		process_func_new func_new,
		process_func_process_segmentation func_process_segmentations,
		process_func_release func_release);
void addProcessorDefDescriptor(const char *code, const char *help,
		process_func_new func_new,
		process_func_process_descriptor func_process_descriptors,
		process_func_release func_release);

typedef struct Processor Processor;

const char *getProcessorHelp(const char *prCode);
Processor *getProcessor2(const char *prCode, const char *prParameters);
Processor *getProcessor(const char *prCodeAndParameters);

bool runProcessDb(Processor *pr, DB *db);
bool runProcessSegmentation(Processor *pr, LoadSegmentation *segloader);
bool runProcessDescriptor(Processor *pr, LoadDescriptors *desloader);

void releaseProcessor(Processor *pr);

#endif
