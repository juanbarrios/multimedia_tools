/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DATATYPE_HPP_
#define MKNN_DATATYPE_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 * Static methods for datatype constants
 */
class Datatype {
public:
	/**
	 * Integer  8 bits-length signed. Corresponds to @c int8_t (see stdint.h).
	 */
	static const std::string SIGNED_INTEGER_8bits;
	/**
	 * Integer 16 bits-length signed. Corresponds to @c int16_t (see stdint.h).
	 */
	static const std::string SIGNED_INTEGER_16bits;
	/**
	 * Integer 32 bits-length signed. Corresponds to @c int32_t (see stdint.h).
	 */
	static const std::string SIGNED_INTEGER_32bits;
	/**
	 * Integer 64 bits-length signed. Corresponds to @c long long (see stdint.h).
	 */
	static const std::string SIGNED_INTEGER_64bits;

	/**
	 * Integer  8 bits-length unsigned. Corresponds to  @c uint8_t (see stdint.h).
	 */
	static const std::string UNSIGNED_INTEGER_8bits;
	/**
	 * Integer 16 bits-length unsigned. Corresponds to @c uint16_t (see stdint.h).
	 */
	static const std::string UNSIGNED_INTEGER_16bits;
	/**
	 * Integer 32 bits-length unsigned. Corresponds to @c uint32_t (see stdint.h).
	 */
	static const std::string UNSIGNED_INTEGER_32bits;
	/**
	 * Integer 64 bits-length unsigned. Corresponds to @c ulong long (see stdint.h).
	 */
	static const std::string UNSIGNED_INTEGER_64bits;

	/**
	 * Floating point 32 bits-length. Corresponds to standard C type @c float.
	 */
	static const std::string FLOATING_POINT_32bits;

	/**
	 * Floating point 64 bits-length. Corresponds to standard C type @c double.
	 */
	static const std::string FLOATING_POINT_64bits;

	/**
	 * Returns the size in bytes of the given datatype (a number between 1 and 8).
	 * @param datatype one of the constants
	 * @return size in bytes of the given datatype.
	 */
	static long long getNumBytes(const std::string datatype);

	/**
	 * Returns whether the given datatype is one of the signed integer types.
	 * @param datatype one of the constants
	 * @return true if datatype is
	 * #SIGNED_INTEGER_8bits or
	 * #SIGNED_INTEGER_16bits or
	 * #SIGNED_INTEGER_32bits or
	 * #SIGNED_INTEGER_64bits, false otherwise.
	 */
	static bool isSignedInteger(const std::string datatype);

	/**
	 * Returns whether the given datatype is one of the unsigned integer types.
	 * @param datatype one of the constants
	 * @return true if datatype is
	 * #UNSIGNED_INTEGER_8bits or
	 * #UNSIGNED_INTEGER_16bits or
	 * #UNSIGNED_INTEGER_32bits or
	 * #UNSIGNED_INTEGER_64bits, false otherwise.
	 */
	static bool isUnsignedInteger(const std::string datatype);

	/**
	 * Returns whether the given datatype is one of the floating point types.
	 * @param datatype one of the constants
	 * @return true if datatype is
	 * #FLOATING_POINT_32bits or
	 * #FLOATING_POINT_64bits, false otherwise.
	 */
	static bool isFloatingPoint(const std::string datatype);

};
}
#endif
