/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "ex.h"

DescriptorType descriptorType(int64_t dtype, int64_t array_length,
		int64_t num_subarrays, int64_t subarray_length) {
	DescriptorType td = { 0 };
	td.dtype = dtype;
	td.array_length = array_length;
	td.num_subarrays = num_subarrays;
	td.subarray_length = subarray_length;
	return td;
}

const char *dtype2string(int64_t dtype) {
	switch (dtype) {
	case DTYPE_ARRAY_UCHAR:
		return "ARRAY_UCHAR";
	case DTYPE_ARRAY_FLOAT:
		return "ARRAY_FLOAT";
	case DTYPE_ARRAY_DOUBLE:
		return "ARRAY_DOUBLE";
	case DTYPE_MATRIX_BITS:
		return "MATRIX_BITS";
	case DTYPE_LOCAL_VECTORS:
		return "LOCAL_VECTORS";
	case DTYPE_SPARSE_ARRAY:
		return "SPARSE_ARRAY";
	}
	my_log_error("unknown descriptor type %"PRIi64"\n", dtype);
	return NULL;
}

int64_t string2dtype(const char *dtypeName) {
	if (my_string_equals(dtypeName, "ARRAY_UCHAR"))
		return DTYPE_ARRAY_UCHAR;
	else if (my_string_equals(dtypeName, "ARRAY_FLOAT"))
		return DTYPE_ARRAY_FLOAT;
	else if (my_string_equals(dtypeName, "ARRAY_DOUBLE"))
		return DTYPE_ARRAY_DOUBLE;
	else if (my_string_equals(dtypeName, "MATRIX_BITS"))
		return DTYPE_MATRIX_BITS;
	else if (my_string_equals(dtypeName, "LOCAL_VECTORS"))
		return DTYPE_LOCAL_VECTORS;
	else if (my_string_equals(dtypeName, "SPARSE_ARRAY"))
		return DTYPE_SPARSE_ARRAY;
	my_log_error("unknown descriptor name %s\n", dtypeName);
	return 0;
}

void assertEqualDescriptorType(DescriptorType td1, DescriptorType td2) {
	if (td1.dtype != td2.dtype || td1.array_length != td2.array_length
			|| td1.num_subarrays != td2.num_subarrays
			|| td1.subarray_length != td2.subarray_length)
		my_log_error(
				"incompatible descriptors: (%s,%"PRIi64",%"PRIi64",%"PRIi64") != (%s,%"PRIi64",%"PRIi64",%"PRIi64")\n",
				dtype2string(td1.dtype), td1.array_length, td1.num_subarrays,
				td1.subarray_length, dtype2string(td2.dtype), td2.array_length,
				td2.num_subarrays, td2.subarray_length);
}
int64_t getLengthBytesDescriptor(DescriptorType td) {
	switch (td.dtype) {
	case DTYPE_ARRAY_UCHAR:
	case DTYPE_MATRIX_BITS:
		return sizeof(char) * td.array_length;
	case DTYPE_ARRAY_FLOAT:
		return sizeof(float) * td.array_length;
	case DTYPE_ARRAY_DOUBLE:
		return sizeof(double) * td.array_length;
	}
	my_log_error("unsupported dtype %"PRIi64"\n", td.dtype);
	return 0;
}
//hay que hacerle MY_FREE a lo retornado
void* cloneDescriptor(DescriptorType td, void *desSrc) {
	if (desSrc == NULL) {
		return NULL;
	} else if (td.dtype == DTYPE_LOCAL_VECTORS) {
		return my_localDescriptors_clone(desSrc);
	} else if (td.dtype == DTYPE_SPARSE_ARRAY) {
		return my_sparseArray_clone(desSrc);
	} else {
		int64_t numBytes = getLengthBytesDescriptor(td);
		char *bytes = MY_MALLOC_NOINIT(numBytes, char);
		memcpy(bytes, desSrc, numBytes);
		return bytes;
	}
}
char *printDescriptor(DescriptorType td, void *descriptor) {
	my_assert_notNull("descriptor", descriptor);
	switch (td.dtype) {
	case DTYPE_ARRAY_UCHAR:
	case DTYPE_MATRIX_BITS:
		return my_newString_arrayUint8(descriptor, td.array_length, '\t');
	case DTYPE_ARRAY_FLOAT:
		return my_newString_arrayFloat(descriptor, td.array_length, '\t');
	case DTYPE_ARRAY_DOUBLE:
		return my_newString_arrayDouble(descriptor, td.array_length, '\t');
	case DTYPE_LOCAL_VECTORS:
		return my_localDescriptors_toString(descriptor);
	case DTYPE_SPARSE_ARRAY:
		return my_sparseArray_toString(descriptor);
	}
	my_log_error("unknown dtype %"PRIi64"\n", td.dtype);
	return NULL;
}
void descriptor2doubleArray(DescriptorType td, void *descriptor,
		double *out_array) {
	int64_t i;
	switch (td.dtype) {
	case DTYPE_ARRAY_FLOAT:
		for (i = 0; i < td.array_length; ++i) {
			float val = ((float*) descriptor)[i];
			out_array[i] = val;
		}
		break;
	case DTYPE_ARRAY_DOUBLE:
		for (i = 0; i < td.array_length; ++i) {
			double val = ((double*) descriptor)[i];
			out_array[i] = val;
		}
		break;
	case DTYPE_ARRAY_UCHAR:
		for (i = 0; i < td.array_length; ++i) {
			uint8_t val = ((uint8_t*) descriptor)[i];
			out_array[i] = val;
		}
		break;
	default:
		my_log_error("dtype %"PRIi64" unknown\n", td.dtype);
		break;
	}
}

//quantizacion (R|L) round o lineal, (1|2|4|8) bytes, (U|S|F) unsigned,signed,float
struct Quantization getQuantization(const char *name) {
	if (name == NULL || strlen(name) < 2)
		my_log_error("invalid quantization %s\n", name);
	struct Quantization quant = { 0, 0 };
	if (name[0] == 'R') {
		quant.quantizeType = QUANT_ROUND;
		name++;
	} else {
		quant.quantizeType = QUANT_LINEAL;
	}
	if (my_string_equals(name, "1U"))
		quant.dtype = DTYPE_ARRAY_UCHAR;
	else if (my_string_equals(name, "4F"))
		quant.dtype = DTYPE_ARRAY_FLOAT;
	else if (my_string_equals(name, "8F"))
		quant.dtype = DTYPE_ARRAY_DOUBLE;
	else
		my_log_error("unknown quantization %s\n", name);
	return quant;
}
void *newDescriptorQuantize(struct Quantization quant, int64_t length) {
	switch (quant.dtype) {
	case DTYPE_ARRAY_UCHAR:
		return MY_MALLOC_NOINIT(length, uint8_t);
	case DTYPE_ARRAY_FLOAT:
		return MY_MALLOC_NOINIT(length, float);
	case DTYPE_ARRAY_DOUBLE:
		return MY_MALLOC_NOINIT(length, double);
	}
	my_log_error("tipoDescriptor %"PRIi64" desconocido\n", quant.dtype);
	return NULL;
}
int64_t qlineal_01_To_0Max(double double_val, int64_t max) {
	my_assert_lessEqualDouble("val in qlineal_01_To_0Max", double_val, 1);
	double val_dbl = double_val * max;
	int64_t val_int = (int64_t) val_dbl;
	if (val_int <= 0)
		return 0;
	else if (val_int >= max)
		return max;
	else if (val_int == val_dbl)
		return val_int;
	else
		return val_int + 1;
}
void quantize(struct Quantization quant, double *src_array, void *descriptor,
		int64_t length) {
	int64_t i;
	switch (quant.dtype) {
	case DTYPE_ARRAY_UCHAR: {
		uint8_t *out = descriptor;
		if (quant.quantizeType == QUANT_ROUND) {
			for (i = 0; i < length; ++i)
				out[i] = my_math_round_uint8(src_array[i]);
		} else {
			for (i = 0; i < length; ++i)
				out[i] = (uint8_t) qlineal_01_To_0Max(src_array[i], 255);
		}
		break;
	}
	case DTYPE_ARRAY_FLOAT: {
		float *out = descriptor;
		for (i = 0; i < length; ++i)
			out[i] = (float) src_array[i];
		break;
	}
	case DTYPE_ARRAY_DOUBLE: {
		double *out = descriptor;
		for (i = 0; i < length; ++i)
			out[i] = src_array[i];
		break;
	}
	default:
		my_log_error("unknown quantization %"PRIi64"\n", quant.dtype);
		break;
	}
}

#ifndef NO_OPENCV

double averageIntensity(IplImage *imgGray, int64_t startW, int64_t startH,
		int64_t endWNotIncluded, int64_t endHNotIncluded) {
	my_assert_lessEqualInt("endW", endWNotIncluded, imgGray->width);
	my_assert_lessEqualInt("endH", endHNotIncluded, imgGray->height);
	double average = 0;
	int64_t x, y, cont = 0;
	for (y = startH; y < endHNotIncluded; ++y) {
		uchar *ptr = (uchar*) (imgGray->imageData + imgGray->widthStep * y);
		for (x = startW; x < endWNotIncluded; ++x) {
			cont++;
			average += (ptr[x] - average) / cont;
		}
	}
	return average;
}

// ------> width
// d00 d01
// d10 d11
double *newKernel(double d00, double d01, double d10, double d11) {
	double * k = MY_MALLOC_NOINIT(4, double);
	k[0] = d00;
	k[1] = d01;
	k[2] = d10;
	k[3] = d11;
	return k;
}

struct Zoning *parseZoning(const char *paramsZoning) {
	MyTokenizer *tk = my_tokenizer_new(paramsZoning, '+');
	MyVectorObj *zone_list = my_vectorObj_new();
	while (my_tokenizer_hasNext(tk)) {
		const char *param = my_tokenizer_nextToken(tk);
		MyTokenizer *tk2 = my_tokenizer_new(param, 'x');
		int64_t zw = my_tokenizer_nextInt(tk2);
		int64_t zh = my_tokenizer_nextInt(tk2);
		my_tokenizer_releaseValidateEnd(tk2);
		int64_t i, j;
		for (i = 0; i < zh; ++i) {
			for (j = 0; j < zw; ++j) {
				struct Zone *z = MY_MALLOC(1, struct Zone);
				z->prop_start_w = j / (double) zw;
				z->prop_end_w = (j + 1) / (double) zw;
				z->prop_start_h = i / (double) zh;
				z->prop_end_h = (i + 1) / (double) zh;
				z->numZone = my_vectorObj_size(zone_list);
				my_vectorObj_add(zone_list, z);
			}
		}
	}
	my_tokenizer_releaseValidateEnd(tk);
	struct Zoning *zoning = MY_MALLOC(1, struct Zoning);
	zoning->numZones = my_vectorObj_size(zone_list);
	zoning->zones = (struct Zone **) my_vectorObj_array(zone_list);
	MY_FREE(zone_list);
	return zoning;
}
void processZoning(IplImage *image, struct Zoning *zoning,
		void (*func_processZone)(IplImage *image, struct Zone *zone,
				void *state), void *state) {
	if (zoning->currentW != image->width || zoning->currentH != image->height) {
		zoning->currentW = image->width;
		zoning->currentH = image->height;
		for (int64_t i = 0; i < zoning->numZones; ++i) {
			struct Zone *z = zoning->zones[i];
			z->pix_start_w = (int64_t) (z->prop_start_w * zoning->currentW);
			z->pix_end_w = (int64_t) (z->prop_end_w * zoning->currentW);
			z->pix_start_h = (int64_t) (z->prop_start_h * zoning->currentH);
			z->pix_end_h = (int64_t) (z->prop_end_h * zoning->currentH);
			z->rect = cvRect(z->pix_start_w, z->pix_start_h,
					z->pix_end_w - z->pix_start_w,
					z->pix_end_h - z->pix_start_h);
		}
	}
	for (int64_t i = 0; i < zoning->numZones; ++i) {
		struct Zone *z = zoning->zones[i];
		func_processZone(image, z, state);
	}
}
void releaseZoning(struct Zoning *zoning) {
	if (zoning == NULL)
		return;
	for (int64_t i = 0; i < zoning->numZones; ++i) {
		struct Zone *z = zoning->zones[i];
		free(z);
	}
	free(zoning);
}
void applyPcaToLocalDescriptors(MknnPcaAlgorithm *pca,
		MyLocalDescriptors *src_descriptor, MyLocalDescriptors *dst_descriptor) {
	int64_t num = my_localDescriptors_getNumDescriptors(src_descriptor);
	if (num == 0) {
		my_localDescriptors_redefineNumDescriptors(dst_descriptor, 0);
		return;
	}
	int64_t dims1 = my_localDescriptors_getVectorDimensions(src_descriptor);
	int64_t dims2 = my_localDescriptors_getVectorDimensions(dst_descriptor);
	my_assert_equalInt("pca input dims", dims1,
			mknn_pca_getInputDimension(pca));
	my_assert_lessEqualInt("pca output dims", dims2, dims1);
	MyDatatype dtype1 = my_localDescriptors_getVectorDatatype(src_descriptor);
	MyDatatype dtype2 = my_localDescriptors_getVectorDatatype(dst_descriptor);
	my_localDescriptors_redefineNumDescriptors(dst_descriptor, num);
	for (int64_t i = 0; i < num; ++i) {
		struct MyLocalKeypoint kp = my_localDescriptors_getKeypoint(
				src_descriptor, i);
		my_localDescriptors_setKeypointSt(dst_descriptor, i, kp);
	}
	mknn_pca_func_transformVector funcTransf =
			mknn_pca_getTransformVectorFunction(
					mknn_datatype_convertMy2Mknn(dtype1),
					mknn_datatype_convertMy2Mknn(dtype2));
	for (int64_t i = 0; i < num; ++i) {
		void *vector1 = my_localDescriptors_getVector(src_descriptor, i);
		void *vector2 = my_localDescriptors_getVector(dst_descriptor, i);
		funcTransf(pca, vector1, vector2, dims2);
	}
}
#endif
