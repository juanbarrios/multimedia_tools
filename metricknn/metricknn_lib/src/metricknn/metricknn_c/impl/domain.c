/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnDomain {
	MknnGeneralDomain general_domain;
	struct {
		int64_t num_dimensions;
		MknnDatatype dimension_datatype;
		size_t num_bytes_one_vector;
	} vector;
	struct {
		int64_t length;
		MknnDomain **subdomains;
		bool free_subdomains_on_domain_release;
	} multiobject;
	struct {
		int64_t custom_id;
		void *custom_data;
	} customobject;
};

MknnDomain *mknn_domain_newString() {
	MknnDomain *domain = MY_MALLOC(1, MknnDomain);
	domain->general_domain = MKNN_GENERAL_DOMAIN_STRING;
	return domain;
}

MknnDomain *mknn_domain_newVector(int64_t num_dimensions,
		MknnDatatype dimension_datatype) {
	MknnDomain *domain = MY_MALLOC(1, MknnDomain);
	domain->general_domain = MKNN_GENERAL_DOMAIN_VECTOR;
	domain->vector.num_dimensions = num_dimensions;
	domain->vector.dimension_datatype = dimension_datatype;
	MyDatatype my_datatype = mknn_datatype_convertMknn2My(dimension_datatype);
	domain->vector.num_bytes_one_vector = num_dimensions
			* my_datatype_sizeof(my_datatype);
	return domain;
}

MknnDomain *mknn_domain_newMultiobject(int64_t length, MknnDomain **subdomains,
bool free_subdomains_on_domain_release) {
	MknnDomain *domain = MY_MALLOC(1, MknnDomain);
	domain->general_domain = MKNN_GENERAL_DOMAIN_MULTIOBJECT;
	domain->multiobject.length = length;
	domain->multiobject.free_subdomains_on_domain_release =
			free_subdomains_on_domain_release;
	domain->multiobject.subdomains = MY_MALLOC(length, MknnDomain*);
	for (int64_t i = 0; i < length; ++i) {
		domain->multiobject.subdomains[i] = subdomains[i];
	}
	return domain;
}
MknnDomain *mknn_domain_newCustomObject(int64_t custom_id, void *custom_data) {
	MknnDomain *domain = MY_MALLOC(1, MknnDomain);
	domain->general_domain = MKNN_GENERAL_DOMAIN_CUSTOMOBJECT;
	domain->customobject.custom_id = custom_id;
	domain->customobject.custom_data = custom_data;
	return domain;
}

MknnGeneralDomain mknn_domain_getGeneralDomain(MknnDomain *domain) {
	if (domain != NULL)
		return domain->general_domain;
	MknnGeneralDomain d = { 0 };
	return d;
}
bool mknn_domain_isGeneralDomainVector(MknnDomain *domain) {
	return domain != NULL && mknn_generalDomain_isVector(domain->general_domain);
}
bool mknn_domain_isGeneralDomainString(MknnDomain *domain) {
	return domain != NULL && mknn_generalDomain_isString(domain->general_domain);
}
bool mknn_domain_isGeneralDomainMultiObject(MknnDomain *domain) {
	return domain != NULL
			&& mknn_generalDomain_isMultiObject(domain->general_domain);
}
bool mknn_domain_isGeneralDomainCustomObject(MknnDomain *domain) {
	return domain != NULL && mknn_generalDomain_isCustom(domain->general_domain);
}
int64_t mknn_domain_vector_getNumDimensions(MknnDomain *domain) {
	return domain->vector.num_dimensions;
}
MknnDatatype mknn_domain_vector_getDimensionDataType(MknnDomain *domain) {
	return domain->vector.dimension_datatype;
}
int64_t mknn_domain_vector_getVectorLengthInBytes(MknnDomain *domain) {
	return domain->vector.num_bytes_one_vector;
}
void *mknn_domain_vector_createNewEmptyVectors(MknnDomain *domain,
		int64_t num_vectors) {
	return MY_MALLOC(domain->vector.num_bytes_one_vector * num_vectors, char);
}
void *mknn_domain_vector_getVectorInArray(MknnDomain *domain,
		void *vectors_array, int64_t pos_vector) {
	char *ptrbytes = vectors_array;
	return ptrbytes + domain->vector.num_bytes_one_vector * pos_vector;
}
int64_t mknn_domain_multiobject_getLength(MknnDomain *domain) {
	return domain->multiobject.length;
}
MknnDomain *mknn_domain_multiobject_getSubDomain(MknnDomain *domain,
		int64_t num_subdomain) {
	my_assert_indexRangeInt("num_subdomain", num_subdomain,
			domain->multiobject.length);
	return domain->multiobject.subdomains[num_subdomain];
}
int64_t mknn_domain_custom_getId(MknnDomain *domain) {
	return domain->customobject.custom_id;
}
void *mknn_domain_custom_getData(MknnDomain *domain) {
	return domain->customobject.custom_data;
}

void mknn_domain_release(MknnDomain *domain) {
	if (domain == NULL)
		return;
	if (mknn_generalDomain_isMultiObject(domain->general_domain)) {
		if (domain->multiobject.free_subdomains_on_domain_release) {
			for (int64_t i = 0; i < domain->multiobject.length; ++i)
				mknn_domain_release(domain->multiobject.subdomains[i]);
		}
		free(domain->multiobject.subdomains);
	}
	free(domain);
}
static bool compare_domains(MknnDomain *domain1, MknnDomain *domain2,
bool compare_datatypes) {
	if (domain1 == NULL && domain2 == NULL) {
		return true;
	} else if ((domain1 == NULL && domain2 != NULL)
			|| (domain1 != NULL && domain2 == NULL)
			|| !mknn_generalDomain_areEqual(domain1->general_domain,
					domain2->general_domain)) {
		return false;
	}
	if (mknn_generalDomain_isVector(domain1->general_domain)) {
		if (domain1->vector.num_dimensions != domain2->vector.num_dimensions)
			return false;
		if (compare_datatypes
				&& !mknn_datatype_areEqual(domain1->vector.dimension_datatype,
						domain2->vector.dimension_datatype))
			return false;
	} else if (mknn_generalDomain_isMultiObject(domain1->general_domain)) {
		if (domain1->multiobject.length != domain2->multiobject.length)
			return false;
		for (int64_t i = 0; i < domain1->multiobject.length; ++i) {
			if (!compare_domains(domain1->multiobject.subdomains[i],
					domain2->multiobject.subdomains[i], compare_datatypes))
				return false;
		}
	} else if (mknn_generalDomain_isCustom(domain1->general_domain)) {
		if (domain1->customobject.custom_id != domain2->customobject.custom_id
				|| domain1->customobject.custom_data
						!= domain2->customobject.custom_data)
			return false;
	}
	return true;
}
bool mknn_domain_testEqual(MknnDomain *domain1, MknnDomain *domain2) {
	return compare_domains(domain1, domain2, true);
}

bool mknn_domain_testEqualExceptDatatype(MknnDomain *domain1,
		MknnDomain *domain2) {
	return compare_domains(domain1, domain2, false);
}

char *mknn_domain_toString(MknnDomain *domain) {
	if (domain == NULL)
		return my_newString_string("");
	MyStringBuffer *sb = my_stringbuf_new();
	my_stringbuf_appendString(sb,
			mknn_generalDomain_toString(domain->general_domain));
	if (mknn_generalDomain_isVector(domain->general_domain)) {
		my_stringbuf_appendString(sb, ":");
		my_stringbuf_appendInt(sb, domain->vector.num_dimensions);
		my_stringbuf_appendString(sb, ":");
		my_stringbuf_appendString(sb,
				mknn_datatype_toString(domain->vector.dimension_datatype));
	} else if (mknn_generalDomain_isMultiObject(domain->general_domain)) {
		my_stringbuf_appendString(sb, ":");
		my_stringbuf_appendInt(sb, domain->multiobject.length);
		for (int64_t i = 0; i < domain->multiobject.length; ++i) {
			char *st = mknn_domain_toString(domain->multiobject.subdomains[i]);
			my_stringbuf_appendString(sb, ":(");
			my_stringbuf_appendString(sb, st);
			my_stringbuf_appendString(sb, ")");
			free(st);
		}
	} else if (mknn_generalDomain_isCustom(domain->general_domain)) {
		my_stringbuf_appendString(sb, ":");
		my_stringbuf_appendInt(sb, domain->customobject.custom_id);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}

MknnDomain *mknn_domain_newParseString(const char *string_domain) {
	if (string_domain == NULL || strlen(string_domain) == 0)
		return NULL;
	MyTokenizer *tk = my_tokenizer_new(string_domain, ':');
	my_tokenizer_useBrackets(tk, '(', ')');
	MknnGeneralDomain dname = { 0 };
	if (!mknn_generalDomain_parseString(my_tokenizer_nextToken(tk), &dname))
		my_log_error("parse error domain %s\n", string_domain);
	MknnDomain *domain = NULL;
	if (mknn_generalDomain_isString(dname)) {
		domain = mknn_domain_newString();
	} else if (mknn_generalDomain_isVector(dname)) {
		int64_t num_dimensions = my_tokenizer_nextInt(tk);
		MknnDatatype dimension_datatype;
		if (!mknn_datatype_parseString(my_tokenizer_nextToken(tk),
				&dimension_datatype))
			my_log_error("parse error domain %s\n", string_domain);
		domain = mknn_domain_newVector(num_dimensions, dimension_datatype);
	} else if (mknn_generalDomain_isMultiObject(dname)) {
		int64_t length = my_tokenizer_nextInt(tk);
		MknnDomain **subdomains = MY_MALLOC(length, MknnDomain*);
		for (int64_t i = 0; i < length; ++i) {
			const char *string = my_tokenizer_nextToken(tk);
			subdomains[i] = mknn_domain_newParseString(string);
		}
		domain = mknn_domain_newMultiobject(length, subdomains, true);
		free(subdomains);
	} else if (mknn_generalDomain_isCustom(dname)) {
		int64_t custom_id = my_tokenizer_nextInt(tk);
		domain = mknn_domain_newCustomObject(custom_id, NULL);
	}
	my_tokenizer_releaseValidateEnd(tk);
	return domain;
}
MknnDomain *mknn_domain_newClone(MknnDomain *domain) {
	char *st = mknn_domain_toString(domain);
	MknnDomain *nd = mknn_domain_newParseString(st);
	free(st);
	return nd;
}
