/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DOMAIN_H_
#define MKNN_DOMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../metricknn_c.h"

/**
 * A MknnDomain represents the type of object that are contained in a dataset.
 *
 * A general domain represents a broad type of object. Currently, MetricKnn supports
 * two general domains: Strings and Vectors. In the case of vectors a domain considers
 * the number of dimensions and its datatype.
 *
 * In order to avoid memory leaks, each created MknnDomain* must call the
 * function #mknn_domain_release.
 *
 * The declaration of object domain is needed in order to use some of the pre-defined
 * distances. The domain enables to cast the <tt>void*</tt> object into other more meaningful type.
 * If the objects in a dataset will be compared with some user-defined distance, then
 * there is no need to declare the domain of objects.
 *
 *  @file
 */

/**
 * @name Specific functions for the general domain String.
 * @{
 */
/**
 * Creates a new domain object that represents strings.
 * The objects will be casted to <tt>char*</tt>.
 * As usual in C, the end of each string is given by <tt>char '\0'</tt>.
 *
 * @return a new domain (it must be released with #mknn_domain_release).
 */
MknnDomain *mknn_domain_newString();

/**
 * @}
 */

/**
 * @name Specific functions for the general domain Vector.
 * @{
 */
/**
 * Creates a new domain object that represents a vector of some fixed dimension and datatype.
 * The objects will be casted to <tt>datatype*</tt>. See @c MKNN_DATATYPE_xxx constants.
 *
 * @param num_dimensions number of dimensions
 * @param dimension_datatype datatype for the vector values. Must be one of the constants @c MKNN_DATATYPE_xxx.
 * @return a new domain (it must be released with #mknn_domain_release).
 */
MknnDomain *mknn_domain_newVector(int64_t num_dimensions,
		MknnDatatype dimension_datatype);

/**
 * Returns the number of dimensions of the vectors.
 * If the general domain is not #MKNN_GENERAL_DOMAIN_VECTOR the method returns 0.
 * @param domain
 * @return the number of dimensions of the domain.
 */
int64_t mknn_domain_vector_getNumDimensions(MknnDomain *domain);

/**
 * Returns the dimension datatype of the vectors.
 * If the general domain is not #MKNN_GENERAL_DOMAIN_VECTOR the method returns NULL.
 * @param domain
 * @return the datatype for the vectors.
 */
MknnDatatype mknn_domain_vector_getDimensionDataType(MknnDomain *domain);

/**
 * Creates an array with the size to store consecutive num_vectors.
 * @param domain
 * @return a pointer to a new allocated array (it must be released with free).
 */
void *mknn_domain_vector_createNewEmptyVectors(MknnDomain *domain,
		int64_t num_vectors);

/**
 *
 * @param domain
 * @param vectors_array
 * @param pos_vector
 */
void *mknn_domain_vector_getVectorInArray(MknnDomain *domain,
		void *vectors_array, int64_t pos_vector);

/**
 * Returns total size in bytes for one vector, which corresponds to <tt>dimensions * sizeof(datatype)</tt>, where
 * the size of a datatype is given by #mknn_datatype_sizeof.
 * If the general domain is not #MKNN_GENERAL_DOMAIN_VECTOR the method returns 0.
 * @param domain
 * @return the total size in bytes for one vector.
 */
int64_t mknn_domain_vector_getVectorLengthInBytes(MknnDomain *domain);

/**
 * @}
 */

/**
 * @name Specific functions for the general domain Multi-Object.
 * @{
 */
/**
 * Creates a new domain object that represents a multi-object, i.e., the combination of
 * many objects in an array.
 * The objects will be casted to <tt>void**</tt>. The size of the multi-object corresponds
 * to the number of objects that are combined.
 *
 * @param length the number of objects that are combined in a single array.
 * @param subdomains the domain of each object to be combined.
 * @param free_subdomains_on_domain_release to release all subdomains during the new domain release.
 * @return a multi-object domain.
 */
MknnDomain *mknn_domain_newMultiobject(int64_t length, MknnDomain **subdomains,
bool free_subdomains_on_domain_release);

/**
 * The number of objects in the multi-object, i.e., the size of the <tt>void**</tt> array.
 *
 * @param domain
 * @return the number of objects combined in the multi-object.
 */
int64_t mknn_domain_multiobject_getLength(MknnDomain *domain);

/**
 * The domain of each object in the multi-object.
 *
 * @param domain
 * @param num_subdomain the number of the subdomain to retrieve. A number between zero and
 * #mknn_domain_multiobject_getLength - 1.
 * @return the domain that describes each object in the multi-object.
 */
MknnDomain *mknn_domain_multiobject_getSubDomain(MknnDomain *domain,
		int64_t num_subdomain);

/**
 * @}
 */

/**
 * @name Specific functions for the general domain Custom.
 * @{
 */
/**
 * Creates a new domain object that represents a multi-object, i.e., the combination of
 * many objects in an array.
 *
 * @param custom_id a user-defined parameter
 * @param custom_data a user-defined parameter
 * @return the new domain
 */
MknnDomain *mknn_domain_newCustomObject(int64_t custom_id, void *custom_data);
/**
 *
 * @param domain
 * @return the parameter @c custom_id defined in the constructor
 */
int64_t mknn_domain_custom_getId(MknnDomain *domain);
/**
 *
 * @param domain
 * @return the parameter @c custom_data defined in the constructor
 */
void *mknn_domain_custom_getData(MknnDomain *domain);

/**
 * @}
 */
/**
 * Returns the general domain of the given domain object.
 * @param domain
 * @return the constant @c MKNN_DOMAIN_xxx that represents the general domain of @p domain.
 */
MknnGeneralDomain mknn_domain_getGeneralDomain(MknnDomain *domain);

/**
 * Returns true if the general domain is string.
 * test if #mknn_domain_getGeneralDomain returns the constant #MKNN_GENERAL_DOMAIN_STRING.
 *
 * @param domain
 * @return true if general domain of @p domain is #MKNN_GENERAL_DOMAIN_STRING.
 */
bool mknn_domain_isGeneralDomainString(MknnDomain *domain);

/**
 * Returns true if the general domain is vector.
 * test if #mknn_domain_getGeneralDomain returns the constant #MKNN_GENERAL_DOMAIN_VECTOR.
 *
 * @param domain
 * @return true if general domain of @p domain is #MKNN_GENERAL_DOMAIN_VECTOR.
 */
bool mknn_domain_isGeneralDomainVector(MknnDomain *domain);

/**
 * Returns true if the general domain is multi-object.
 * test if #mknn_domain_getGeneralDomain returns the constant #MKNN_GENERAL_DOMAIN_MULTIOBJECT.
 *
 * @param domain
 * @return true if general domain of @p domain is #MKNN_GENERAL_DOMAIN_MULTIOBJECT.
 */
bool mknn_domain_isGeneralDomainMultiObject(MknnDomain *domain);

/**
 * Returns true if the general domain is custom-object.
 * test if #mknn_domain_getGeneralDomain returns the constant #MKNN_GENERAL_DOMAIN_CUSTOMOBJECT.
 *
 * @param domain
 * @return true if general domain of @p domain is #MKNN_GENERAL_DOMAIN_CUSTOMOBJECT.
 */
bool mknn_domain_isGeneralDomainCustomObject(MknnDomain *domain);
/**
 * Two domains are equal if they coincide in their general domains and
 * all their attributes.
 * e.g., in the general domain vector:
 *  @li <tt>128-double</tt> and <tt>128-double</tt> are equal.
 *  @li <tt>128-double</tt> and <tt>128-uint8</tt> are NOT equal.
 *  @li <tt>128-double</tt> and <tt>100-double</tt> are NOT equal.
 *
 * @param domain1
 * @param domain2
 * @return true if both domains are equal, false otherwise.
 */
bool mknn_domain_testEqual(MknnDomain *domain1, MknnDomain *domain2);

/**
 * Two domains are equalExceptDatatype if they coincide in their general domains and
 * all their attributes EXCEPT in datatypes.
 * e.g., in the general domain vector:
 *  @li <tt>128-double</tt> and <tt>128-double</tt> are equalExceptDatatype.
 *  @li <tt>128-double</tt> and <tt>128-uint8</tt> are equalExceptDatatype.
 *  @li <tt>128-double</tt> and <tt>100-double</tt> are NOT equalExceptDatatype.
 *
 * @param domain1
 * @param domain2
 * @return true if both domains are equal without considering datatypes, false otherwise.
 */
bool mknn_domain_testEqualExceptDatatype(MknnDomain *domain1,
		MknnDomain *domain2);

/**
 * Creates a text representation of the domain and its attributes.
 *
 * @param domain
 * @return a string representation of the domain
 */
char *mknn_domain_toString(MknnDomain *domain);

/**
 * Creates a new domain object by parsing a string representation generated
 * by #mknn_domain_toString.
 *
 * @param string_domain a string representation of a domain.
 * @return a new domain (it must be released with #mknn_domain_release).
 */
MknnDomain *mknn_domain_newParseString(const char *string_domain);

/**
 * Duplicates a domain.
 *
 * @param domain the original domain
 * @return a new domain equal to @p domain (it must be released with #mknn_domain_release).
 */
MknnDomain *mknn_domain_newClone(MknnDomain *domain);

/**
 * Releases the memory used by the domain.
 * The released object becomes invalid and it must not be read.
 * In order to avoid memory leaks, each created MknnDomain* must call the
 * function #mknn_domain_release.
 *
 * @param domain
 */
void mknn_domain_release(MknnDomain *domain);

#ifdef __cplusplus
}
#endif

#endif
