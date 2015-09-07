/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PRINTER_H_
#define MKNN_PRINTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 *  A MknnPrinter converts an object from a given domain to its string representation.
 *
 *  @file
 */

/**
 * Creates an empty object.
 *
 * @param domain the domain of the objects
 * @return a new printer object.
 */
MknnPrinter *mknn_printer_new(MknnDomain *domain);

/**
 * Creates a new string
 * @param object the object to convert
 * @param printer the printer object
 * @return a new string (must be freed)
 */
char *mknn_printer_objectToNewString(MknnPrinter *printer, void *object);

/**
 * Changes the format for objects of the general domain vector.
 *
 * @param vector_prefix the string that is print in front of the vector.
 * @param dimension_separator the string that is print between dimensions.
 * @param vector_suffix the string that is print after the vector.
 * @param printer the printer object
 */
void mknn_printer_setVectorFormat(MknnPrinter *printer,
		const char *vector_prefix, const char *dimension_separator,
		const char *vector_suffix);

/**
 * Changes the format for objects of the general domain multiobject.
 *
 * @param prefix the string that is print in front of the multiobject.
 * @param separator the string that is print between objects.
 * @param suffix the string that is print after the multiobject.
 * @param printer the printer object
 */
void mknn_printer_setMultiobjectFormat(MknnPrinter *printer, const char *prefix,
		const char *separator, const char *suffix);

/**
 * Releases the printer.
 *
 * @param printer
 */
void mknn_printer_release(MknnPrinter *printer);

#ifdef __cplusplus
}
#endif

#endif
