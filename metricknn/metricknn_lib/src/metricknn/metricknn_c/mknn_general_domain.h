/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_GENERAL_DOMAIN_H_
#define MKNN_GENERAL_DOMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * General Domain Constants
 * @file
 */

/**
 * General Domain of Strings.
 * The objects will be casted to <tt>char*</tt>.
 * As usual in C, the end of each string is given by <tt>char '\0'</tt>.
 */
extern MknnGeneralDomain MKNN_GENERAL_DOMAIN_STRING;

/**
 * General Domain of Vectors.
 * In order to declare a domain the number of dimensions and datatype are needed.
 * The objects will be casted to <tt>datatype*</tt>. See @p MKNN_DATATYPE_xxx constants.
 */
extern MknnGeneralDomain MKNN_GENERAL_DOMAIN_VECTOR;

/**
 * General Domain of Multi-Object.
 * The objects will be casted to <tt>void**</tt> (an array of objects).
 */
extern MknnGeneralDomain MKNN_GENERAL_DOMAIN_MULTIOBJECT;

/**
 * General Domain of Custom Objects.
 * The custom objects are not casted, thus they cannot be saved or printed.
 */
extern MknnGeneralDomain MKNN_GENERAL_DOMAIN_CUSTOMOBJECT;

/**
 * Returns if two domains are identical.
 * @param general_domain1 one of the constants @c MKNN_GENERAL_DOMAIN_xxx.
 * @param general_domain2 one of the constants @c MKNN_GENERAL_DOMAIN_xxx.
 * @return true if general_domain1 and general_domain2 are identical.
 */
bool mknn_generalDomain_areEqual(const MknnGeneralDomain general_domain1,
		const MknnGeneralDomain general_domain2);

bool mknn_generalDomain_isString(const MknnGeneralDomain general_domain);
bool mknn_generalDomain_isVector(const MknnGeneralDomain general_domain);
bool mknn_generalDomain_isMultiObject(const MknnGeneralDomain general_domain);
bool mknn_generalDomain_isCustom(const MknnGeneralDomain general_domain);

/**
 *
 * @param general_domain the constant
 * @return a string with a string representation
 */
const char *mknn_generalDomain_toString(const MknnGeneralDomain general_domain);

/**
 *
 * @param string
 * @return a general domain constant
 */
/**
 *
 * @param string the string to read and parse a general domain. The string should be a value returned by #mknn_generalDomain_toString.
 * @param out_general_domain the parsed value
 * @return true if some value was assigned to out_general_domain
 */
bool mknn_generalDomain_parseString(const char *string,
		MknnGeneralDomain *out_general_domain);

#ifdef __cplusplus
}
#endif

#endif
