/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "histogram.h"

struct MknnHistogram {
	double max_dist, bin_width;
	bool flag_int_dists;
	int64_t num_bins;
	double *mins, *maxs;
	int64_t n;
	double *contBins;
	double mean, M2;
	double *tabla_conversion;
	int64_t tabla_conversion_max;
};

static bool hist_testIntegerValues(int64_t num_values, double *values) {
	for (int64_t i = 0; i < num_values; ++i) {
		int64_t value_int = (int64_t) values[i];
		if (value_int != values[i] || value_int < 0 || value_int >= 100000000)
			return false;
	}
	return true;
}
static double hist_getMaxDist(int64_t num_values, double *values) {
	double max = values[0];
	for (int64_t i = 0; i < num_values; ++i) {
		if (values[i] > max)
			max = values[i];
	}
	return max;
}
MknnHistogram *mknn_histogram_new(int64_t num_values, double *values,
		int64_t max_bins) {
	MknnHistogram *hist = MY_MALLOC(1, MknnHistogram);
	hist->max_dist = hist_getMaxDist(num_values, values);
	hist->flag_int_dists = hist_testIntegerValues(num_values, values);
	if (hist->flag_int_dists) {
		int64_t numVals = 1 + ((int64_t) hist->max_dist);
		int64_t iwidth = numVals / max_bins;
		if (numVals % max_bins > 0)
			iwidth++;
		hist->bin_width = iwidth;
		hist->num_bins = numVals / iwidth;
		if (numVals % iwidth > 0)
			hist->num_bins++;
	} else {
		hist->bin_width = hist->max_dist / max_bins;
		hist->num_bins = my_math_round_int(hist->max_dist / hist->bin_width);
	}
	hist->contBins = MY_MALLOC(hist->num_bins, double);
	hist->mins = MY_MALLOC(hist->num_bins, double);
	hist->maxs = MY_MALLOC(hist->num_bins, double);
	for (int64_t i = 0; i < hist->num_bins; ++i) {
		hist->mins[i] = DBL_MAX;
		hist->maxs[i] = -DBL_MAX;
	}
	//add values to bins
	for (int64_t i = 0; i < num_values; ++i)
		mknn_histogram_addValue(hist, values[i]);
	return hist;
}
MknnHistogram *mknn_histogram_restore(const char *filenameHist, bool fail) {
	MyLineReader *reader = my_lreader_config_open(
			my_io_openFileRead1(filenameHist, fail), "MetricKnn",
			"MknnHistogram", 1, 1);
	if (reader == NULL)
		return NULL;
	const char *line = my_lreader_readLine(reader);
	MyTokenizer *tk = my_tokenizer_new(line, '\t');
	MknnHistogram *ch = MY_MALLOC(1, MknnHistogram);
	ch->max_dist = my_tokenizer_nextDouble(tk);
	ch->bin_width = my_tokenizer_nextDouble(tk);
	ch->flag_int_dists = (my_tokenizer_nextInt(tk) > 0) ? true : false;
	ch->num_bins = my_tokenizer_nextInt(tk);
	ch->mins = MY_MALLOC(ch->num_bins, double);
	ch->maxs = MY_MALLOC(ch->num_bins, double);
	ch->contBins = MY_MALLOC(ch->num_bins, double);
	ch->n = my_tokenizer_nextInt(tk);
	ch->mean = my_tokenizer_nextDouble(tk);
	ch->M2 = my_tokenizer_nextDouble(tk);
	ch->tabla_conversion = NULL;
	ch->tabla_conversion_max = 0;
	my_tokenizer_releaseValidateEnd(tk);
	while ((line = my_lreader_readLine(reader)) != NULL) {
		tk = my_tokenizer_new(line, '\t');
		int64_t i = my_tokenizer_nextInt(tk);
		const char *ss = my_tokenizer_nextToken(tk);
		if (my_string_equals(ss, "\"\""))
			ch->mins[i] = DBL_MAX;
		else
			ch->mins[i] = my_parse_double(ss);
		ss = my_tokenizer_nextToken(tk);
		if (my_string_equals(ss, "\"\""))
			ch->maxs[i] = -DBL_MAX;
		else
			ch->maxs[i] = my_parse_double(ss);
		ch->contBins[i] = my_tokenizer_nextDouble(tk);
		my_tokenizer_release(tk);
	}
	my_lreader_close(reader, true);
	return ch;
}

void mknn_histogram_addValue(MknnHistogram *hist, double value) {
	if (value > hist->max_dist)
		hist->max_dist = value;
	int64_t pos = (int64_t) (value / hist->bin_width);
	if (pos < 0)
		pos = 0;
	else if (pos >= hist->num_bins)
		pos = hist->num_bins - 1;
	hist->contBins[pos] += 1;
	if (value < hist->mins[pos])
		hist->mins[pos] = value;
	if (value > hist->maxs[pos])
		hist->maxs[pos] = value;
	//media y varianza online: Knuth, the ACP, vol.2, pag 232
	//http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#On-line_algorithm
	hist->n++;
	double delta = value - hist->mean;
	hist->mean += delta / hist->n;
	hist->M2 += delta * (value - hist->mean);
}
static double priv_buscarPercentil(MknnHistogram *hist, double value) {
	if (value >= hist->max_dist)
		return 1;
	//busco la posicion igual que en addDistancia
	int64_t pos = (int64_t) (value / hist->bin_width);
	if (pos < 0)
		pos = 0;
	else if (pos >= hist->num_bins)
		pos = hist->num_bins - 1;
	//
	double cantPrev = 0;
	for (int64_t i = 0; i < pos; ++i)
		cantPrev += hist->contBins[i];
	//calculo la fraccion del bin i que le corresponde
	if (hist->flag_int_dists) {
		int64_t idist = (int64_t) value;
		if (idist == value) {
			//distancias enteras la cota inferior de bin se lleva 1/binwidth del total
			//ej, si el bin contiene las distancias 15,16,17 (binwidth=3)
			//entonces 15 sum 1/3 del contBins, 16 sum 2/3 y 17 sum los 3/3
			//asi, si hay muchas distancias 0 el cero debe perder significancia
			//y no retornar el percentil 0, si no que aumente segun el numero de
			//distancias ceros en el histogram (a diferencia del caso distancias decimales).
			int64_t iwidth = (int64_t) hist->bin_width;
			double fracBin = ((idist % iwidth) + 1) / hist->bin_width;
			return (cantPrev + hist->contBins[pos] * fracBin)
					/ ((double) hist->n);
		}
	}
	double fracBin = (value / hist->bin_width) - pos;
	return (cantPrev + hist->contBins[pos] * fracBin) / ((double) hist->n);
}
double mknn_histogram_getQuantile(MknnHistogram *hist, double value) {
	if (hist->flag_int_dists) {
		if (hist->tabla_conversion == NULL) {
			hist->tabla_conversion_max = (int64_t) hist->max_dist;
			hist->tabla_conversion = MY_MALLOC_NOINIT(
					hist->tabla_conversion_max + 1, double);
			for (int64_t i = 0; i <= hist->tabla_conversion_max; ++i)
				hist->tabla_conversion[i] = priv_buscarPercentil(hist, i);
		}
		int64_t idist = (int64_t) value;
		if (idist == value)
			return hist->tabla_conversion[MIN(idist, hist->tabla_conversion_max)];
	}
	return priv_buscarPercentil(hist, value);
}
double mknn_histogram_getValueQuantile(MknnHistogram *hist, double quantile) {
	if (quantile >= 1.0)
		return hist->max_dist;
	double sumaPrev = 0, sum = 0;
	int64_t sumaBuscada = my_math_round_int(quantile * hist->n);
	//selecciono el threshold mirando los maximos entre cada bin
	double maxPrev = 0;
	for (int64_t i = 0; i < hist->num_bins; ++i) {
		if (hist->contBins[i] == 0)
			continue;
		sum += hist->contBins[i];
		if (sumaPrev <= sumaBuscada && sumaBuscada < sum) {
			return maxPrev
					+ (hist->maxs[i] - maxPrev) * (sumaBuscada - sumaPrev)
							/ ((double) (sum - sumaPrev));
		}
		sumaPrev = sum;
		maxPrev = hist->maxs[i];
	}
	return hist->max_dist;
}
double* mknn_histogram_getBins(MknnHistogram *ch) {
	return ch->contBins;
}
int64_t mknn_histogram_getNumBins(MknnHistogram *ch) {
	return ch->num_bins;
}

void mknn_histogram_save(MknnHistogram *ch, const char *filename) {
	FILE *out = my_io_openFileWrite1Config(filename, "MetricKnn",
			"MknnHistogram", 1, 1);
	double varianza = ch->M2 / ch->n;
	double dimen_intrin = (ch->mean * ch->mean) / (2 * varianza);
	fprintf(out, "#computed distances\t%"PRIi64"\n", ch->n);
	if (ch->flag_int_dists)
		fprintf(out, "#max distance\t%"PRIi64"\n", (int64_t) ch->max_dist);
	else
		fprintf(out, "#max distance\t%1.14lf\n", ch->max_dist);
	fprintf(out, "#median\t%1.14lf\n", ch->mean);
	fprintf(out, "#variance\t%1.14lf\n", varianza);
	fprintf(out, "#intrinsic dimension\t%1.14lf\n", dimen_intrin);
	fprintf(out, "#bins\t%"PRIi64"\n", ch->num_bins);
	double sum = (double) ch->n;
	int64_t max_bin = 0;
	double max_cont = 0, max_acumulado = 0, cant_acumulado = 0;
	for (int64_t i = 0; i < ch->num_bins; ++i) {
		cant_acumulado += ch->contBins[i];
		if (ch->contBins[i] > max_cont) {
			max_cont = ch->contBins[i];
			max_acumulado = cant_acumulado;
			max_bin = i;
		}
	}
	if (ch->flag_int_dists)
		fprintf(out,
				"#peak bin\tbin=%"PRIi64"\td1=%1.0lf\td2=%1.0lf\tcont=%lf\tpct=%lf\tacum=%lf\n",
				max_bin, ch->mins[max_bin], ch->maxs[max_bin], max_cont,
				max_cont / sum, max_acumulado / sum);
	else
		fprintf(out,
				"#peak bin\tbin=%"PRIi64"\td1=%lf\td2=%lf\tcont=%lf\tpct=%lf\tacum=%lf\n",
				max_bin, ch->mins[max_bin], ch->maxs[max_bin], max_cont,
				max_cont / sum, max_acumulado / sum);
	fprintf(out, "#alpha 1\t%1.14lf\n", mknn_histogram_getValueQuantile(ch, 1));
	fprintf(out, "#alpha 0.5\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.5));
	fprintf(out, "#alpha 0.1\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.1));
	fprintf(out, "#alpha 0.01\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.01));
	fprintf(out, "#alpha 0.001\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.001));
	fprintf(out, "#alpha 0.0001\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.0001));
	fprintf(out, "#alpha 0.00001\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.00001));
	fprintf(out, "#alpha 0.000001\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 0.000001));
	fprintf(out,
			"#alphas\t%1.14lf\t%1.14lf\t%1.14lf\t%1.14lf\t%1.14lf\t%1.14lf\t%1.14lf\t%1.14lf\n",
			mknn_histogram_getValueQuantile(ch, 1),
			mknn_histogram_getValueQuantile(ch, 0.5),
			mknn_histogram_getValueQuantile(ch, 0.1),
			mknn_histogram_getValueQuantile(ch, 0.01),
			mknn_histogram_getValueQuantile(ch, 0.001),
			mknn_histogram_getValueQuantile(ch, 0.0001),
			mknn_histogram_getValueQuantile(ch, 0.00001),
			mknn_histogram_getValueQuantile(ch, 0.000001));
	fprintf(out, "#\n");
	//header
	fprintf(out,
			"#max_dist\tbin_width\tflag_int_dists\tnum_bins\tn\tmean\tM2\n");
	if (ch->flag_int_dists)
		fprintf(out,
				"%1.0lf\t%1.0lf\t%u\t%"PRIi64"\t%"PRIi64"\t%1.14lf\t%1.14lf\n",
				ch->max_dist, ch->bin_width, ch->flag_int_dists, ch->num_bins,
				ch->n, ch->mean, ch->M2);
	else
		fprintf(out,
				"%1.14lf\t%1.14lf\t%u\t%"PRIi64"\t%"PRIi64"\t%1.14lf\t%1.14lf\n",
				ch->max_dist, ch->bin_width, ch->flag_int_dists, ch->num_bins,
				ch->n, ch->mean, ch->M2);
	//bins
	fprintf(out,
			"#bin_number\tinterval_start\tinterval_end\tamount\tpct\taccumulated\taccumulated_pct\n");
	cant_acumulado = 0;
	for (int64_t i = 0; i < ch->num_bins; ++i) {
		if (ch->contBins[i] == 0) {
			fprintf(out, "%"PRIi64"\t\"\"\t\"\"\t0\t0\t%lf\t%1.14lf\n", i,
					cant_acumulado, cant_acumulado / sum);
		} else {
			cant_acumulado += ch->contBins[i];
			if (ch->flag_int_dists)
				fprintf(out,
						"%"PRIi64"\t%1.0lf\t%1.0lf\t%lf\t%1.14lf\t%lf\t%1.14lf\n",
						i, ch->mins[i], ch->maxs[i], ch->contBins[i],
						ch->contBins[i] / sum, cant_acumulado,
						cant_acumulado / sum);
			else
				fprintf(out,
						"%"PRIi64"\t%1.14lf\t%1.14lf\t%lf\t%1.14lf\t%lf\t%1.14lf\n",
						i, ch->mins[i], ch->maxs[i], ch->contBins[i],
						ch->contBins[i] / sum, cant_acumulado,
						cant_acumulado / sum);
		}
	}
	fclose(out);
}
void mknn_histogram_release(MknnHistogram *ch) {
	MY_FREE(ch->contBins);
	MY_FREE(ch->mins);
	MY_FREE(ch->maxs);
	MY_FREE(ch->tabla_conversion);
	MY_FREE(ch);
}
