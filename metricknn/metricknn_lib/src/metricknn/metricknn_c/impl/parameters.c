/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#define TYPE_STRING 1
#define TYPE_OBJECT 2

struct Value {
	char type;
	void *value;
};
struct MknnDistanceParams {
	char *id_distance;
	char *text_string;
	MyMapStringObj *map;
};
struct MknnIndexParams {
	char *id_index;
	char *text_string;
	MyMapStringObj *map;
};
struct MknnResolverParams {
	char *text_string;
	MyMapStringObj *map;
};

static void addToMap(MyMapStringObj *map, const char *name, void *value,
		char type) {
	struct Value *val = my_mapStringObj_get(map, name);
	if (val != NULL) {
		if (val->type == TYPE_STRING && val->value != NULL)
			free(val->value);
		val->value = value;
	} else {
		struct Value *val = MY_MALLOC(1, struct Value);
		val->type = type;
		val->value = value;
		my_mapStringObj_add(map, name, val);
	}
}
static void *getFromMap(MyMapStringObj *map, const char *name, char type) {
	struct Value *val = my_mapStringObj_get(map, name);
	if (val == NULL)
		return NULL;
	if (type != val->type)
		my_log_error("parameter %s does not match declared type\n", name);
	return val->value;
}

static void releaseMap(MyMapStringObj *map) {
	for (int64_t i = 0; i < my_mapStringObj_size(map); ++i) {
		struct Value *val = my_mapStringObj_getObjAt(map, i);
		if (val->type == TYPE_STRING)
			MY_FREE(val->value);
	}
	my_mapStringObj_release(map, true, true);
}
/////////////////////////////////////////////
static void parseArgs(MyMapStringObj *map, const char *parameters_string) {
	MyTokenizer *tk = my_tokenizer_new(parameters_string, ',');
	my_tokenizer_useBrackets(tk, '(', ')');
	while (my_tokenizer_hasNext(tk)) {
		const char *token = my_tokenizer_nextToken(tk);
		int64_t pos = my_string_indexOf(token, "=");
		if (pos > 0) {
			char *param_name = my_subStringI_fromTo(token, 0, pos);
			char *param_value = my_subStringI_fromEnd(token, pos + 1);
			addToMap(map, param_name, param_value, TYPE_STRING);
		} else {
			my_log_info("warning, invalid parameter %s\n", token);
		}
	}
	my_tokenizer_release(tk);
}
static char *parseString(MyMapStringObj *map, const char *parameters_string) {
	if (parameters_string == NULL || my_string_equals(parameters_string, ""))
		return NULL;
	int64_t pos_comma = my_string_indexOf(parameters_string, ",");
	int64_t pos_equal = my_string_indexOf(parameters_string, "=");
	char *header = NULL;
	if (pos_comma < 0 && pos_equal < 0) {
		header = my_newString_string(parameters_string);
	} else if (pos_comma < 0 && pos_equal >= 0) {
		parseArgs(map, parameters_string);
	} else if (pos_comma >= 0 && pos_equal < 0) {
		header = my_subStringI_fromTo(parameters_string, 0, pos_comma);
		parseArgs(map, parameters_string + pos_comma + 1);
	} else if (pos_comma >= 0 && pos_equal >= 0) {
		if (pos_equal < pos_comma) {
			parseArgs(map, parameters_string);
		} else {
			header = my_subStringI_fromTo(parameters_string, 0, pos_comma);
			parseArgs(map, parameters_string + pos_comma + 1);
		}
	}
	return header;
}
static void copyAll(MyMapStringObj *map_to, MyMapStringObj *map_from) {
	for (int64_t i = 0; i < my_mapStringObj_size(map_from); ++i) {
		const char *name = my_mapStringObj_getKeyAt(map_from, i);
		struct Value *value = my_mapStringObj_getObjAt(map_from, i);
		if (value->type == TYPE_STRING) {
			addToMap(map_to, my_newString_string(name),
					my_newString_string(value->value), TYPE_STRING);
		} else if (value->type == TYPE_OBJECT) {
			addToMap(map_to, my_newString_string(name), value->value,
			TYPE_OBJECT);
		}
	}
}
static char *mapToString(MyMapStringObj *map) {
	MyStringBuffer *sb = my_stringbuf_new();
	bool first = true;
	for (int64_t i = 0; i < my_mapStringObj_size(map); ++i) {
		const char *name = my_mapStringObj_getKeyAt(map, i);
		struct Value *val = my_mapStringObj_getObjAt(map, i);
		if (val->type != TYPE_STRING)
			continue;
		char *value = (char*) val->value;
		if (value == NULL || strlen(value) == 0)
			continue;
		if (!first)
			my_stringbuf_appendString(sb, ",");
		my_stringbuf_appendString(sb, name);
		my_stringbuf_appendString(sb, "=");
		if (my_string_indexOf(value, ",") >= 0
				|| my_string_indexOf(value, "=") >= 0) {
			my_stringbuf_appendString(sb, "(");
			my_stringbuf_appendString(sb, value);
			my_stringbuf_appendString(sb, ")");
		} else {
			my_stringbuf_appendString(sb, value);
		}
		first = false;
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
/********************************/
#define PARAM_FUNCTIONS(PTYPE, NAME) \
PTYPE *mknn_##NAME##_newEmpty() { \
	PTYPE *p = MY_MALLOC(1, PTYPE); \
	p->map = my_mapStringObj_newIgnoreCase(); \
	return p; \
} \
void mknn_##NAME##_release(PTYPE *p) { \
	if (p == NULL) \
		return; \
	releaseMap(p->map); \
	free(p); \
} \
void mknn_##NAME##_addString(PTYPE *p, const char *name, const char *value) { \
	if (p == NULL) \
		return; \
	addToMap(p->map, my_newString_string(name), my_newString_string(value), TYPE_STRING); \
} \
void mknn_##NAME##_addDouble(PTYPE *p, const char *name, double value) { \
	if (p == NULL) \
		return; \
	addToMap(p->map, my_newString_string(name), my_newString_double(value), TYPE_STRING); \
} \
void mknn_##NAME##_addInt(PTYPE *p, const char *name, int64_t value) { \
	if (p == NULL) \
		return; \
	addToMap(p->map, my_newString_string(name), my_newString_int(value), TYPE_STRING); \
} \
void mknn_##NAME##_addBool(PTYPE *p, const char *name, bool value) { \
	if (p == NULL) \
		return; \
	addToMap(p->map, my_newString_string(name), my_newString_bool(value), TYPE_STRING); \
} \
void mknn_##NAME##_addObject(PTYPE *p, const char *name, void *value) { \
	if (p == NULL) \
		return; \
	addToMap(p->map, my_newString_string(name), value, TYPE_OBJECT); \
} \
const char *mknn_##NAME##_getString(PTYPE *p, const char *name) { \
	if (p == NULL) \
		return NULL; \
	return getFromMap(p->map, name, TYPE_STRING); \
} \
double mknn_##NAME##_getDouble(PTYPE *p, const char *name) { \
	if (p == NULL) \
		return 0; \
	char *st = getFromMap(p->map, name, TYPE_STRING); \
	if (st == NULL) \
		return 0; \
	return my_parse_double(st); \
} \
int64_t mknn_##NAME##_getInt(PTYPE *p, const char *name) { \
	if (p == NULL) \
		return 0; \
	char *st = getFromMap(p->map, name, TYPE_STRING); \
	if (st == NULL) \
		return 0; \
	return my_parse_int(st); \
} \
bool mknn_##NAME##_getBool(PTYPE *p, const char *name) { \
	if (p == NULL) \
		return false; \
	char *st = getFromMap(p->map, name, TYPE_STRING); \
	if (st == NULL) \
		return false; \
	return my_parse_bool(st); \
} \
void *mknn_##NAME##_getObject(PTYPE *p, const char *name) { \
	if (p == NULL) \
		return NULL; \
	return getFromMap(p->map, name, TYPE_OBJECT); \
} \
void mknn_##NAME##_copyAll(PTYPE *p_to, PTYPE *p_from) { \
	if (p_to == NULL || p_from == NULL) \
		return; \
	copyAll(p_to->map, p_from->map); \
} \
/********************************/
PARAM_FUNCTIONS(MknnDistanceParams, distanceParams)
PARAM_FUNCTIONS(MknnIndexParams, indexParams)
PARAM_FUNCTIONS(MknnResolverParams, resolverParams)
/********************************/
void mknn_distanceParams_parseString(MknnDistanceParams *p,
		const char *parameters_string) {
	if (p == NULL)
		return;
	char *id = parseString(p->map, parameters_string);
	if (id != NULL) {
		mknn_distanceParams_setDistanceId(p, id);
		free(id);
	}
}
void mknn_indexParams_parseString(MknnIndexParams *p,
		const char *parameters_string) {
	if (p == NULL)
		return;
	char *id = parseString(p->map, parameters_string);
	if (id != NULL) {
		mknn_indexParams_setIndexId(p, id);
		free(id);
	}
}
void mknn_resolverParams_parseString(MknnResolverParams *p,
		const char *parameters_string) {
	if (p == NULL)
		return;
	char *id = parseString(p->map, parameters_string);
	if (id != NULL) {
		free(id);
	}
}
MknnDistanceParams *mknn_distanceParams_newParseString(
		const char *parameters_string) {
	MknnDistanceParams *p = mknn_distanceParams_newEmpty();
	mknn_distanceParams_parseString(p, parameters_string);
	return p;
}
char *concatIdParams(char *id, char *params) {
	bool withId = (id != NULL && strlen(id) > 0) ? true : false;
	bool withParams = (params != NULL && strlen(params) > 0) ? true : false;
	if (withId && withParams)
		return my_newString_format("%s,%s", id, params);
	if (withId && !withParams)
		return my_newString_format("%s", id);
	if (!withId && withParams)
		return my_newString_format(",%s", params);
	return my_newString_string("");
}
const char *mknn_distanceParams_toString(MknnDistanceParams *p) {
	if (p == NULL)
		return "";
	if (p->text_string != NULL)
		return p->text_string;
	char *stParams = mapToString(p->map);
	p->text_string = concatIdParams(p->id_distance, stParams);
	free(stParams);
	return p->text_string;
}
const char *mknn_indexParams_toString(MknnIndexParams *p) {
	if (p == NULL)
		return "";
	if (p->text_string != NULL)
		return p->text_string;
	char *stParams = mapToString(p->map);
	p->text_string = concatIdParams(p->id_index, stParams);
	free(stParams);
	return p->text_string;
}
const char *mknn_resolverParams_toString(MknnResolverParams *p) {
	if (p == NULL)
		return "";
	if (p->text_string != NULL)
		return p->text_string;
	p->text_string = mapToString(p->map);
	return p->text_string;
}
MknnIndexParams *mknn_indexParams_newParseString(const char *parameters_string) {
	MknnIndexParams *p = mknn_indexParams_newEmpty();
	mknn_indexParams_parseString(p, parameters_string);
	return p;
}
MknnResolverParams *mknn_resolverParams_newParseString(int64_t knn,
		double range, int64_t max_threads, const char *parameters_string) {
	MknnResolverParams *p = mknn_resolverParams_newEmpty();
	if (knn != 0)
		mknn_resolverParams_setKnn(p, knn);
	if (range != 0)
		mknn_resolverParams_setRange(p, range);
	if (max_threads != 0)
		mknn_resolverParams_setMaxThreads(p, max_threads);
	mknn_resolverParams_parseString(p, parameters_string);
	return p;
}

void mknn_distanceParams_setDistanceId(MknnDistanceParams *p,
		const char *id_dist) {
	if (p->id_distance != NULL)
		free(p->id_distance);
	p->id_distance = my_newString_string(id_dist);
}
const char *mknn_distanceParams_getDistanceId(MknnDistanceParams *p) {
	return p->id_distance;
}
void mknn_indexParams_setIndexId(MknnIndexParams *p, const char *id_index) {
	if (p->id_index != NULL)
		free(p->id_index);
	p->id_index = my_newString_string(id_index);
}
const char *mknn_indexParams_getIndexId(MknnIndexParams *p) {
	return p->id_index;
}

void mknn_resolverParams_setKnn(MknnResolverParams *p, int64_t knn) {
	mknn_resolverParams_addInt(p, "knn", knn);
}
int64_t mknn_resolverParams_getKnn(MknnResolverParams *p) {
	return mknn_resolverParams_getInt(p, "knn");
}
void mknn_resolverParams_setRange(MknnResolverParams *p, double range) {
	mknn_resolverParams_addDouble(p, "range", range);
}
double mknn_resolverParams_getRange(MknnResolverParams *p) {
	return mknn_resolverParams_getDouble(p, "range");
}
void mknn_resolverParams_setMaxThreads(MknnResolverParams *p,
		int64_t max_threads) {
	mknn_resolverParams_addInt(p, "max_threads", max_threads);
}
int64_t mknn_resolverParams_getMaxThreads(MknnResolverParams *p) {
	return mknn_resolverParams_getInt(p, "max_threads");
}
