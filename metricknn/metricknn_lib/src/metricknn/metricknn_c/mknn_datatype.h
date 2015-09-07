/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATATYPE_H_
#define MKNN_DATATYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * Datatype Constants
 * @file
 */

/**
 * Integer  8 bits-length signed. Corresponds to @c int8_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_8bits;
/**
 * Integer 16 bits-length signed. Corresponds to @c int16_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_16bits;
/**
 * Integer 32 bits-length signed. Corresponds to @c int32_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_32bits;
/**
 * Integer 64 bits-length signed. Corresponds to @c int64_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_64bits;

/**
 * Integer  8 bits-length unsigned. Corresponds to  @c uint8_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_8bits;
/**
 * Integer 16 bits-length unsigned. Corresponds to @c uint16_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_16bits;
/**
 * Integer 32 bits-length unsigned. Corresponds to @c uint32_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_32bits;
/**
 * Integer 64 bits-length unsigned. Corresponds to @c uint64_t (see stdint.h).
 */
extern MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_64bits;

/**
 * Floating point 32 bits-length. Corresponds to standard C type @c float.
 */
extern MknnDatatype MKNN_DATATYPE_FLOATING_POINT_32bits;

/**
 * Floating point 64 bits-length. Corresponds to standard C type @c double.
 */
extern MknnDatatype MKNN_DATATYPE_FLOATING_POINT_64bits;

/**
 * Returns whether the given datatype is one of the signed integer types.
 * @param datatype one of the constants @c MKNN_DATATYPE_xxx.
 * @return true if datatype is
 * #MKNN_DATATYPE_SIGNED_INTEGER_8bits or
 * #MKNN_DATATYPE_SIGNED_INTEGER_16bits or
 * #MKNN_DATATYPE_SIGNED_INTEGER_32bits or
 * #MKNN_DATATYPE_SIGNED_INTEGER_64bits, false otherwise.
 */
bool mknn_datatype_isAnySignedInteger(const MknnDatatype datatype);

/**
 * Returns whether the given datatype is one of the unsigned integer types.
 * @param datatype one of the constants @c MKNN_DATATYPE_xxx.
 * @return true if datatype is
 * #MKNN_DATATYPE_UNSIGNED_INTEGER_8bits or
 * #MKNN_DATATYPE_UNSIGNED_INTEGER_16bits or
 * #MKNN_DATATYPE_UNSIGNED_INTEGER_32bits or
 * #MKNN_DATATYPE_UNSIGNED_INTEGER_64bits, false otherwise.
 */
bool mknn_datatype_isAnyUnsignedInteger(const MknnDatatype datatype);

/**
 * Returns whether the given datatype is one of the floating point types.
 * @param datatype one of the constants @c MKNN_DATATYPE_xxx.
 * @return true if datatype is
 * #MKNN_DATATYPE_FLOATING_POINT_32bits or
 * #MKNN_DATATYPE_FLOATING_POINT_64bits, false otherwise.
 */
bool mknn_datatype_isAnyFloatingPoint(const MknnDatatype datatype);

bool mknn_datatype_isInt8(const MknnDatatype datatype);
bool mknn_datatype_isInt16(const MknnDatatype datatype);
bool mknn_datatype_isInt32(const MknnDatatype datatype);
bool mknn_datatype_isInt64(const MknnDatatype datatype);
bool mknn_datatype_isUInt8(const MknnDatatype datatype);
bool mknn_datatype_isUInt16(const MknnDatatype datatype);
bool mknn_datatype_isUInt32(const MknnDatatype datatype);
bool mknn_datatype_isUInt64(const MknnDatatype datatype);
bool mknn_datatype_isFloat(const MknnDatatype datatype);
bool mknn_datatype_isDouble(const MknnDatatype datatype);

/**
 * Returns if two datatypes are identical.
 * @param datatype1 one of the constants @c MKNN_DATATYPE_xxx.
 * @param datatype2 one of the constants @c MKNN_DATATYPE_xxx.
 * @return true if datatype1 and datatype2 are identical.
 */
bool mknn_datatype_areEqual(const MknnDatatype datatype1, const MknnDatatype datatype2);

/**
 * Returns the size in bytes of the given datatype (a number between 1 and 8).
 * @param datatype one of the constants @c MKNN_DATATYPE_xxx.
 * @return size in bytes of the given datatype.
 */
size_t mknn_datatype_sizeof(const MknnDatatype datatype);

/**
 *
 * @param datatype the constant
 * @return a string with a string representation
 */
const char *mknn_datatype_toString(const MknnDatatype datatype);

/**
 *
 * @param string the string to read and parse a datatype. The string should be a value returned by #mknn_datatype_toString.
 * @param out_datatype the parsed value
 * @return true if some value was assigned to out_datatype
 */
bool mknn_datatype_parseString(const char *string, MknnDatatype *out_datatype);

#ifdef __cplusplus
}
#endif

#endif
