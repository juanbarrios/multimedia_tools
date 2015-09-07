/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */

#include "tr.h"

#ifndef NO_OPENCV
struct Proceso_LOCAL {
	Extractor *ex;
};

static void tra_config_local(const char *trCode, const char *trParameters,
		void **out_state) {
	struct Proceso_LOCAL *es = MY_MALLOC(1, struct Proceso_LOCAL);
	es->ex = getExtractor(trParameters);
	DescriptorType td = getDescriptorType(es->ex);
	if (td.dtype != DTYPE_LOCAL_VECTORS)
		my_log_info("extractor must be local descriptors\n");
	*out_state = es;
}
static IplImage *tra_transformar_local(IplImage *imagen, int64_t numFrame,
		void* estado) {
	struct Proceso_LOCAL *es = estado;
	MyLocalDescriptors *ldes = extractVolatileDescriptor(es->ex, imagen);
	my_log_info("%s: %"PRIi64" vectors %s %"PRIi64"-d\n",
			getExtractorCodeAndParameters(es->ex),
			my_localDescriptors_getNumDescriptors(ldes),
			my_datatype_codeToDescription(
					my_localDescriptors_getVectorDatatype(ldes)),
			my_localDescriptors_getVectorDimensions(ldes));
	my_local_drawKeypoints(imagen, ldes);
	return imagen;
}
static void tra_release_local(void *estado) {
	struct Proceso_LOCAL *es = estado;
	releaseExtractor(es->ex);
	free(es);
}

void tra_reg_local() {
	Transform_Def *def = newTransformDef("LOCAL", "[descriptorLocal]");
	def->func_new = tra_config_local;
	def->func_transform_frame = tra_transformar_local;
	def->func_release = tra_release_local;
}
#endif
