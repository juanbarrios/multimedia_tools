/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_PARAMS_H_
#define MKNN_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 *  A parameters object stores parameters in an internal map, which associates a names with its value.
 *
 *  The internal map only stores two type of objects: strings and void pointers. The getter and setter methods
 *  that use integer/double/boolean actually convert them to/from a string.
 *
 *  @file
 */

/**
 * @name MknnDistanceParams
 * @{
 */

/**
 * Creates an empty object.
 * @return a new empty object.
 */
MknnDistanceParams *mknn_distanceParams_newEmpty();

/**
 * Creates an empty object and set parameters by calling method #mknn_distanceParams_parseString
 * @param parameters_string the string with parameters.
 * @return a parameters object
 */
MknnDistanceParams *mknn_distanceParams_newParseString(
		const char *parameters_string);

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
 * The list of valid IDs can be listed by invoking #mknn_predefDistance_helpListDistances.
 *
 * @param params the parameters object
 * @param parameters_string the string with parameters.
 */
void mknn_distanceParams_parseString(MknnDistanceParams *params,
		const char *parameters_string);

/**
 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
 * @param params_to the parameters where the values are copies to
 * @param params_from the parameters from which the values are read.
 */
void mknn_distanceParams_copyAll(MknnDistanceParams *params_to,
		MknnDistanceParams *params_from);

/**
 * Releases the parameters object.
 *
 * @param params the parameters object to be released.
 */
void mknn_distanceParams_release(MknnDistanceParams *params);

/**
 * Returns a string with the parameters which can be parsed.
 * @param params the parameters object
 * @return a string representation of parameters. The string is released during mknn_distanceParams_release.
 */
const char *mknn_distanceParams_toString(MknnDistanceParams *params);

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
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_distanceParams_addString(MknnDistanceParams *params, const char *name,
		const char *value);

/**
 * Adds a floating point number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_distanceParams_addDouble(MknnDistanceParams *params, const char *name,
		double value);

/**
 * Adds an integer number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_distanceParams_addInt(MknnDistanceParams *params, const char *name,
		int64_t value);

/**
 * Adds an boolean value to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_distanceParams_addBool(MknnDistanceParams *params, const char *name,
bool value);

/**
 * Adds an object number to the parameters.
 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_distanceParams_addObject(MknnDistanceParams *params, const char *name,
		void *value);
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
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored string (it must not be modified).
 */
const char *mknn_distanceParams_getString(MknnDistanceParams *params,
		const char *name);
/**
 * Returns a floating-point number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
double mknn_distanceParams_getDouble(MknnDistanceParams *params,
		const char *name);

/**
 * Returns an integer number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
int64_t mknn_distanceParams_getInt(MknnDistanceParams *params, const char *name);

/**
 * Returns a boolean value.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
 */
bool mknn_distanceParams_getBool(MknnDistanceParams *params, const char *name);

/**
 * Returns a object reference.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored object.
 */
void *mknn_distanceParams_getObject(MknnDistanceParams *params,
		const char *name);

/**
 * @}
 */

/**
 * @name MknnDistanceParams Other Methods
 * @{
 */

/**
 *
 * @param p
 * @param id_dist
 */
void mknn_distanceParams_setDistanceId(MknnDistanceParams *p,
		const char *id_dist);

/**
 *
 * @param p
 * @return
 */
const char *mknn_distanceParams_getDistanceId(MknnDistanceParams *p);
/**
 * @}
 */

/**
 * @name MknnIndexParams
 * @{
 */
/**
 * Creates an empty object.
 * @return a new empty object.
 */
MknnIndexParams *mknn_indexParams_newEmpty();

/**
 * Creates an empty object and set parameters by calling method #mknn_indexParams_parseString
 * @param parameters_string the string in the given format.
 * @return a parameters object
 */
MknnIndexParams *mknn_indexParams_newParseString(const char *parameters_string);

/**
 * Parses a string in the format:
 *  <tt><em>ID_INDEX</em>,<em>parameter1</em>=<em>value1</em>,<em>parameter2</em>=<em>value2</em></tt>
 *
 * Each parsed parameters is added to @p params.
 *
 * The list of valid indexes and their IDs can be listed by invoking #mknn_predefIndex_helpListIndexes.
 *
 * @param params the parameters object
 * @param parameters_string the string in the given format.
 */
void mknn_indexParams_parseString(MknnIndexParams *params,
		const char *parameters_string);

/**
 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
 * @param params_to the parameters where the values are copies to
 * @param params_from the parameters from which the values are read.
 */
void mknn_indexParams_copyAll(MknnIndexParams *params_to,
		MknnIndexParams *params_from);

/**
 * Releases the parameters object.
 *
 * @param params the parameters object to be released.
 */
void mknn_indexParams_release(MknnIndexParams *params);

/**
 * Returns a string with the parameters which can be parsed.
 * @param params the parameters object
 * @return a string representation of parameters. The string is released during mknn_indexParams_release.
 */
const char *mknn_indexParams_toString(MknnIndexParams *params);
/**
 * @}
 */

/**
 * @name MknnIndexParams Add Methods
 * @{
 */
/**
 * Adds a string to the parameters.
 * A copy of @p name and @p value are stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_indexParams_addString(MknnIndexParams *params, const char *name,
		const char *value);

/**
 * Adds a floating point number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_indexParams_addDouble(MknnIndexParams *params, const char *name,
		double value);

/**
 * Adds an integer number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_indexParams_addInt(MknnIndexParams *params, const char *name,
		int64_t value);

/**
 * Adds an boolean value to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_indexParams_addBool(MknnIndexParams *params, const char *name,
bool value);

/**
 * Adds an object number to the parameters.
 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_indexParams_addObject(MknnIndexParams *params, const char *name,
		void *value);
/**
 * @}
 */

/**
 * @name MknnIndexParams Get Methods
 * @{
 */

/**
 * Returns a string value.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored string (it must not be modified).
 */
const char *mknn_indexParams_getString(MknnIndexParams *params,
		const char *name);
/**
 * Returns a floating-point number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
double mknn_indexParams_getDouble(MknnIndexParams *params, const char *name);

/**
 * Returns an integer number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
int64_t mknn_indexParams_getInt(MknnIndexParams *params, const char *name);

/**
 * Returns a boolean value.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
 */
bool mknn_indexParams_getBool(MknnIndexParams *params, const char *name);

/**
 * Returns a object reference.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored object.
 */
void *mknn_indexParams_getObject(MknnIndexParams *params, const char *name);

/**
 * @}
 */

/**
 * @name MknnIndexParams Other Methods
 * @{
 */
/**
 *
 * @param p
 * @param id_index
 */
void mknn_indexParams_setIndexId(MknnIndexParams *p, const char *id_index);

/**
 *
 * @param p
 * @return
 */
const char *mknn_indexParams_getIndexId(MknnIndexParams *p);

/**
 * @}
 */

/**
 * @name MknnResolverParams
 * @{
 */

/**
 * Creates an empty object.
 * @return a new empty object.
 */
MknnResolverParams *mknn_resolverParams_newEmpty();

/**
 * Creates an empty object and set parameters by calling methods:
 * #mknn_resolverParams_setKnn, #mknn_resolverParams_setRange, #mknn_resolverParams_setMaxThreads, #mknn_resolverParams_parseString
 *
 * @param knn
 * @param range
 * @param max_threads
 * @param parameters_string the string in the format <tt><em>parameter1</em>=<em>value1</em>,...</tt>
 * @return a parameters object
 */
MknnResolverParams *mknn_resolverParams_newParseString(int64_t knn,
		double range, int64_t max_threads, const char *parameters_string);

/**
 * Parses a string in the format:
 *  <tt><em>parameter1</em>=<em>value1</em>,<em>parameter2</em>=<em>value2</em></tt>
 *
 * Each parsed parameters is added to @c params
 *
 *
 * @param params the parameters object
 * @param parameters_string the string in the format <tt><em>parameter1</em>=<em>value1</em>,...</tt>
 */
void mknn_resolverParams_parseString(MknnResolverParams *params,
		const char *parameters_string);

/**
 * Copies all the key-values pairs in @p params_from and adds them to @p params_to.
 * @param params_to the parameters where the values are copies to
 * @param params_from the parameters from which the values are read.
 */
void mknn_resolverParams_copyAll(MknnResolverParams *params_to,
		MknnResolverParams *params_from);

/**
 * Releases the parameters object.
 *
 * @param params the parameters object to be released.
 */
void mknn_resolverParams_release(MknnResolverParams *params);
/**
 * Returns a string with the parameters which can be parsed.
 * @param params the parameters object
 * @return a string representation of parameters. The string is released during mknn_resolverParams_release.
 */
const char *mknn_resolverParams_toString(MknnResolverParams *params);
/**
 * @}
 */

/**
 * @name MknnResolverParams Add Methods
 * @{
 */
/**
 * Adds a string to the parameters.
 * A copy of @p name and @p value are stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_resolverParams_addString(MknnResolverParams *params, const char *name,
		const char *value);

/**
 * Adds a floating point number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_resolverParams_addDouble(MknnResolverParams *params, const char *name,
		double value);

/**
 * Adds an integer number to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_resolverParams_addInt(MknnResolverParams *params, const char *name,
		int64_t value);

/**
 * Adds an boolean value to the parameters.
 * A copy of @p name is stored in @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_resolverParams_addBool(MknnResolverParams *params, const char *name,
bool value);

/**
 * Adds an object number to the parameters.
 * A copy of @p name is stored in @p params but the value pointer itself is stored in @p params.
 * Therefore the @p value pointer must remain valid during all the lifetime of @p params.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @param value the value of the parameter.
 */
void mknn_resolverParams_addObject(MknnResolverParams *params, const char *name,
		void *value);
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
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored string (it must not be modified).
 */
const char *mknn_resolverParams_getString(MknnResolverParams *params,
		const char *name);
/**
 * Returns a floating-point number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
double mknn_resolverParams_getDouble(MknnResolverParams *params,
		const char *name);

/**
 * Returns an integer number.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored number. If the stored value is not a number, it will raise an error.
 */
int64_t mknn_resolverParams_getInt(MknnResolverParams *params, const char *name);

/**
 * Returns a boolean value.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return the stored value. If the stored value is not a boolean value, it will raise an error.
 */
bool mknn_resolverParams_getBool(MknnResolverParams *params, const char *name);

/**
 * Returns a object reference.
 *
 * @param params parameters
 * @param name the name of the parameter.
 * @return a pointer to the stored object.
 */
void *mknn_resolverParams_getObject(MknnResolverParams *params,
		const char *name);
/**
 * @}
 */

/**
 * @name MknnResolverParams Other Methods
 * @{
 */
/**
 * Sets the K parameter for resolvers in k-NN searches.
 * @param params parameters
 * @param knn
 */
void mknn_resolverParams_setKnn(MknnResolverParams *params, int64_t knn);
/**
 * Sets the range parameter for resolvers in range searches.
 * @param params parameters
 * @param range
 */
void mknn_resolverParams_setRange(MknnResolverParams *params, double range);
/**
 * Sets the maximum number of threads to be used by a resolver.
 *
 * @param params parameters
 * @param max_threads
 */
void mknn_resolverParams_setMaxThreads(MknnResolverParams *params,
		int64_t max_threads);
/**
 *
 * @param params
 * @return
 */
int64_t mknn_resolverParams_getKnn(MknnResolverParams *params);
/**
 *
 * @param params
 * @return
 */
double mknn_resolverParams_getRange(MknnResolverParams *params);
/**
 *
 * @param params
 * @return
 */
int64_t mknn_resolverParams_getMaxThreads(MknnResolverParams *params);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
