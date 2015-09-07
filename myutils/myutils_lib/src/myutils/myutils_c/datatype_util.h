/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MY_DATATYPE_UTIL_H
#define MY_DATATYPE_UTIL_H

#include "../myutils_c.h"

/**
 * Integer  8 bits-length signed. Corresponds to @c int8_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_INT8;
/**
 * Integer 16 bits-length signed. Corresponds to @c int16_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_INT16;
/**
 * Integer 32 bits-length signed. Corresponds to @c int32_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_INT32;
/**
 * Integer 64 bits-length signed. Corresponds to @c int64_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_INT64;

/**
 * Integer  8 bits-length unsigned. Corresponds to  @c uint8_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_UINT8;
/**
 * Integer 16 bits-length unsigned. Corresponds to @c uint16_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_UINT16;
/**
 * Integer 32 bits-length unsigned. Corresponds to @c uint32_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_UINT32;
/**
 * Integer 64 bits-length unsigned. Corresponds to @c uint64_t (see stdint.h).
 */
extern MyDatatype MY_DATATYPE_UINT64;

/**
 * Floating point 32 bits-length. Corresponds to standard C type @c float.
 */
extern MyDatatype MY_DATATYPE_FLOAT32;
/**
 * Floating point 64 bits-length. Corresponds to standard C type @c double.
 */
extern MyDatatype MY_DATATYPE_FLOAT64;

/**
 * @param datatype
 * @return the sizeof(type)
 */
size_t my_datatype_sizeof(const MyDatatype datatype);
/**
 * @param datatype
 * @return true if datatype represents an integer signed or unsigned
 */
bool my_datatype_isAnyInteger(const MyDatatype datatype);
/**
 * @param datatype
 * @return true if datatype represents an integer signed
 */
bool my_datatype_isAnySignedInteger(const MyDatatype datatype);
/**
 * @param datatype
 * @return true if datatype represents an integer unsigned
 */
bool my_datatype_isAnyUnsignedInteger(const MyDatatype datatype);
/**
 * @param datatype
 * @return true if datatype represents a floating point number
 */
bool my_datatype_isAnyFloatingPoint(const MyDatatype datatype);

bool my_datatype_isInt8(const MyDatatype datatype);
bool my_datatype_isInt16(const MyDatatype datatype);
bool my_datatype_isInt32(const MyDatatype datatype);
bool my_datatype_isInt64(const MyDatatype datatype);
bool my_datatype_isUInt8(const MyDatatype datatype);
bool my_datatype_isUInt16(const MyDatatype datatype);
bool my_datatype_isUInt32(const MyDatatype datatype);
bool my_datatype_isUInt64(const MyDatatype datatype);
bool my_datatype_isFloat(const MyDatatype datatype);
bool my_datatype_isDouble(const MyDatatype datatype);

bool my_datatype_areEqual(const MyDatatype datatype1,
		const MyDatatype datatype2);

const char *my_datatype_codeToDescription(const MyDatatype datatype);
MyDatatype my_datatype_descriptionToCode(const char *description);

/**
 * Function that creates a string from an array.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatype *vector = ptr_vector;
 * char *st = malloc(...);
 * sprintf(st, "%...", vector[pos]);
 * return st;
 * @endcode
 *
 * The format given to <tt>sprintf</tt> depends on the datatype.
 *
 * @param array the array to print
 * @param array_length the size of the array
 * @param prefix
 * @param separator
 * @param suffix
 * @return a new string containing the value (must be freed).
 */
typedef char *(*my_function_to_string)(void *array, size_t array_length,
		const char *prefix, const char *separator, const char *suffix);

/**
 * Function that parse a string at stores in the given position of a vector.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatype *vector = ptr_vector;
 * vector[pos] = (datatype) strtoXXX(string);
 * @endcode
 *
 * The function <tt>strtoXXX</tt> depends on the datatype (e.g., it may be <tt>strtoll</tt> or <tt>strtod</tt>).
 *
 * @param string a text containing one value
 * @param ptr_vector the vector to store the parsed value
 * @param pos the position of the vector to store the parsed value.
 */
typedef void (*my_function_parse_string)(const char *string, void *ptr_vector,
		size_t pos);

/**
 * Function that can copy vectors between any two datatypes.
 * The conversion between different datatypes is resolved by casting values.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatypeSrc *vector_src = ptr_vector_src;
 * datatypeDst *vector_dst = ptr_vector_dst;
 * for(i = 0; i < dimensions; ++i)
 *     vector_dst[i] = (datatypeDst) vector_src[i];
 * @endcode
 *
 * @param ptr_vector_src pointer to source array.
 * @param ptr_vector_dst pointer to destiny array.
 * @param dimensions the number of values to copy from @c ptr_vector_src to @c ptr_vector_dst.
 */
typedef void (*my_function_copy_vector)(void *ptr_vector_src,
		void *ptr_vector_dst, size_t dimensions);

/**
 * Returns a function that can print an array of any value.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatype *vector = ptr_vector;
 * char *st = malloc(...);
 * sprintf(st, "%...", vector[pos]);
 * return st;
 * @endcode
 *
 * The format given to <tt>sprintf</tt> depends on the datatype.
 *
 * @param datatype constant @c MKNN_DATATYPE_xxx for the value to print to a string.
 * @return a function that can print values from @c datatype to a string.
 */
my_function_to_string my_datatype_getFunctionToString(
		const MyDatatype datatype);

/**
 * Returns a function that can read a string and parse a value of the given datatype.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatype *vector = ptr_vector;
 * vector[pos] = (datatype) strtoXXX(string);
 * @endcode
 *
 * The function <tt>strtoXXX</tt> depends on the datatype (e.g., it may be <tt>strtoll</tt> or <tt>strtod</tt>).
 *
 * @param datatype constant @c MKNN_DATATYPE_xxx for the value to read from a string.
 * @return a function that can parse values from @c datatype.
 */
my_function_parse_string my_datatype_getFunctionParseVector(
		const MyDatatype datatype);

/**
 * Returns a function that can copy values between any two datatypes.
 * The conversion between different datatypes is resolved by casting values.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatypeSrc *vector_src = ptr_vector_src;
 * datatypeDst *vector_dst = ptr_vector_dst;
 * for(i = 0; i < dimensions; ++i)
 *     vector_dst[i] = (datatypeDst) vector_src[i];
 * @endcode
 *
 * @param datatype_src constant @c MKNN_DATATYPE_xxx of the datatype of the source.
 * @param datatype_dst constant @c MKNN_DATATYPE_xxx of the datatype of the destiny.
 * @return a function that can copy values from @c datatype_src to  @c datatype_dst.
 */
my_function_copy_vector my_datatype_getFunctionCopyVector(
		const MyDatatype datatype_src, const MyDatatype datatype_dst);

/**
 * Function that convert numbers, e.g., fabs, sqrt, sin, cos, and many others in math.h.
 *
 * @param input_value an number
 * @returns an output numbers
 */
typedef double (*my_function_number_operator)(double input_value);

typedef void (*my_function_copyOperate_vector)(void *ptr_vector_src,
		void *ptr_vector_dst, size_t dimensions,
		my_function_number_operator funcOperate);

/**
 * Returns a function that can copy values between any two datatypes.
 * The conversion between different datatypes is resolved by casting values.
 *
 * Basically, the implementation is the following (without considering any validation):
 *
 * @code
 * datatypeSrc *vector_src = ptr_vector_src;
 * datatypeDst *vector_dst = ptr_vector_dst;
 * for(i = 0; i < dimensions; ++i)
 *     vector_dst[i] = (datatypeDst) vector_src[i];
 * @endcode
 *
 * @param datatype_src constant @c MKNN_DATATYPE_xxx of the datatype of the source.
 * @param datatype_dst constant @c MKNN_DATATYPE_xxx of the datatype of the destiny.
 * @return a function that can copy values from @c datatype_src to  @c datatype_dst.
 */
my_function_copyOperate_vector my_datatype_getFunctionCopyOperateVector(
		const MyDatatype datatype_src, const MyDatatype datatype_dst);

#endif
