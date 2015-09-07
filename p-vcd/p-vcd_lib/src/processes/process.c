/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "process.h"

void proc_reg_cont();
void proc_reg_export();
void proc_reg_computePca();
void proc_reg_applyPca();
void proc_reg_rootSift();

void pvcd_register_default_processors() {
	proc_reg_cont();
	proc_reg_export();
	proc_reg_computePca();
	proc_reg_applyPca();
	proc_reg_rootSift();
}

struct Processor_Def {
	char *code, *help;
	process_func_new func_new;
	process_func_process_db func_process_db;
	process_func_process_segmentation func_process_segmentation;
	process_func_process_descriptor func_process_descriptor;
	process_func_release func_release;
};

static MyVectorObj *defs_pr = NULL;

static void newProcessorDef(const char *code, const char *help,
		process_func_new func_new, process_func_process_db func_process_db,
		process_func_process_segmentation func_process_segmentation,
		process_func_process_descriptor func_process_descriptor,
		process_func_release func_release) {
	struct Processor_Def *def = MY_MALLOC(1, struct Processor_Def);
	def->code = my_newString_string(code);
	def->help = my_newString_string(help);
	def->func_new = func_new;
	def->func_process_db = func_process_db;
	def->func_process_segmentation = func_process_segmentation;
	def->func_process_descriptor = func_process_descriptor;
	def->func_release = func_release;
	if (defs_pr == NULL) {
		defs_pr = my_vectorObj_new();
	} else {
		for (int64_t i = 0; i < my_vectorObj_size(defs_pr); ++i) {
			struct Processor_Def *d = my_vectorObj_get(defs_pr, i);
			if (my_string_equals(d->code, code))
				my_log_error("processor %s already defined\n", code);
		}
	}
	my_vectorObj_add(defs_pr, def);
}
void addProcessorDefDb(const char *code, const char *help,
		process_func_new func_new, process_func_process_db func_process_db,
		process_func_release func_release) {
	newProcessorDef(code, help, func_new, func_process_db, NULL, NULL,
			func_release);
}
void addProcessorDefSegmentation(const char *code, const char *help,
		process_func_new func_new,
		process_func_process_segmentation func_process_segmentation,
		process_func_release func_release) {
	newProcessorDef(code, help, func_new, NULL, func_process_segmentation,
	NULL, func_release);
}
void addProcessorDefDescriptor(const char *code, const char *help,
		process_func_new func_new,
		process_func_process_descriptor func_process_descriptor,
		process_func_release func_release) {
	newProcessorDef(code, help, func_new, NULL, NULL, func_process_descriptor,
			func_release);
}
void print_processors() {
	my_log_info("\nAvailable Processes for DB (-db):\n");
	for (int64_t i = 0; i < my_vectorObj_size(defs_pr); ++i) {
		struct Processor_Def *proc = my_vectorObj_get(defs_pr, i);
		if (proc->func_process_db != NULL)
			my_log_info("  %s%s%s\n", proc->code,
					(proc->help == NULL) ? "" : "_",
					(proc->help == NULL) ? "" : proc->help);
	}
	my_log_info("\nAvailable Processes for segmentations (-seg):\n");
	for (int64_t i = 0; i < my_vectorObj_size(defs_pr); ++i) {
		struct Processor_Def *proc = my_vectorObj_get(defs_pr, i);
		if (proc->func_process_segmentation != NULL)
			my_log_info("  %s%s%s\n", proc->code,
					(proc->help == NULL) ? "" : "_",
					(proc->help == NULL) ? "" : proc->help);
	}
	my_log_info("\nAvailable Processes for DB (-desc):\n");
	for (int64_t i = 0; i < my_vectorObj_size(defs_pr); ++i) {
		struct Processor_Def *proc = my_vectorObj_get(defs_pr, i);
		if (proc->func_process_descriptor != NULL)
			my_log_info("  %s%s%s\n", proc->code,
					(proc->help == NULL) ? "" : "_",
					(proc->help == NULL) ? "" : proc->help);
	}
}

struct Processor_Def *searchProcessorDef(const char *code) {
	for (int64_t i = 0; i < my_vectorObj_size(defs_pr); ++i) {
		struct Processor_Def *d = my_vectorObj_get(defs_pr, i);
		if (my_string_equals(d->code, code))
			return d;
	}
	my_log_error("unknown process %s\n", code);
	return NULL;
}
struct Processor {
	char *codeAndParameters;
	struct Processor_Def *def;
	void *state;
};

Processor *getProcessor2(const char *code, const char *parameters) {
	Processor *pr = MY_MALLOC(1, Processor);
	pr->def = searchProcessorDef(code);
	pr->def->func_new(pr->def->code, parameters, &pr->state);
	pr->codeAndParameters =
			(parameters == NULL || strlen(parameters) == 0) ?
					my_newString_string(pr->def->code) :
					my_newString_format("%s_%s", pr->def->code, parameters);
	return pr;
}
Processor *getProcessor(const char *codeAndParameters) {
	MyTokenizer *tk = my_tokenizer_new(codeAndParameters, '_');
	const char *code = my_tokenizer_nextToken(tk);
	Processor *ex = getProcessor2(code, my_tokenizer_getCurrentTail(tk));
	my_tokenizer_release(tk);
	return ex;
}
const char *getProcessorHelp(const char *extCode) {
	struct Processor_Def *def = searchProcessorDef(extCode);
	return def->help;
}
bool runProcessDb(Processor *pr, DB *db) {
	if (pr == NULL || db == NULL || pr->def->func_process_db == NULL)
		return false;
	pr->def->func_process_db(db, pr->state);
	return true;
}
bool runProcessSegmentation(Processor *pr, LoadSegmentation *segloader) {
	if (pr == NULL || segloader == NULL
			|| pr->def->func_process_segmentation == NULL)
		return false;
	pr->def->func_process_segmentation(segloader, pr->state);
	return true;
}
bool runProcessDescriptor(Processor *pr, LoadDescriptors *desloader) {
	if (pr == NULL || desloader == NULL
			|| pr->def->func_process_descriptor == NULL)
		return false;
	pr->def->func_process_descriptor(desloader, pr->state);
	return true;
}
void releaseProcessor(Processor *pr) {
	if (pr == NULL)
		return;
	if (pr->def->func_release != NULL)
		pr->def->func_release(pr->state);
	free(pr->codeAndParameters);
	free(pr);
}
