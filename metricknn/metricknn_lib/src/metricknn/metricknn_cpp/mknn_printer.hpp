/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PRINTER_HPP_
#define MKNN_PRINTER_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 *  The object printer  converts an object from a given domain to its string representation.
 */

class Printer {
public:
	/**
	 * Creates an empty object.
	 *
	 * @param domain the domain of the objects
	 * @return a new printer object.
	 */
	static Printer newPrinter(Domain domain);

	/**
	 * Creates a new string
	 * @param object the object to convert
	 * @return a new string (must be freed)
	 */
	std::string objectToString(void *object);

	/**
	 * Changes the format for objects of the general domain vector.
	 *
	 * @param vector_prefix the string that is print in front of the vector.
	 * @param dimension_separator the string that is print between dimensions.
	 * @param vector_suffix the string that is print after the vector.
	 */
	void setVectorFormat(std::string vector_prefix,
			std::string dimension_separator, std::string vector_suffix);

	/**
	 * Changes the format for objects of the general domain multiobject.
	 *
	 * @param prefix the string that is print in front of the multiobject.
	 * @param separator the string that is print between objects.
	 * @param suffix the string that is print after the multiobject.
	 */
	void setMultiobjectFormat(std::string prefix, std::string separator,
			std::string suffix);

	/**
	 * Default constructor.
	 */
	Printer();
	/**
	 * Default destructor.
	 */
	virtual ~Printer();
	/**
	 * Copy constructor.
	 */
	Printer(const Printer &other);
	/**
	 * Assignment operator.
	 */
	Printer &operator=(const Printer &other);

protected:
	/**
	 * Internal opaque class
	 */
	class Impl;
	/**
	 * Internal opaque class
	 */
	std::unique_ptr<Impl> pimpl;
};

}
#endif
