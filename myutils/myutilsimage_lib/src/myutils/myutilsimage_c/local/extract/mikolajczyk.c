/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MultimediaTools. https://github.com/juanbarrios/multimedia_tools
 * MultimediaTools is made available under the terms of the BSD 2-Clause License.
 */

#include "mikolajczyk.h"
#include <myutils/myutils_c.h>

//Interest point detectors/descriptors implemented by K.Mikolajczyk
//at [ref. http://www.robots.ox.ac.uk/~vgg/research/affine]
//Downloaded from: http://kahlan.eps.surrey.ac.uk/featurespace/web/

struct MyMikolajczykOptions {
	struct {
		bool harris; // harris detector
		bool hessian; // hessian detector
		bool harmulti; // multi-scale harris detector
		bool hesmulti; // multi-scale hessian detector
		bool harhesmulti; // multi-scale harris-hessian detector
		bool harlap; // harris-laplace detector
		bool heslap; // hessian-laplace detector
		bool dog; // DoG detector
		bool mser; // mser detector
		bool haraff; // harris-affine detector
		bool hesaff; // hessian-affine detector
		bool harhes; // harris-hessian-laplace detector
		bool dense; //dense sampling
		double dense_dx, dense_dy;		//dense sampling
	} keypoints; //Interest points
	struct {
		double density; // feature density per pixels (1 descriptor per 100pix)
		double harThres; // harris threshold [100]
		double hesThres; // hessian threshold [200]
		double edgeLThres; // lower canny threshold [5]
		double edgeHThres; // higher canny threshold [10]
	} keypoints_params; //Interest points parameters
	struct {
		bool sift; // sift [D. Lowe]
		bool gloh; // gloh [KM]
	} descriptor; //Descriptors
	struct {
		bool color;	//color sift [KM]
		bool dradius;	//patch radius for computing descriptors at scale 1
		bool noangle;// computes rotation variant descriptors (no rotation esimation)
	} descriptor_params; //Descriptor paramenters
	struct {
		bool DP; //draws features as points in out.desc.png
		bool DC; //draws regions as circles in out.desc.png
		bool DE; //draws regions as ellipses in out.desc.png
		int gray; //-c 255 - draws points in grayvalue [0,...,255]
	} misc;
};

struct MyMikolajczykOptions *my_local_mikolajczyk_newOptionsCsift() {
	struct MyMikolajczykOptions *p = MY_MALLOC(1, struct MyMikolajczykOptions);
	p->keypoints.heslap = true;
	p->descriptor.sift = true;
	p->descriptor_params.color = true;
	return p;
}
struct MyMikolajczykOptions *my_local_mikolajczyk_newOptionsParse(
		const char *parameters) {
	struct MyMikolajczykOptions *options = MY_MALLOC(1,
			struct MyMikolajczykOptions);
	MyTokenizer *tk = my_tokenizer_new(parameters, '_');
	while (my_tokenizer_hasNext(tk)) {
		const char *opt = my_tokenizer_nextToken(tk);
		if (my_string_equals_ignorecase(opt, "HARRIS")) {
			options->keypoints.harris = true;
		} else if (my_string_equals_ignorecase(opt, "HESSIAN")) {
			options->keypoints.hessian = true;
		} else if (my_string_equals_ignorecase(opt, "HARMULTI")) {
			options->keypoints.harmulti = true;
		} else if (my_string_equals_ignorecase(opt, "HESMULTI")) {
			options->keypoints.hesmulti = true;
		} else if (my_string_equals_ignorecase(opt, "HARHESMULTI")) {
			options->keypoints.harhesmulti = true;
		} else if (my_string_equals_ignorecase(opt, "HARLAP")) {
			options->keypoints.harlap = true;
		} else if (my_string_equals_ignorecase(opt, "HESLAP")) {
			options->keypoints.heslap = true;
		} else if (my_string_equals_ignorecase(opt, "DOG")) {
			options->keypoints.dog = true;
		} else if (my_string_equals_ignorecase(opt, "MSER")) {
			options->keypoints.mser = true;
		} else if (my_string_equals_ignorecase(opt, "HARAFF")) {
			options->keypoints.haraff = true;
		} else if (my_string_equals_ignorecase(opt, "HARHES")) {
			options->keypoints.harhes = true;
		} else if (my_string_equals_ignorecase(opt, "DENSE")) {
			options->keypoints.dense = true;
			options->keypoints.dense_dx = my_tokenizer_nextDouble(tk);
			options->keypoints.dense_dy = my_tokenizer_nextDouble(tk);
		} else if (my_string_equals_ignorecase(opt, "SIFT")) {
			options->descriptor.sift = true;
		} else if (my_string_equals_ignorecase(opt, "GLOH")) {
			options->descriptor.gloh = true;
		} else if (my_string_equals_ignorecase(opt, "COLOR")) {
			options->descriptor_params.color = true;
		} else if (my_string_equals_ignorecase(opt, "NOANGLE")) {
			options->descriptor_params.noangle = true;
		} else {
			my_log_error("unknown option %s. help=%s\n",
					my_tokenizer_nextToken(tk),
					my_local_mikolajczyk_helpOptions());
		}
	}
	my_tokenizer_releaseValidateEnd(tk);
	if (!options->keypoints.harris && !options->keypoints.hessian
			&& !options->keypoints.harmulti && !options->keypoints.hesmulti
			&& !options->keypoints.harhesmulti && !options->keypoints.harlap
			&& !options->keypoints.heslap && !options->keypoints.dog
			&& !options->keypoints.mser && !options->keypoints.haraff
			&& !options->keypoints.harhes && !options->keypoints.dense)
		my_log_error("no keypoint defined. help=%s\n",
				my_local_mikolajczyk_helpOptions());
	if (!options->descriptor.sift && !options->descriptor.gloh)
		my_log_error("no descriptor defined. help=%s\n",
				my_local_mikolajczyk_helpOptions());
	return options;
}
const char *my_local_mikolajczyk_helpOptions() {
	return "(HARRIS|HESSIAN|HARMULTI|HESMULTI|HARHESMULTI|HARLAP|HESLAP|DOG|MSER|HARAFF|HARHES|DENSE_x_y)_(SIFT|GLOH)(_COLOR)(_NOANGLE)";
}
void my_local_mikolajczyk_releaseOptions(struct MyMikolajczykOptions *options) {
	if (options != NULL)
		free(options);
}

static char *getOptionsKeypoints(struct MyMikolajczykOptions *p) {
	MyStringBuffer *sb = my_stringbuf_new();
	if (p->keypoints.harris)
		my_stringbuf_appendString(sb, " -harris");
	if (p->keypoints.hessian)
		my_stringbuf_appendString(sb, " -hessian");
	if (p->keypoints.harmulti)
		my_stringbuf_appendString(sb, " -harmulti");
	if (p->keypoints.hesmulti)
		my_stringbuf_appendString(sb, " -hesmulti");
	if (p->keypoints.harhesmulti)
		my_stringbuf_appendString(sb, " -harhesmulti");
	if (p->keypoints.harlap)
		my_stringbuf_appendString(sb, " -harlap");
	if (p->keypoints.heslap)
		my_stringbuf_appendString(sb, " -heslap");
	if (p->keypoints.dog)
		my_stringbuf_appendString(sb, " -dog");
	if (p->keypoints.mser)
		my_stringbuf_appendString(sb, " -mser");
	if (p->keypoints.haraff)
		my_stringbuf_appendString(sb, " -haraff");
	if (p->keypoints.hesaff)
		my_stringbuf_appendString(sb, " -hesaff");
	if (p->keypoints.harhes)
		my_stringbuf_appendString(sb, " -harhes");
	if (p->keypoints.dense) {
		my_stringbuf_appendString(sb, " -dense ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints.dense_dx, 1);
		my_stringbuf_appendString(sb, " ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints.dense_dy, 1);
	}
	if (p->keypoints_params.density > 0) {
		my_stringbuf_appendString(sb, " -density ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints_params.density, 1);
	}
	if (p->keypoints_params.harThres > 0) {
		my_stringbuf_appendString(sb, " -harThres ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints_params.harThres, 1);
	}
	if (p->keypoints_params.hesThres > 0) {
		my_stringbuf_appendString(sb, " -hesThres ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints_params.hesThres, 1);
	}
	if (p->keypoints_params.edgeLThres > 0) {
		my_stringbuf_appendString(sb, " -edgeLThres ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints_params.edgeLThres, 1);
	}
	if (p->keypoints_params.edgeHThres > 0) {
		my_stringbuf_appendString(sb, " -edgeHThres ");
		my_stringbuf_appendDoubleRound(sb, p->keypoints_params.edgeHThres, 1);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
static char *getOptionsDescriptors(struct MyMikolajczykOptions *p) {
	MyStringBuffer *sb = my_stringbuf_new();
	if (p->descriptor.sift)
		my_stringbuf_appendString(sb, " -sift");
	if (p->descriptor.gloh)
		my_stringbuf_appendString(sb, " -gloh");
	if (p->descriptor_params.color)
		my_stringbuf_appendString(sb, " -color");
	if (p->descriptor_params.dradius)
		my_stringbuf_appendString(sb, " -dradius");
	if (p->descriptor_params.noangle)
		my_stringbuf_appendString(sb, " -noangle");
	if (p->misc.DP)
		my_stringbuf_appendString(sb, " -DP");
	if (p->misc.DC)
		my_stringbuf_appendString(sb, " -DC");
	if (p->misc.DE)
		my_stringbuf_appendString(sb, " -DE");
	if (p->misc.gray > 0) {
		my_stringbuf_appendString(sb, " -c ");
		my_stringbuf_appendInt(sb, p->misc.gray);
	}
	return my_stringbuf_releaseReturnBuffer(sb);
}
static double get_radius(double a, double b, double c) {
	//a b c in ellipsis a(x^2)+2b(xy)+c(y^2)=1
	if (a != 0 && b == 0 && c != 0)
		return sqrt(MAX(1 / a, 1 / c)) / 2;
	else if (a != 0 && b != 0 && c != 0)
		return sqrt(MAX(1 / a, 1 / c)) / 2;
	else
		return 0;
}
static void parse_file_mikolajczyk(char *filenameTxt,
		MyLocalDescriptors *descriptors) {
	if (!my_io_existsFile(filenameTxt))
		return;
	MyVectorString *lines = my_io_loadLinesFile(filenameTxt);
	if (my_vectorString_size(lines) < 3)
		return;
	int64_t dimension = my_parse_int(my_vectorString_get(lines, 0));
	int64_t numDesc = my_parse_int(my_vectorString_get(lines, 1));
	my_assert_equalInt("numLines", numDesc, my_vectorString_size(lines) - 2);
	my_localDescriptors_redefineVectorDatatype(descriptors, MY_DATATYPE_UINT8,
			dimension, numDesc);
	uint8_t vector[dimension];
	for (int64_t i = 0; i < my_vectorString_size(lines) - 2; ++i) {
		char *line = my_vectorString_get(lines, i + 2);
		MyVectorString *values = my_tokenizer_splitLine(line, ' ');
		my_assert_equalInt("line length", my_vectorString_size(values),
				dimension + 5);
		double x = my_parse_double(my_vectorString_get(values, 0));
		double y = my_parse_double(my_vectorString_get(values, 1));
		double a = my_parse_double(my_vectorString_get(values, 2));
		double b = my_parse_double(my_vectorString_get(values, 3));
		double c = my_parse_double(my_vectorString_get(values, 4));
		for (int64_t j = 0; j < dimension; ++j)
			vector[j] = my_parse_uint8(my_vectorString_get(values, j + 5));
		my_vectorString_release(values, true);
		my_localDescriptors_setKeypoint(descriptors, i, x, y,
				get_radius(a, b, c), 0);
		my_localDescriptors_setVector(descriptors, i, vector);
	}
	my_vectorString_release(lines, true);
}
static void extract_mikolajczyk(const char *filename,
		MyLocalDescriptors *descriptors, struct MyMikolajczykOptions *options) {
	my_localDescriptors_redefineNumDescriptors(descriptors, 0);
	const char *binary = my_env_getString_def2("COMPUTEDESCRIPTORS",
			"compute_descriptors.exe", "compute_descriptors_64bit.ln");
	char *optionsPoints = getOptionsKeypoints(options);
	char *optionsDesc = getOptionsDescriptors(options);
	char *fPoints = my_newString_format("%s.points", filename);
	char *fDesc = my_newString_format("%s.desc", filename);
	char *fLogPoints = my_newString_format("%s.points.log", filename);
	char *fLogDesc = my_newString_format("%s.desc.log", filename);
	my_io_deleteFile(fPoints, false);
	my_io_deleteFile(fDesc, false);
	my_io_deleteFile(fLogPoints, false);
	my_io_deleteFile(fLogDesc, false);
	char *cmd1 = my_newString_format("%s -i %s %s -o1 %s > %s 2>&1", binary,
			filename, optionsPoints, fPoints, fLogPoints);
	char *cmd2 = my_newString_format("%s -i %s %s -p1 %s -o1 %s > %s 2>&1",
			binary, filename, optionsDesc, fPoints, fDesc, fLogDesc);
	my_io_system(cmd1);
	my_io_system(cmd2);
	parse_file_mikolajczyk(fDesc, descriptors);
	my_io_deleteFile(fPoints, false);
	my_io_deleteFile(fDesc, false);
	my_io_deleteFile(fLogPoints, false);
	my_io_deleteFile(fLogDesc, false);
	MY_FREE_MULTI(optionsPoints, optionsDesc, fPoints, fDesc, fLogPoints,
			fLogDesc, cmd1, cmd2);
}

void my_local_mikolajczyk_extract_filename(const char *filename,
		MyLocalDescriptors *descriptors, struct MyMikolajczykOptions *options) {
	extract_mikolajczyk(filename, descriptors, options);
}

#ifndef NO_OPENCV
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

void my_local_mikolajczyk_extract(IplImage *image,
		MyLocalDescriptors *descriptors, struct MyMikolajczykOptions *options) {
	char *filename = my_io_temp_createNewFile(".png");
	cvSaveImage(filename, image, NULL);
	extract_mikolajczyk(filename, descriptors, options);
	my_io_deleteFile(filename, false);
	free(filename);
}
#endif
