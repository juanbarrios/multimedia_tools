/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#ifndef MACROS_UTIL_H_
#define MACROS_UTIL_H_

/******************/

#define INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, constantDatatype, typeVector) \
} else if (varDatatype.mknn_datatype_code == constantDatatype.mknn_datatype_code) { \
	varAssign = namePrefix##typeVector; \

#define ASSIGN_SINGLE_DATATYPE(varAssign, varDatatype, namePrefix) \
if(false) { \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_8bits, int8_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_16bits, int16_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_32bits, int32_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_64bits, int64_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_8bits, uint8_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_16bits, uint16_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_32bits, uint32_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_64bits, uint64_t) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_FLOATING_POINT_32bits, float) \
INTERNAL_TEST_SINGLE_ASSIGN(varAssign, varDatatype, namePrefix, MKNN_DATATYPE_FLOATING_POINT_64bits, double) \
} else \
my_log_error("unknown mknn_datatype code %i\n", varDatatype.mknn_datatype_code);

/******************/

#define INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, constantDatatype2, typeVector2) \
} else if (varDatatype1.mknn_datatype_code == constantDatatype1.mknn_datatype_code && varDatatype2.mknn_datatype_code == constantDatatype2.mknn_datatype_code) { \
	varAssign = namePrefix##typeVector1##_##typeVector2; \

#define INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_SIGNED_INTEGER_8bits, int8_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_SIGNED_INTEGER_16bits, int16_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_SIGNED_INTEGER_32bits, int32_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_SIGNED_INTEGER_64bits, int64_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_UNSIGNED_INTEGER_8bits, uint8_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_UNSIGNED_INTEGER_16bits, uint16_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_UNSIGNED_INTEGER_32bits, uint32_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_UNSIGNED_INTEGER_64bits, uint64_t) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_FLOATING_POINT_32bits, float) \
INTERNAL_TEST_DOUBLE_ASSIGN(varAssign, varDatatype1, varDatatype2, namePrefix, constantDatatype1, typeVector1, MKNN_DATATYPE_FLOATING_POINT_64bits, double)

#define ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix) \
if(false) { \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_8bits, int8_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_16bits, int16_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_32bits, int32_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_SIGNED_INTEGER_64bits, int64_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_8bits, uint8_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_16bits, uint16_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_32bits, uint32_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_UNSIGNED_INTEGER_64bits, uint64_t) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_FLOATING_POINT_32bits, float) \
INTERNAL_ASSIGN_DOUBLE_DATATYPE(varAssign, varDatatype1, varDatatype2, namePrefix, MKNN_DATATYPE_FLOATING_POINT_64bits, double) \
} else \
my_log_error("unknown mknn_datatypes codes %i %i\n", varDatatype1.mknn_datatype_code, varDatatype2.mknn_datatype_code);


/******************/
#define GENERATE_SINGLE_DATATYPE(VAR_NAME) \
VAR_NAME(int8_t) \
VAR_NAME(int16_t) \
VAR_NAME(int32_t) \
VAR_NAME(int64_t) \
VAR_NAME(uint8_t) \
VAR_NAME(uint16_t) \
VAR_NAME(uint32_t) \
VAR_NAME(uint64_t) \
VAR_NAME(float) \
VAR_NAME(double)

/******************/

#define INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME, type1) \
VAR_NAME(type1,   int8_t) \
VAR_NAME(type1,  uint8_t) \
VAR_NAME(type1,  int16_t) \
VAR_NAME(type1, uint16_t) \
VAR_NAME(type1,  int32_t) \
VAR_NAME(type1, uint32_t) \
VAR_NAME(type1,  int64_t) \
VAR_NAME(type1, uint64_t) \
VAR_NAME(type1,    float) \
VAR_NAME(type1,   double)

#define GENERATE_DOUBLE_DATATYPE(VAR_NAME) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,   int8_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,  uint8_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,  int16_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME, uint16_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,  int32_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME, uint32_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,  int64_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME, uint64_t) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,    float) \
INTERNAL_GENERATE_DOUBLE_DATATYPE(VAR_NAME,   double)

/* **************** */

#define INTERNAL_FLOAT_WITH_DIFF_ABS(VAR_NAME, varFloat) \
VAR_NAME(varFloat ,   int8_t ,  float , fabsf) \
VAR_NAME(varFloat ,  uint8_t ,  float , fabsf) \
VAR_NAME(varFloat ,  int16_t , double , fabs ) \
VAR_NAME(varFloat , uint16_t , double , fabs ) \
VAR_NAME(varFloat ,  int32_t , double , fabs ) \
VAR_NAME(varFloat , uint32_t , double , fabs ) \
VAR_NAME(varFloat ,  int64_t , double , fabs ) \
VAR_NAME(varFloat , uint64_t , double , fabs ) \
VAR_NAME(varFloat ,    float , float  , fabsf) \
VAR_NAME(varFloat ,   double , double , fabs )

#define INTERNAL_DOUBLE_WITH_DIFF_ABS(VAR_NAME, varDouble) \
VAR_NAME(varDouble,   int8_t , double , fabs ) \
VAR_NAME(varDouble,  uint8_t , double , fabs ) \
VAR_NAME(varDouble,  int16_t , double , fabs ) \
VAR_NAME(varDouble, uint16_t , double , fabs ) \
VAR_NAME(varDouble,  int32_t , double , fabs ) \
VAR_NAME(varDouble, uint32_t , double , fabs ) \
VAR_NAME(varDouble,  int64_t , double , fabs ) \
VAR_NAME(varDouble, uint64_t , double , fabs ) \
VAR_NAME(varDouble,    float , double , fabs ) \
VAR_NAME(varDouble,   double , double , fabs )

#define INTERNAL_INT3264_WITH_DIFF_ABS(VAR_NAME, varInt) \
VAR_NAME(varInt ,   int8_t , int64_t , llabs) \
VAR_NAME(varInt ,  uint8_t , int64_t , llabs) \
VAR_NAME(varInt ,  int16_t , int64_t , llabs) \
VAR_NAME(varInt , uint16_t , int64_t , llabs) \
VAR_NAME(varInt ,  int32_t , int64_t , llabs) \
VAR_NAME(varInt , uint32_t , int64_t , llabs) \
VAR_NAME(varInt ,  int64_t , int64_t , llabs) \
VAR_NAME(varInt , uint64_t , int64_t , llabs) \
VAR_NAME(varInt ,    float ,  double , fabs ) \
VAR_NAME(varInt ,   double ,  double , fabs )

#define INTERNAL_INT16_WITH_DIFF_ABS(VAR_NAME, varInt16) \
VAR_NAME(varInt16 ,   int8_t , int32_t , labs ) \
VAR_NAME(varInt16 ,  uint8_t , int32_t , labs ) \
VAR_NAME(varInt16 ,  int16_t , int32_t , labs ) \
VAR_NAME(varInt16 , uint16_t , int32_t , labs ) \
VAR_NAME(varInt16 ,  int32_t , int64_t , llabs) \
VAR_NAME(varInt16 , uint32_t , int64_t , llabs) \
VAR_NAME(varInt16 ,  int64_t , int64_t , llabs) \
VAR_NAME(varInt16 , uint64_t , int64_t , llabs) \
VAR_NAME(varInt16 ,    float ,  double , fabs ) \
VAR_NAME(varInt16 ,   double ,  double , fabs )

#define INTERNAL_INT8_WITH_DIFF_ABS(VAR_NAME, varInt8) \
VAR_NAME(varInt8 ,   int8_t , int32_t , abs  ) \
VAR_NAME(varInt8 ,  uint8_t , int32_t , abs  ) \
VAR_NAME(varInt8 ,  int16_t , int32_t , labs ) \
VAR_NAME(varInt8 , uint16_t , int32_t , labs ) \
VAR_NAME(varInt8 ,  int32_t , int64_t , llabs) \
VAR_NAME(varInt8 , uint32_t , int64_t , llabs) \
VAR_NAME(varInt8 ,  int64_t , int64_t , llabs) \
VAR_NAME(varInt8 , uint64_t , int64_t , llabs) \
VAR_NAME(varInt8 ,    float ,  double , fabs ) \
VAR_NAME(varInt8 ,   double ,  double , fabs )

#define GENERATE_DOUBLE_DATATYPE_WITH_DIFF_ABS(VAR_NAME) \
INTERNAL_INT8_WITH_DIFF_ABS(VAR_NAME,      int8_t) \
INTERNAL_INT8_WITH_DIFF_ABS(VAR_NAME,     uint8_t) \
INTERNAL_INT16_WITH_DIFF_ABS(VAR_NAME,    int16_t) \
INTERNAL_INT16_WITH_DIFF_ABS(VAR_NAME,   uint16_t) \
INTERNAL_INT3264_WITH_DIFF_ABS(VAR_NAME,  int32_t) \
INTERNAL_INT3264_WITH_DIFF_ABS(VAR_NAME, uint32_t) \
INTERNAL_INT3264_WITH_DIFF_ABS(VAR_NAME,  int64_t) \
INTERNAL_INT3264_WITH_DIFF_ABS(VAR_NAME, uint64_t) \
INTERNAL_FLOAT_WITH_DIFF_ABS(VAR_NAME,      float) \
INTERNAL_DOUBLE_WITH_DIFF_ABS(VAR_NAME,     double)

#endif
