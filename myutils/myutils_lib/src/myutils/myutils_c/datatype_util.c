/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "datatype_util.h"

#define IN08 65
#define IN16 66
#define IN32 67
#define IN64 68

#define UN08 73
#define UN16 74
#define UN32 75
#define UN64 76

#define FL32 85
#define FL64 86

MyDatatype MY_DATATYPE_INT8 = { IN08 }; //A
MyDatatype MY_DATATYPE_INT16 = { IN16 }; //B
MyDatatype MY_DATATYPE_INT32 = { IN32 }; //C
MyDatatype MY_DATATYPE_INT64 = { IN64 }; //D

MyDatatype MY_DATATYPE_UINT8 = { UN08 }; //I
MyDatatype MY_DATATYPE_UINT16 = { UN16 }; //J
MyDatatype MY_DATATYPE_UINT32 = { UN32 }; //K
MyDatatype MY_DATATYPE_UINT64 = { UN64 }; //L

MyDatatype MY_DATATYPE_FLOAT32 = { FL32 }; //U
MyDatatype MY_DATATYPE_FLOAT64 = { FL64 }; //V

size_t my_datatype_sizeof(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
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
bool my_datatype_isAnyInteger(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
	case IN08:
	case IN16:
	case IN32:
	case IN64:
	case UN08:
	case UN16:
	case UN32:
	case UN64:
		return true;
	default:
		return false;
	}
}
bool my_datatype_isAnySignedInteger(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
	case IN08:
	case IN16:
	case IN32:
	case IN64:
		return true;
	default:
		return false;
	}
}
bool my_datatype_isAnyUnsignedInteger(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
	case UN08:
	case UN16:
	case UN32:
	case UN64:
		return true;
	default:
		return false;
	}
}
bool my_datatype_isAnyFloatingPoint(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
	case FL32:
	case FL64:
		return true;
	default:
		return false;
	}
}

bool my_datatype_isInt8(const MyDatatype datatype) {
	return datatype.my_datatype_code == IN08;
}
bool my_datatype_isInt16(const MyDatatype datatype) {
	return datatype.my_datatype_code == IN16;
}
bool my_datatype_isInt32(const MyDatatype datatype) {
	return datatype.my_datatype_code == IN32;
}
bool my_datatype_isInt64(const MyDatatype datatype) {
	return datatype.my_datatype_code == IN64;
}
bool my_datatype_isUInt8(const MyDatatype datatype) {
	return datatype.my_datatype_code == UN08;
}
bool my_datatype_isUInt16(const MyDatatype datatype) {
	return datatype.my_datatype_code == UN16;
}
bool my_datatype_isUInt32(const MyDatatype datatype) {
	return datatype.my_datatype_code == UN32;
}
bool my_datatype_isUInt64(const MyDatatype datatype) {
	return datatype.my_datatype_code == UN64;
}
bool my_datatype_isFloat(const MyDatatype datatype) {
	return datatype.my_datatype_code == FL32;
}
bool my_datatype_isDouble(const MyDatatype datatype) {
	return datatype.my_datatype_code == FL64;
}
bool my_datatype_areEqual(const MyDatatype datatype1,
		const MyDatatype datatype2) {
	return datatype1.my_datatype_code == datatype2.my_datatype_code;
}

const char *my_datatype_codeToDescription(const MyDatatype datatype) {
	switch (datatype.my_datatype_code) {
	case IN08:
		return "INT8";
	case UN08:
		return "UINT8";
	case IN16:
		return "INT16";
	case UN16:
		return "UINT16";
	case IN32:
		return "INT32";
	case UN32:
		return "UINT32";
	case FL32:
		return "FLOAT32";
	case IN64:
		return "INT64";
	case UN64:
		return "UINT64";
	case FL64:
		return "FLOAT64";
	default:
		return "?";
	}
}
MyDatatype my_datatype_descriptionToCode(const char *description) {
	if (strcmp(description, "INT8") == 0)
		return MY_DATATYPE_INT8;
	else if (strcmp(description, "UINT8") == 0)
		return MY_DATATYPE_UINT8;
	else if (strcmp(description, "INT16") == 0)
		return MY_DATATYPE_INT16;
	else if (strcmp(description, "UINT16") == 0)
		return MY_DATATYPE_UINT16;
	else if (strcmp(description, "INT32") == 0)
		return MY_DATATYPE_INT32;
	else if (strcmp(description, "UINT32") == 0)
		return MY_DATATYPE_UINT32;
	else if (strcmp(description, "FLOAT32") == 0)
		return MY_DATATYPE_FLOAT32;
	else if (strcmp(description, "INT64") == 0)
		return MY_DATATYPE_INT64;
	else if (strcmp(description, "UINT64") == 0)
		return MY_DATATYPE_UINT64;
	else if (strcmp(description, "FLOAT64") == 0)
		return MY_DATATYPE_FLOAT64;
	else
		my_log_error("unknown datatype %s\n", description);
	MyDatatype d = { 0 };
	return d;
}

/***************************************************/
#define FUNC_VALUE_TOSTRING(datatype, typeVector, typePrintf) \
static char *value_to_string_##datatype(typeVector value) { \
	return my_newString_format("%"typePrintf, value); \
}
FUNC_VALUE_TOSTRING(IN08, int8_t, PRIi8)
FUNC_VALUE_TOSTRING(UN08, int16_t, PRIi16)
FUNC_VALUE_TOSTRING(IN16, int32_t, PRIi32)
FUNC_VALUE_TOSTRING(UN16, int64_t, PRIi64)
FUNC_VALUE_TOSTRING(IN32, uint8_t, PRIu8)
FUNC_VALUE_TOSTRING(UN32, uint16_t, PRIu16)
FUNC_VALUE_TOSTRING(IN64, uint32_t, PRIu32)
FUNC_VALUE_TOSTRING(UN64, uint64_t, PRIu64)

static char *value_to_string_FL32(float value) {
	return my_newString_float(value);
}
static char *value_to_string_FL64(double value) {
	return my_newString_double(value);
}

#define FUNC_ARRAY_TOSTRING(datatype, typeVector, functionName) \
static char *array_to_string_##datatype(void *array, size_t array_length, const char *prefix,\
		const char *separator, const char *suffix) {\
	typeVector *vector = array; \
	MyStringBuffer *sb = my_stringbuf_new();\
	if (prefix != NULL)\
		my_stringbuf_appendString(sb, prefix);\
	for (size_t i = 0; i < array_length; ++i) {\
		if (i > 0 && separator != NULL)\
			my_stringbuf_appendString(sb, separator);\
		char *st = functionName(vector[i]);\
		my_stringbuf_appendString(sb, st);\
		free(st);\
	}\
	if (suffix != NULL)\
		my_stringbuf_appendString(sb, suffix);\
	return my_stringbuf_releaseReturnBuffer(sb);\
}
FUNC_ARRAY_TOSTRING(IN08, int8_t, value_to_string_IN08)
FUNC_ARRAY_TOSTRING(UN08, uint8_t, value_to_string_UN08)
FUNC_ARRAY_TOSTRING(IN16, int16_t, value_to_string_IN16)
FUNC_ARRAY_TOSTRING(UN16, uint16_t, value_to_string_UN16)
FUNC_ARRAY_TOSTRING(IN32, int32_t, value_to_string_IN32)
FUNC_ARRAY_TOSTRING(UN32, uint32_t, value_to_string_UN32)
FUNC_ARRAY_TOSTRING(IN64, int64_t, value_to_string_IN64)
FUNC_ARRAY_TOSTRING(UN64, uint64_t, value_to_string_UN64)
FUNC_ARRAY_TOSTRING(FL32, float, value_to_string_FL32)
FUNC_ARRAY_TOSTRING(FL64, double, value_to_string_FL64)

#define INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, constantDatatype) \
if (varDatatype.my_datatype_code == constantDatatype) { \
	return namePrefix##constantDatatype; \
}

#define RETURN_SINGLE_DATATYPE(varDatatype, namePrefix) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, IN08 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, IN16 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, IN32 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, IN64 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, UN08 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, UN16 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, UN32 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, UN64 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, FL32 ) \
INTERNAL_TEST_SINGLE_ASSIGN(varDatatype, namePrefix, FL64 )

my_function_to_string my_datatype_getFunctionToString(const MyDatatype datatype) {
	RETURN_SINGLE_DATATYPE(datatype, array_to_string_)
	my_log_error("unknown my datatype code %i\n", datatype.my_datatype_code);
	return NULL;
}

/***************************************************/

#define FUNC_PARSE(datatype, typeVector, parse_function) \
static void convert_from_string_##datatype(const char *string, void *ptr_vector, size_t pos) { \
	typeVector *vector = ptr_vector; \
	vector[pos] = (typeVector) parse_function(string); \
}
FUNC_PARSE(IN08, int8_t, my_parse_int)
FUNC_PARSE(UN08, uint8_t, my_parse_uint8)
FUNC_PARSE(IN16, int16_t, my_parse_int)
FUNC_PARSE(UN16, uint16_t, my_parse_int)
FUNC_PARSE(IN32, int32_t, my_parse_int)
FUNC_PARSE(UN32, uint32_t, my_parse_int)
FUNC_PARSE(IN64, int64_t, my_parse_int)
FUNC_PARSE(UN64, uint64_t, my_parse_int)
FUNC_PARSE(FL32, float, my_parse_double)
FUNC_PARSE(FL64, double, my_parse_double)

my_function_parse_string my_datatype_getFunctionParseVector(
		const MyDatatype datatype) {
	RETURN_SINGLE_DATATYPE(datatype, convert_from_string_)
	my_log_error("unknown my datatype code %i\n", datatype.my_datatype_code);
	return NULL;
}

/*************************************/

#define FUNC_COPY_VECTOR(datatypeSrc, typeSrc, datatypeDst, typeDst) \
static void copy_vector_##datatypeSrc##_##datatypeDst(void *ptr_vector_src, void *ptr_vector_dst, size_t dimensions) {\
	typeSrc *vector_src = ptr_vector_src; \
	typeDst *vector_dst = ptr_vector_dst; \
	size_t numN = dimensions / 4; \
	while (numN > 0) { \
		vector_dst[0] = (typeDst) (vector_src[0]); \
		vector_dst[1] = (typeDst) (vector_src[1]); \
		vector_dst[2] = (typeDst) (vector_src[2]); \
		vector_dst[3] = (typeDst) (vector_src[3]); \
		vector_src += 4; \
		vector_dst += 4; \
		numN--; \
	} \
	size_t numB = dimensions % 4; \
	while (numB > 0) { \
		vector_dst[0] = (typeDst) (vector_src[0]); \
		vector_src += 1; \
		vector_dst += 1; \
		numB--; \
	} \
}

#define INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, datatype1, type1) \
VAR_NAME(datatype1, type1, IN08 ,   int8_t ) \
VAR_NAME(datatype1, type1, UN08 ,  uint8_t ) \
VAR_NAME(datatype1, type1, IN16 ,  int16_t ) \
VAR_NAME(datatype1, type1, UN16 , uint16_t ) \
VAR_NAME(datatype1, type1, IN32 ,  int32_t ) \
VAR_NAME(datatype1, type1, UN32 , uint32_t ) \
VAR_NAME(datatype1, type1, IN64 ,  int64_t ) \
VAR_NAME(datatype1, type1, UN64 , uint64_t ) \
VAR_NAME(datatype1, type1, FL32 ,    float ) \
VAR_NAME(datatype1, type1, FL64 ,   double )

#define GENERATE_TWO_DATATYPES(VAR_NAME) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, IN08 ,   int8_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, UN08 ,  uint8_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, IN16 ,  int16_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, UN16 , uint16_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, IN32 ,  int32_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, UN32 , uint32_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, IN64 ,  int64_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, UN64 , uint64_t ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, FL32 ,    float ) \
INTERNAL_GENERATE_TWO_DATATYPES(VAR_NAME, FL64 ,   double )

GENERATE_TWO_DATATYPES(FUNC_COPY_VECTOR)

#define INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, constantDatatype2) \
if (varDatatype1.my_datatype_code == constantDatatype1 && varDatatype2.my_datatype_code == constantDatatype2) { \
	return namePrefix##constantDatatype1##_##constantDatatype2; \
}

#define INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, IN08 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, IN16 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, IN32 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, IN64 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, UN08 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, UN16 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, UN32 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, UN64 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, FL32 ) \
INTERNAL_TEST_TWO(varDatatype1, varDatatype2, namePrefix, constantDatatype1, FL64 )

#define RETURN_TWO_DATATYPES(varDatatype1, varDatatype2, namePrefix) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, IN08 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, IN16 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, IN32 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, IN64 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, UN08 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, UN16 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, UN32 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, UN64 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, FL32 ) \
INTERNAL_RETURN_TWO(varDatatype1, varDatatype2, namePrefix, FL64 )

my_function_copy_vector my_datatype_getFunctionCopyVector(
		const MyDatatype datatype_src, const MyDatatype datatype_dst) {
	RETURN_TWO_DATATYPES(datatype_src, datatype_dst, copy_vector_)
	my_log_error("unknown my datatypes codes %i %i\n",
			datatype_src.my_datatype_code, datatype_dst.my_datatype_code);
	return NULL;
}

/*************************************/

#define FUNC_COPYOPERATE_VECTOR(datatypeSrc, typeSrc, datatypeDst, typeDst) \
static void copyOperate_vector_##datatypeSrc##_##datatypeDst(void *ptr_vector_src, void *ptr_vector_dst, size_t dimensions, my_function_number_operator funcOperate) {\
	typeSrc *vector_src = ptr_vector_src; \
	typeDst *vector_dst = ptr_vector_dst; \
	size_t numN = dimensions / 4; \
	while (numN > 0) { \
		vector_dst[0] = (typeDst) funcOperate((double) vector_src[0]); \
		vector_dst[1] = (typeDst) funcOperate((double) vector_src[1]); \
		vector_dst[2] = (typeDst) funcOperate((double) vector_src[2]); \
		vector_dst[3] = (typeDst) funcOperate((double) vector_src[3]); \
		vector_src += 4; \
		vector_dst += 4; \
		numN--; \
	} \
	size_t numB = dimensions % 4; \
	while (numB > 0) { \
		vector_dst[0] = (typeDst) funcOperate((double) vector_src[0]); \
		vector_src += 1; \
		vector_dst += 1; \
		numB--; \
	} \
}

GENERATE_TWO_DATATYPES(FUNC_COPYOPERATE_VECTOR)

my_function_copyOperate_vector my_datatype_getFunctionCopyOperateVector(
		const MyDatatype datatype_src, const MyDatatype datatype_dst) {
	RETURN_TWO_DATATYPES(datatype_src, datatype_dst, copyOperate_vector_)
	my_log_error("unknown my datatypes codes %i %i\n",
			datatype_src.my_datatype_code, datatype_dst.my_datatype_code);
	return NULL;
}

/*************************************/
