/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

MknnGeneralDomain MKNN_GENERAL_DOMAIN_STRING = { 31 };
MknnGeneralDomain MKNN_GENERAL_DOMAIN_VECTOR = { 32 };
MknnGeneralDomain MKNN_GENERAL_DOMAIN_MULTIOBJECT = { 33 };
MknnGeneralDomain MKNN_GENERAL_DOMAIN_CUSTOMOBJECT = { 40 };

static const char *STRING_GENERAL_DOMAIN_STRING = "STRING";
static const char *STRING_GENERAL_DOMAIN_VECTOR = "VECTOR";
static const char *STRING_GENERAL_DOMAIN_MULTIOBJECT = "MULTIOBJECT";
static const char *STRING_GENERAL_DOMAIN_CUSTOMOBJECT = "CUSTOMOBJECT";

bool mknn_generalDomain_areEqual(const MknnGeneralDomain general_domain1,
		const MknnGeneralDomain general_domain2) {
	return general_domain1.mknn_domain_code == general_domain2.mknn_domain_code;
}
bool mknn_generalDomain_isString(const MknnGeneralDomain general_domain) {
	return general_domain.mknn_domain_code
			== MKNN_GENERAL_DOMAIN_STRING.mknn_domain_code;
}
bool mknn_generalDomain_isVector(const MknnGeneralDomain general_domain) {
	return general_domain.mknn_domain_code
			== MKNN_GENERAL_DOMAIN_VECTOR.mknn_domain_code;
}
bool mknn_generalDomain_isMultiObject(const MknnGeneralDomain general_domain) {
	return general_domain.mknn_domain_code
			== MKNN_GENERAL_DOMAIN_MULTIOBJECT.mknn_domain_code;
}
bool mknn_generalDomain_isCustom(const MknnGeneralDomain general_domain) {
	return general_domain.mknn_domain_code
			== MKNN_GENERAL_DOMAIN_CUSTOMOBJECT.mknn_domain_code;
}

#define RETURN_IF_EQ_DOM(var, test_constant, ret_constant) \
	if (var.mknn_domain_code == test_constant.mknn_domain_code)\
	return ret_constant;

const char *mknn_generalDomain_toString(const MknnGeneralDomain general_domain) {
	RETURN_IF_EQ_DOM(general_domain, MKNN_GENERAL_DOMAIN_STRING,
			STRING_GENERAL_DOMAIN_STRING)
	RETURN_IF_EQ_DOM(general_domain, MKNN_GENERAL_DOMAIN_VECTOR,
			STRING_GENERAL_DOMAIN_VECTOR)
	RETURN_IF_EQ_DOM(general_domain, MKNN_GENERAL_DOMAIN_MULTIOBJECT,
			STRING_GENERAL_DOMAIN_MULTIOBJECT)
	RETURN_IF_EQ_DOM(general_domain, MKNN_GENERAL_DOMAIN_CUSTOMOBJECT,
			STRING_GENERAL_DOMAIN_CUSTOMOBJECT)
	return "UNKNOWN";
}

#define ASSIGN_RETURN_IF_EQUAL_STR(var_test, var_assign, test_constant, assign_constant) \
	if (my_string_equals_ignorecase(var_test, test_constant)) { \
	var_assign = assign_constant; \
	return true; \
	}

bool mknn_generalDomain_parseString(const char *string,
		MknnGeneralDomain *out_general_domain) {
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_general_domain,
			STRING_GENERAL_DOMAIN_VECTOR, MKNN_GENERAL_DOMAIN_VECTOR)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_general_domain,
			STRING_GENERAL_DOMAIN_STRING, MKNN_GENERAL_DOMAIN_STRING)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_general_domain,
			STRING_GENERAL_DOMAIN_MULTIOBJECT, MKNN_GENERAL_DOMAIN_MULTIOBJECT)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_general_domain,
			STRING_GENERAL_DOMAIN_CUSTOMOBJECT,
			MKNN_GENERAL_DOMAIN_CUSTOMOBJECT)
	return false;
}

#define IN08 110
#define IN16 111
#define IN32 112
#define IN64 113

#define UN08 115
#define UN16 116
#define UN32 117
#define UN64 118

#define FL32 120
#define FL64 121

MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_8bits = { IN08 };
MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_16bits = { IN16 };
MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_32bits = { IN32 };
MknnDatatype MKNN_DATATYPE_SIGNED_INTEGER_64bits = { IN64 };

MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_8bits = { UN08 };
MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_16bits = { UN16 };
MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_32bits = { UN32 };
MknnDatatype MKNN_DATATYPE_UNSIGNED_INTEGER_64bits = { UN64 };

MknnDatatype MKNN_DATATYPE_FLOATING_POINT_32bits = { FL32 };
MknnDatatype MKNN_DATATYPE_FLOATING_POINT_64bits = { FL64 };

const char *STRING_DATATYPE_SIGNED_INTEGER_8bits = "INT8";
const char *STRING_DATATYPE_SIGNED_INTEGER_16bits = "INT16";
const char *STRING_DATATYPE_SIGNED_INTEGER_32bits = "INT32";
const char *STRING_DATATYPE_SIGNED_INTEGER_64bits = "INT64";

const char *STRING_DATATYPE_UNSIGNED_INTEGER_8bits = "UINT8";
const char *STRING_DATATYPE_UNSIGNED_INTEGER_16bits = "UINT16";
const char *STRING_DATATYPE_UNSIGNED_INTEGER_32bits = "UINT32";
const char *STRING_DATATYPE_UNSIGNED_INTEGER_64bits = "UINT64";

const char *STRING_DATATYPE_FLOATING_POINT_32bits = "FLOAT";
const char *STRING_DATATYPE_FLOATING_POINT_64bits = "DOUBLE";

const char *mknn_datatype_toString(const MknnDatatype datatype) {
	switch (datatype.mknn_datatype_code) {
	case IN08:
		return STRING_DATATYPE_SIGNED_INTEGER_8bits;
	case IN16:
		return STRING_DATATYPE_SIGNED_INTEGER_16bits;
	case IN32:
		return STRING_DATATYPE_SIGNED_INTEGER_32bits;
	case IN64:
		return STRING_DATATYPE_SIGNED_INTEGER_64bits;
	case UN08:
		return STRING_DATATYPE_UNSIGNED_INTEGER_8bits;
	case UN16:
		return STRING_DATATYPE_UNSIGNED_INTEGER_16bits;
	case UN32:
		return STRING_DATATYPE_UNSIGNED_INTEGER_32bits;
	case UN64:
		return STRING_DATATYPE_UNSIGNED_INTEGER_64bits;
	case FL32:
		return STRING_DATATYPE_FLOATING_POINT_32bits;
	case FL64:
		return STRING_DATATYPE_FLOATING_POINT_64bits;
	default:
		return "UNKNOWN";
	}
}

bool mknn_datatype_parseString(const char *string, MknnDatatype *out_datatype) {
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_SIGNED_INTEGER_8bits,
			MKNN_DATATYPE_SIGNED_INTEGER_8bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_SIGNED_INTEGER_16bits,
			MKNN_DATATYPE_SIGNED_INTEGER_16bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_SIGNED_INTEGER_32bits,
			MKNN_DATATYPE_SIGNED_INTEGER_32bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_SIGNED_INTEGER_64bits,
			MKNN_DATATYPE_SIGNED_INTEGER_64bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_UNSIGNED_INTEGER_8bits,
			MKNN_DATATYPE_UNSIGNED_INTEGER_8bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_UNSIGNED_INTEGER_16bits,
			MKNN_DATATYPE_UNSIGNED_INTEGER_16bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_UNSIGNED_INTEGER_32bits,
			MKNN_DATATYPE_UNSIGNED_INTEGER_32bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_UNSIGNED_INTEGER_64bits,
			MKNN_DATATYPE_UNSIGNED_INTEGER_64bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_FLOATING_POINT_32bits,
			MKNN_DATATYPE_FLOATING_POINT_32bits)
	ASSIGN_RETURN_IF_EQUAL_STR(string, *out_datatype,
			STRING_DATATYPE_FLOATING_POINT_64bits,
			MKNN_DATATYPE_FLOATING_POINT_64bits)
	return false;
}
bool mknn_datatype_isAnySignedInteger(const MknnDatatype datatype) {
	switch (datatype.mknn_datatype_code) {
	case IN08:
	case IN16:
	case IN32:
	case IN64:
		return true;
	default:
		return false;
	}
}

bool mknn_datatype_isAnyUnsignedInteger(const MknnDatatype datatype) {
	switch (datatype.mknn_datatype_code) {
	case UN08:
	case UN16:
	case UN32:
	case UN64:
		return true;
	default:
		return false;
	}
}

bool mknn_datatype_isAnyFloatingPoint(const MknnDatatype datatype) {
	switch (datatype.mknn_datatype_code) {
	case FL32:
	case FL64:
		return true;
	default:
		return false;
	}
}
bool mknn_datatype_isInt8(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == IN08;
}
bool mknn_datatype_isInt16(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == IN16;
}
bool mknn_datatype_isInt32(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == IN32;
}
bool mknn_datatype_isInt64(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == IN64;
}
bool mknn_datatype_isUInt8(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == UN08;
}
bool mknn_datatype_isUInt16(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == UN16;
}
bool mknn_datatype_isUInt32(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == UN32;
}
bool mknn_datatype_isUInt64(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == UN64;
}
bool mknn_datatype_isFloat(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == FL32;
}
bool mknn_datatype_isDouble(const MknnDatatype datatype) {
	return datatype.mknn_datatype_code == FL64;
}
bool mknn_datatype_areEqual(const MknnDatatype datatype1, const MknnDatatype datatype2) {
	return datatype1.mknn_datatype_code == datatype2.mknn_datatype_code;
}

size_t mknn_datatype_sizeof(const MknnDatatype datatype) {
	switch (datatype.mknn_datatype_code) {
	case IN08:
	case UN08:
		return 1;
	case IN16:
	case UN16:
		return 2;
	case IN32:
	case UN32:
	case FL32:
		return 4;
	case IN64:
	case UN64:
	case FL64:
		return 8;
	default:
		return 0;
	}
}
#define RETURN_IF_EQ_MY(var, test_constant, ret_constant) \
	if (var.my_datatype_code == test_constant.my_datatype_code)\
	return ret_constant;

MknnDatatype mknn_datatype_convertMy2Mknn(const MyDatatype my_datatype) {
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_INT8,
			MKNN_DATATYPE_SIGNED_INTEGER_8bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_INT16,
			MKNN_DATATYPE_SIGNED_INTEGER_16bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_INT32,
			MKNN_DATATYPE_SIGNED_INTEGER_32bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_INT64,
			MKNN_DATATYPE_SIGNED_INTEGER_64bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_UINT8,
			MKNN_DATATYPE_UNSIGNED_INTEGER_8bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_UINT16,
			MKNN_DATATYPE_UNSIGNED_INTEGER_16bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_UINT32,
			MKNN_DATATYPE_UNSIGNED_INTEGER_32bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_UINT64,
			MKNN_DATATYPE_UNSIGNED_INTEGER_64bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_FLOAT32,
			MKNN_DATATYPE_FLOATING_POINT_32bits)
	RETURN_IF_EQ_MY(my_datatype, MY_DATATYPE_FLOAT64,
			MKNN_DATATYPE_FLOATING_POINT_64bits)
	my_log_error("unknown my_datatype %i\n",
			(int) my_datatype.my_datatype_code);
	MknnDatatype d = { 0 };
	return d;
}
MyDatatype mknn_datatype_convertMknn2My(const MknnDatatype mknn_datatype) {
	switch (mknn_datatype.mknn_datatype_code) {
	case IN08:
		return MY_DATATYPE_INT8;
	case IN16:
		return MY_DATATYPE_INT16;
	case IN32:
		return MY_DATATYPE_INT32;
	case IN64:
		return MY_DATATYPE_INT64;
	case UN08:
		return MY_DATATYPE_UINT8;
	case UN16:
		return MY_DATATYPE_UINT16;
	case UN32:
		return MY_DATATYPE_UINT32;
	case UN64:
		return MY_DATATYPE_UINT64;
	case FL32:
		return MY_DATATYPE_FLOAT32;
	case FL64:
		return MY_DATATYPE_FLOAT64;
	}
	my_log_error("unknown mknn_datatype %i\n",
			(int) mknn_datatype.mknn_datatype_code);
	MyDatatype d = { 0 };
	return d;
}

