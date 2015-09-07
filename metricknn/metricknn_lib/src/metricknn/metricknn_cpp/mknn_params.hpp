/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PARAMS_HPP_
#define MKNN_PARAMS_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 *  Stores parameters in an internal map, which associates a names with its value.
 *  The internal map only stores two type of objects: strings and void pointers. The getter and setter methods
 *  that use integer/double/boolean actually convert them to/from a string.
 */
class DistanceParams {
public:
	/**
	 * Creates an empty object.
	 * @return a new empty object.
	 */
	static DistanceParams newEmpty();

	/**
	 * Creates an empty object and set parameters by calling method #parseString
	 * @param parameters_string the string with parameters.
	 * @return a parameters object
	 */
	static DistanceParams newParseString(std::string parameters_string);

	/**
	 * Parses a string in the format:
	 *  <tt><em>ID_DISTANCE</em>,<em>parameter1</em>=<em>value1</em>,<em>parameter2</em>=<em>value2</em></tt>
	 *
	 * Each parsed parameter is added to @p params.
	 *
	 * For example:
	 * @li <tt>LP,order=0.5</tt>
	 * @li <tt>DPF,order=1,pct_discard=0.1</tt>
	 *
	 * The list of valid IDs can be listed by invoking PredefDistance::helpListDistances.
	 *
	 * @param parameters_string the string with parameters.
	 */
	void parseString(std::string parameters_string);

	/**
	 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
	 * @param params_from the parameters from which the values are read.
	 */
	void copyAll(DistanceParams &params_from);

	/**
	 * Returns a string with the parameters which can be parsed.
	 * @return a string representation of parameters.
	 */
	std::string toString();

	/**
	 * @}
	 */

	/**
	 * @name MknnDistanceParams Add Methods
	 * @{
	 */
	/**
	 * Adds a string to the parameters.
	 * A copy of @p name and @p value are stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addString(std::string name, std::string value);

	/**
	 * Adds a floating point number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addDouble(std::string name, double value);

	/**
	 * Adds an integer number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addInt(std::string name, long long value);

	/**
	 * Adds an boolean value to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addBool(std::string name, bool value);

	/**
	 * Adds an object number to the parameters.
	 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
	 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addObject(std::string name, void *value);
	/**
	 * @}
	 */

	/**
	 * @name MknnDistanceParams Get Methods
	 * @{
	 */

	/**
	 * Returns a string value.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored string (it must not be modified).
	 */
	std::string getString(std::string name);
	/**
	 * Returns a floating-point number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	double getDouble(std::string name);

	/**
	 * Returns an integer number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	long long getInt(std::string name);

	/**
	 * Returns a boolean value.
	 *
	 * @param name the name of the parameter.
	 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
	 */
	bool getBool(std::string name);

	/**
	 * Returns a object reference.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored object.
	 */
	void *getObject(std::string name);

	/**
	 * @}
	 */

	/**
	 * @name MknnDistanceParams Other Methods
	 * @{
	 */
	/**
	 *
	 * @param id_dist
	 */
	void setDistanceId(std::string id_dist);
	/**
	 *
	 * @return
	 */
	std::string getDistanceId();
	/**
	 * @}
	 */

	/**
	 * Default constructor.
	 */
	DistanceParams();
	/**
	 * Default destructor.
	 */
	virtual ~DistanceParams();
	/**
	 * Copy constructor.
	 */
	DistanceParams(const DistanceParams &other);
	/**
	 * Assignment operator.
	 */
	DistanceParams &operator=(const DistanceParams &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Distance;
	friend class PredefDistance;
};

/**
 *  Stores parameters in an internal map, which associates a names with its value.
 *  The internal map only stores two type of objects: strings and void pointers. The getter and setter methods
 *  that use integer/double/boolean actually convert them to/from a string.
 */
class IndexParams {
public:
	/**
	 * Creates an empty object.
	 * @return a new empty object.
	 */
	static IndexParams newEmpty();

	/**
	 * Creates an empty object and set parameters by calling method #parseString
	 * @param parameters_string the string in the given format.
	 * @return a parameters object
	 */
	static IndexParams newParseString(std::string parameters_string);

	/**
	 * Parses a string in the format:
	 *  <tt><em>ID_INDEX</em>,<em>parameter1</em>=<em>value1</em>,<em>parameter2</em>=<em>value2</em></tt>
	 *
	 * Each parsed parameters is added to @p params.
	 *
	 * The list of valid indexes and their IDs can be listed by invoking PredefIndex::helpListIndexes.
	 *
	 * @param parameters_string the string in the given format.
	 */
	void parseString(std::string parameters_string);

	/**
	 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
	 * @param params_from the parameters from which the values are read.
	 */
	void copyAll(IndexParams &params_from);

	/**
	 * Returns a string with the parameters which can be parsed.
	 * @return a string representation of parameters.
	 */
	std::string toString();

	/**
	 * @name IndexParams Add Methods
	 * @{
	 */
	/**
	 * Adds a string to the parameters.
	 * A copy of @p name and @p value are stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addString(std::string name, std::string value);

	/**
	 * Adds a floating point number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addDouble(std::string name, double value);

	/**
	 * Adds an integer number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addInt(std::string name, long long value);

	/**
	 * Adds an boolean value to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addBool(std::string name, bool value);

	/**
	 * Adds an object number to the parameters.
	 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
	 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addObject(std::string name, void *value);
	/**
	 * @}
	 */

	/**
	 * @name IndexParams Get Methods
	 * @{
	 */

	/**
	 * Returns a string value.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored string (it must not be modified).
	 */
	std::string getString(std::string name);
	/**
	 * Returns a floating-point number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	double getDouble(std::string name);

	/**
	 * Returns an integer number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	long long getInt(std::string name);

	/**
	 * Returns a boolean value.
	 *
	 * @param name the name of the parameter.
	 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
	 */
	bool getBool(std::string name);

	/**
	 * Returns a object reference.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored object.
	 */
	void *getObject(std::string name);

	/**
	 * @}
	 */

	/**
	 * @name IndexParams Other Methods
	 * @{
	 */
	/**
	 *
	 * @param id_index
	 */
	void setIndexId(std::string id_index);
	/**
	 *
	 * @return
	 */
	std::string getIndexId();

	/**
	 * @}
	 */
	/**
	 * Default constructor.
	 */
	IndexParams();
	/**
	 * Default destructor.
	 */
	virtual ~IndexParams();
	/**
	 * Copy constructor.
	 */
	IndexParams(const IndexParams &other);
	/**
	 * Assignment operator.
	 */
	IndexParams &operator=(const IndexParams &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Index;
	friend class PredefIndex;
};

/**
 *  Stores parameters in an internal map, which associates a names with its value.
 *  The internal map only stores two type of objects: strings and void pointers. The getter and setter methods
 *  that use integer/double/boolean actually convert them to/from a string.
 */
class ResolverParams {
public:

	/**
	 * Creates an empty object.
	 * @return a new empty object.
	 */
	static ResolverParams newEmpty();

	/**
	 * Creates an empty object and set parameters by calling methods:
	 * #setKnn, #setRange, #setMaxThreads, #parseString
	 *
	 * @param knn
	 * @param range
	 * @param max_threads
	 * @param parameters_string the string in the format <tt><em>parameter1</em>=<em>value1</em>,...</tt>
	 * @return a parameters object
	 */
	static ResolverParams newParseString(long long knn, double range,
			long long max_threads, std::string parameters_string);

	/**
	 * Parses a string in the format:
	 *  <tt><em>parameter1</em>=<em>value1</em>,<em>parameter2</em>=<em>value2</em></tt>
	 *
	 * Each parsed parameters is added to @c params
	 *
	 * @param parameters_string the string in the format <tt><em>parameter1</em>=<em>value1</em>,...</tt>
	 */
	void parseString(std::string parameters_string);

	/**
	 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
	 * @param params_from the parameters from which the values are read.
	 */
	void copyAll(ResolverParams &params_from);

	/**
	 * Returns a string with the parameters which can be parsed.
	 * @return a string representation of parameters.
	 */
	std::string toString();

	/**
	 * @name MknnResolverParams Add Methods
	 * @{
	 */
	/**
	 * Adds a string to the parameters.
	 * A copy of @p name and @p value are stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addString(std::string name, std::string value);

	/**
	 * Adds a floating point number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addDouble(std::string name, double value);

	/**
	 * Adds an integer number to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addInt(std::string name, long long value);

	/**
	 * Adds an boolean value to the parameters.
	 * A copy of @p name is stored in @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addBool(std::string name, bool value);

	/**
	 * Adds an object number to the parameters.
	 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
	 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
	 *
	 * @param name the name of the parameter.
	 * @param value the value of the parameter.
	 */
	void addObject(std::string name, void *value);
	/**
	 * @}
	 */

	/**
	 * @name MknnResolverParams Get Methods
	 * @{
	 */

	/**
	 * Returns a string value.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored string (it must not be modified).
	 */
	std::string getString(std::string name);
	/**
	 * Returns a floating-point number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	double getDouble(std::string name);

	/**
	 * Returns an integer number.
	 *
	 * @param name the name of the parameter.
	 * @return the stored number. If the stored value is not a number, it will raise an error.
	 */
	long long getInt(std::string name);

	/**
	 * Returns a boolean value.
	 *
	 * @param name the name of the parameter.
	 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
	 */
	bool getBool(std::string name);

	/**
	 * Returns a object reference.
	 *
	 * @param name the name of the parameter.
	 * @return a pointer to the stored object.
	 */
	void *getObject(std::string name);
	/**
	 * @}
	 */

	/**
	 * @name MknnResolverParams Other Methods
	 * @{
	 */
	/**
	 * Sets the K parameter for resolvers in k-NN searches.
	 *
	 * @param knn
	 */
	void setKnn(long long knn);
	/**
	 * Sets the range parameter for resolvers in range searches.
	 *
	 * @param range
	 */
	void setRange(double range);
	/**
	 * Sets the maximum number of threads to be used by a resolver.
	 *
	 * @param max_threads
	 */
	void setMaxThreads(long long max_threads);
	/**
	 *
	 * @return
	 */
	long long getKnn();
	/**
	 *
	 * @return
	 */
	long long getRange();
	/**
	 *
	 * @return
	 */
	long long getMaxThreads();

	/**
	 * @}
	 */
	/**
	 * Default constructor.
	 */
	ResolverParams();
	/**
	 * Default destructor.
	 */
	virtual ~ResolverParams();
	/**
	 * Copy constructor.
	 */
	ResolverParams(const ResolverParams &other);
	/**
	 * Assignment operator.
	 */
	ResolverParams &operator=(const ResolverParams &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Resolver;
	friend class Index;
	friend class PredefIndex;
};

}

#endif
