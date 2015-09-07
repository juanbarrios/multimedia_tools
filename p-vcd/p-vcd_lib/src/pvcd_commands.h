/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#ifndef PVCD_SRC_PVCD_COMMANDS_H_
#define PVCD_SRC_PVCD_COMMANDS_H_

#ifdef __cplusplus
extern "C"{
#endif

int pvcd_processDB(int argc, char **argv);
int pvcd_copyDetection(int argc, char **argv);
int pvcd_viewLocalMatches(int argc, char **argv);
int pvcd_similaritySearch(int argc, char **argv);
int pvcd_mergeLocalToGlobal(int argc, char **argv);
int pvcd_mergeNN(int argc, char **argv);
int pvcd_evaluate(int argc, char **argv);
int pvcd_viewImageVideos(int argc, char **argv);

void pvcd_system_setAfterInitFunction(void (*func_afterInit)());
void pvcd_system_setBeforeExitFunction(void (*func_beforeExit)());

#ifdef __cplusplus
}
#endif

#endif
