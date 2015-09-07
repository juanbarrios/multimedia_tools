/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MKNN_DOMAIN_HPP_
#define MKNN_DOMAIN_HPP_

#include "../metricknn_cpp.hpp"

namespace mknn {

/**
 * A domain represents the type of object that are contained in a dataset.
 *
 * A general domain represents a broad type of object. Currently, MetricKnn supports
 * two general domains: Strings and Vectors. In the case of vectors a domain considers
 * the number of dimensions and its datatype.
 *
 * In order to avoid memory leaks, each created MknnDomain* must call the
 * function delete.
 *
 * The declaration of object domain is needed in order to use some of the pre-defined
 * distances. The domain enables to cast the <tt>void*</tt> object into other more meaningful type.
 * If the objects in a dataset will be compared with some user-defined distance, then
 * there is no need to declare the domain of objects.
 *
 */

class Domain {
public:
	/**
	 * General Domain of Strings.
	 * The objects will be casted to <tt>char*</tt>.
	 * As usual in C, the end of each string is given by <tt>char '\0'</tt>.
	 */
	static const std::string GENERAL_DOMAIN_STRING;

	/**
	 * General Domain of Vectors.
	 * In order to declare a domain the number of dimensions and datatype are needed.
	 * The objects will be casted to <tt>datatype*</tt>.
	 */
	static const std::string GENERAL_DOMAIN_VECTOR;

	/**
	 * General Domain of Multi-Object.
	 * The objects will be casted to <tt>void**</tt> (an array of objects).
	 */
	static const std::string GENERAL_DOMAIN_MULTIOBJECT;

	/**
	 * General Domain of Custom Objects.
	 * The custom objects are not casted, thus they cannot be saved or printed.
	 */
	static const std::string GENERAL_DOMAIN_CUSTOMOBJECT;

	/**
	 * @name Specific functions for the general domain String.
	 * @{
	 */
	/**
	 * Creates a new domain object that represents strings.
	 * The objects will be casted to <tt>char*</tt>.
	 * As usual in C, the end of each string is given by <tt>char '\0'</tt>.
	 *
	 * @return a new domain (it must be released with delete).
	 */
	static Domain newString();

	/**
	 * @}
	 */

	/**
	 * @name Specific functions for the general domain Vector.
	 * @{
	 */
	/**
	 * Creates a new domain object that represents a vector of some fixed dimension and datatype.
	 * The objects will be casted to <tt>datatype*</tt>. See constants.
	 *
	 * @param num_dimensions number of dimensions
	 * @param dimension_datatype datatype for the vector values. Must be one of the constants.
	 * @return a new domain (it must be released with delete).
	 */
	static Domain newVector(long long num_dimensions,
			std::string dimension_datatype);

	/**
	 * Returns the number of dimensions of the vectors.
	 * If the general domain is not #GENERAL_DOMAIN_VECTOR the method returns 0.
	 * @return the number of dimensions of the domain.
	 */
	long long getVectorNumDimensions();

	/**
	 * Returns the dimension datatype of the vectors.
	 * If the general domain is not @c GENERAL_DOMAIN_VECTOR the method returns NULL.
	 * @return the datatype for the vectors.
	 */
	std::string getVectorDimensionDataType();

	/**
	 * Returns total size in bytes for one vector, which corresponds to <tt>dimensions * sizeof(datatype)</tt>, where
	 * the size of a datatype is given by Datatype::getNumBytes.
	 * If the general domain is not #GENERAL_DOMAIN_VECTOR the method returns 0.
	 * @return the total size in bytes for one vector.
	 */
	long long getVectorSizeInBytes();
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
	 * @param subdomains the domain of each object to be combined.
	 * @param delete_subdomains_on_domain_release binds the lifetime of subdomains to the new domain
	 * @return a multi-object domain.
	 */
	static Domain newMultiobject(const std::vector<Domain> &subdomains);

	/**
	 * The number of objects in the multi-object, i.e., the size of the <tt>void**</tt> array.
	 *
	 * @return the number of objects combined in the multi-object.
	 */
	long long getMultiobjectLength();

	/**
	 * The domain of each object in the multi-object.
	 *
	 * @param num_subdomain the number of the subdomain to retrieve. A number between zero and
	 * #getMultiobjectLength - 1.
	 * @return the domain that describes each object in the multi-object.
	 */
	Domain &getMultiobjectSubDomain(long long num_subdomain);

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
	static Domain newCustomObject(long long custom_id, void *custom_data);
	/**
	 *
	 * @return the parameter @c custom_id defined in the constructor
	 */
	long long getCustomObjectId();
	/**
	 *
	 * @return the parameter @c custom_data defined in the constructor
	 */
	void *getCustomObjectData();

	/**
	 * @}
	 */

	/**
	 * Returns the general domain of the given domain object.
	 * @return the constant @c MKNN_DOMAIN_xxx that represents the general domain of @p domain.
	 */
	std::string getGeneralDomain();

	/**
	 * Returns true if the general domain is string.
	 * test if #getGeneralDomain returns the constant GENERAL_DOMAIN_STRING.
	 *
	 * @return true if general domain of @p domain is GENERAL_DOMAIN_STRING.
	 */
	bool isGeneralDomainString();

	/**
	 * Returns true if the general domain is vector.
	 * test if #getGeneralDomain returns the constant GENERAL_DOMAIN_VECTOR.
	 *
	 * @return true if general domain of @p domain is GENERAL_DOMAIN_VECTOR.
	 */
	bool isGeneralDomainVector();

	/**
	 * Returns true if the general domain is multi-object.
	 * test if #getGeneralDomain returns the constant GENERAL_DOMAIN_MULTIOBJECT.
	 *
	 * @return true if general domain is GENERAL_DOMAIN_MULTIOBJECT.
	 */
	bool isGeneralDomainMultiObject();

	/**
	 * Returns true if the general domain is custom-object.
	 * test if #getGeneralDomain returns the constant GENERAL_DOMAIN_CUSTOMOBJECT.
	 *
	 * @return true if general domain is GENERAL_DOMAIN_CUSTOMOBJECT.
	 */
	bool isGeneralDomainCustomObject();

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
	static bool testEqual(Domain &domain1, Domain &domain2);

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
	 * @return true if both domains are compatible, false otherwise.
	 */
	static bool testEqualExceptDatatype(Domain &domain1, Domain &domain2);

	/**
	 * Creates a text representation of the domain and its attributes.
	 *
	 * @return a string representation of the domain
	 */
	std::string toString();

	/**
	 * Creates a new domain object by parsing a string representation generated
	 * by #toString.
	 *
	 * @param string_domain a string representation of a domain.
	 * @return a new domain (it must be released with delete).
	 */
	static Domain newParseString(std::string string_domain);

	/**
	 * Duplicates a domain.
	 *
	 * @param domain the original domain
	 * @return a new domain equal to @p domain (it must be released with delete).
	 */
	static Domain newClone(Domain domain);

	/**
	 * Default constructor.
	 */
	Domain();
	/**
	 * Default destructor.
	 */
	virtual ~Domain();
	/**
	 * Copy constructor.
	 */
	Domain(const Domain &other);
	/**
	 * Assignment operator.
	 */
	Domain &operator=(const Domain &other);

protected:
	/**
	 * opaque class
	 */
	class Impl;
	/**
	 * opaque object
	 */
	std::unique_ptr<Impl> pimpl;

	friend class Dataset;
	friend class DatasetLoader;
	friend class Distance;
	friend class Printer;
};

}
#endif
