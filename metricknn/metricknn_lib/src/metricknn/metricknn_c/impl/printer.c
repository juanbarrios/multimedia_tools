/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

struct MknnPrinter {
	bool is_vector, is_string, is_multiobject;
	struct {
		int64_t length;
		my_function_to_string func_to_string;
		char *prefix, *separator, *suffix;
	} vector;
	struct {
		int64_t length;
		MknnPrinter **subprinters;
		char *prefix, *separator, *suffix;
	} multiobject;
};

MknnPrinter *mknn_printer_new(MknnDomain *domain) {
	MknnPrinter *printer = MY_MALLOC(1, MknnPrinter);
	if (mknn_domain_isGeneralDomainString(domain)) {
		printer->is_string = true;
	} else if (mknn_domain_isGeneralDomainVector(domain)) {
		printer->is_vector = true;
		printer->vector.length = mknn_domain_vector_getNumDimensions(domain);
		printer->vector.func_to_string = my_datatype_getFunctionToString(
				mknn_datatype_convertMknn2My(
						mknn_domain_vector_getDimensionDataType(domain)));
		printer->vector.prefix = my_newString_string("");
		printer->vector.separator = my_newString_string("\t");
		printer->vector.suffix = my_newString_string("");
	} else if (mknn_domain_isGeneralDomainMultiObject(domain)) {
		printer->is_multiobject = true;
		printer->multiobject.length = mknn_domain_multiobject_getLength(domain);
		printer->multiobject.subprinters = MY_MALLOC(
				printer->multiobject.length, MknnPrinter*);
		for (int64_t i = 0; i < printer->multiobject.length; ++i) {
			MknnDomain *subdomain = mknn_domain_multiobject_getSubDomain(domain,
					i);
			printer->multiobject.subprinters[i] = mknn_printer_new(subdomain);
		}
		printer->multiobject.prefix = my_newString_string("{");
		printer->multiobject.separator = my_newString_string(",");
		printer->multiobject.suffix = my_newString_string("}");
	} else {
		my_log_error("unknown domain %s for printing\n",
				mknn_generalDomain_toString(
						mknn_domain_getGeneralDomain(domain)));
	}
	return printer;
}

static char *multiobject_to_string(void *object, int64_t length,
		MknnPrinter **subprinters, const char *prefix, const char *separator,
		const char *suffix) {
	MyStringBuffer *sb = my_stringbuf_new();
	if (prefix != NULL)
		my_stringbuf_appendString(sb, prefix);
	void **multiobject = (void**) object;
	for (int64_t i = 0; i < length; ++i) {
		if (i > 0 && separator != NULL)
			my_stringbuf_appendString(sb, separator);
		char *st = mknn_printer_objectToNewString(subprinters[i],
				multiobject[i]);
		my_stringbuf_appendString(sb, st);
		free(st);
	}
	if (suffix != NULL)
		my_stringbuf_appendString(sb, suffix);
	return my_stringbuf_releaseReturnBuffer(sb);
}

char *mknn_printer_objectToNewString(MknnPrinter *printer, void *object) {
	if (printer->is_string) {
		char *string = (char*) object;
		return my_newString_string(string);
	} else if (printer->is_vector) {
		return printer->vector.func_to_string(object, printer->vector.length,
				printer->vector.prefix, printer->vector.separator,
				printer->vector.suffix);
	} else if (printer->is_multiobject) {
		return multiobject_to_string(object, printer->multiobject.length,
				printer->multiobject.subprinters, printer->multiobject.prefix,
				printer->multiobject.separator, printer->multiobject.suffix);
	} else {
		return NULL;
	}
}

void mknn_printer_setVectorFormat(MknnPrinter *printer,
		const char *vector_prefix, const char *dimension_separator,
		const char *vector_suffix) {
	if (printer->is_vector) {
		MY_FREE_MULTI(printer->vector.prefix, printer->vector.separator,
				printer->vector.suffix);
		printer->vector.prefix = my_newString_string(vector_prefix);
		printer->vector.separator = my_newString_string(dimension_separator);
		printer->vector.suffix = my_newString_string(vector_suffix);
	} else if (printer->is_multiobject) {
		//pass-through to subprinters
		for (int64_t i = 0; i < printer->multiobject.length; ++i) {
			MknnPrinter *subprinter = printer->multiobject.subprinters[i];
			mknn_printer_setVectorFormat(subprinter, vector_prefix,
					dimension_separator, vector_suffix);
		}
	}
}
void mknn_printer_setMultiobjectFormat(MknnPrinter *printer, const char *prefix,
		const char *separator, const char *suffix) {
	if (printer->is_multiobject) {
		MY_FREE_MULTI(printer->multiobject.prefix,
				printer->multiobject.separator, printer->multiobject.suffix);
		printer->multiobject.prefix = my_newString_string(prefix);
		printer->multiobject.separator = my_newString_string(separator);
		printer->multiobject.suffix = my_newString_string(suffix);
	}
}

void mknn_printer_release(MknnPrinter *printer) {
	if (printer == NULL)
		return;
	if (printer->is_vector) {
		MY_FREE_MULTI(printer->vector.prefix, printer->vector.separator,
				printer->vector.suffix);
	} else if (printer->is_multiobject) {
		MY_FREE_MULTI(printer->multiobject.prefix,
				printer->multiobject.separator, printer->multiobject.suffix);
		for (int64_t i = 0; i < printer->multiobject.length; ++i) {
			MknnPrinter *subprinter = printer->multiobject.subprinters[i];
			mknn_printer_release(subprinter);
		}
		free(printer->multiobject.subprinters);
	}
	free(printer);
}
