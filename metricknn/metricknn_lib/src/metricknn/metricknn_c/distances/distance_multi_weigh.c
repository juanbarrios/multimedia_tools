/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of MetricKnn. http://metricknn.org/
 * MetricKnn is made available under the terms of the BSD 2-Clause License.
 */

#include "../metricknn_impl.h"

#if 0
struct A {
	//automatic configuration
	char *autoWeight, **autoNorm;
	double *normalization, *fraction;
	MknnDataset *colQuery, *colReference;
	char *path_profile;
};

static void priv_correctWeights(struct State_Multimetric *es) {
	double sum = 0;
	for (int64_t i = 0; i < es->numDistances; ++i) {
		if (es->fraction[i] < 0)
		es->fraction[i] = -es->fraction[i];
		sum += es->fraction[i];
	}
	for (int64_t i = 0; i < es->numDistances; ++i) {
		es->fraction[i] /= sum;
		es->weights[i] = es->normalization[i] * es->fraction[i];
	}
}
static void priv_printIndicator(struct State_Multimetric *es,
		char *nomIndicador, char *st_fase, double valIndicador) {
	if (st_fase != NULL)
	my_log_info("[%s] ", st_fase);
	for (int64_t i = 0; i < es->numDistances; ++i) {
		char *norm = my_newString_double(es->normalization[i]);
		char *pond = my_newString_double(es->fraction[i]);
		if (i > 0)
		my_log_info(" + ");
		my_log_info("%s*%s*%s", es->distances[i]->name, norm, pond);
		MY_FREE_MULTI(norm, pond);
	}
	char *val = my_newString_double(valIndicador);
	if (nomIndicador != NULL)
	my_log_info(" => %s=%s", nomIndicador, val);
	my_log_info("\n");
	MY_FREE(val);
}

static double priv_calcularValorIndicador(Distance* fd, char *nomIndicador,
		int64_t numSamples, int64_t multiplicadorFase) {
	struct State_Multimetric *es = fd->state_dist;
	double *sample = computeDistanceSample(es->colQuery, es->colReference, fd,
			numSamples * multiplicadorFase, "Q", "R");
	struct MyDataStats stats = my_math_computeStats(numSamples, sample);
	struct MyQuantiles quant = my_math_computeQuantiles(numSamples, sample);
	MY_FREE(sample);
	double indicador = 0;
	if (my_string_equals_ignorecase(nomIndicador, "RHO"))
	indicador = stats.rho;
	else if (my_string_equals_ignorecase(nomIndicador, "VAR"))
	indicador = stats.variance;
	else if (my_string_equals_ignorecase(nomIndicador, "A1"))
	indicador = quant.a1;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.5"))
	indicador = quant.a0_5;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.1"))
	indicador = quant.a0_1;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.01"))
	indicador = quant.a0_01;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.001"))
	indicador = quant.a0_001;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.0001"))
	indicador = quant.a0_0001;
	else if (my_string_equals_ignorecase(nomIndicador, "A0.00001"))
	indicador = quant.a0_00001;
	else
	my_log_error("indicador %s desconocido\n", nomIndicador);
	return indicador;
}
#define PRECISION_PESOS 1000
static bool priv_adjustFractions(struct State_Multimetric *params,
		int64_t idPeso, int64_t direccion, double epsilon) {
	if (direccion == 0)
	return 1;
	double newVal = params->fraction[idPeso] + epsilon * direccion;
	int64_t pesoI = my_math_round_int(newVal * ((double) PRECISION_PESOS));
	if (pesoI <= 0 || pesoI >= PRECISION_PESOS)
	return 0;
	int64_t *pesosI = MY_MALLOC_NOINIT(params->numDistances, int64_t);
	pesosI[idPeso] = pesoI;
	double sumaR = 0;
	for (int64_t i = 0; i < params->numDistances; ++i)
	if (i != idPeso)
	sumaR += params->fraction[i];
	for (int64_t i = 0; i < params->numDistances; ++i) {
		if (i != idPeso)
		pesosI[i] = my_math_round_int(
				(params->fraction[i] / sumaR)
				* ((double) (PRECISION_PESOS - pesoI)));
	}
	for (;;) {
		int64_t sumaI = 0;
		for (int64_t i = 0; i < params->numDistances; ++i)
		sumaI += pesosI[i];
		if (sumaI == PRECISION_PESOS)
		break;
		int64_t diff = sumaI - PRECISION_PESOS;
		for (int64_t i = 0; diff != 0 && i < params->numDistances; ++i) {
			if (i == idPeso)
			continue;
			if (diff > 0) {
				pesosI[i]--;
				diff--;
			} else if (diff < 0) {
				pesosI[i]++;
				diff++;
			}
		}
	}
	for (int64_t i = 0; i < params->numDistances; ++i)
	params->fraction[i] = pesosI[i] / ((double) PRECISION_PESOS);
	priv_correctWeights(params);
	return 1;
}
struct Valor {
	double *weights;
	double indicador;
	int64_t multiplicadorFase;
};
struct Historia {
	int64_t numPesos;
	MyVectorObj *historia;
};
static struct Historia *newHistoria(int64_t numPesos) {
	struct Historia *hst = MY_MALLOC(1, struct Historia);
	hst->numPesos = numPesos;
	hst->historia = my_vectorObj_new();
	return hst;
}
static void releaseHistoria(struct Historia *hst) {
	for (int64_t i = 0; i < my_vectorObj_size(hst->historia); ++i) {
		struct Valor *value = my_vectorObj_get(hst->historia, i);
		MY_FREE_MULTI(value->weights, value);
	}
	my_vectorObj_release(hst->historia, 0);
	MY_FREE(hst);
}
static void add_to_historia(struct Historia *hst, double *weights,
		double indicador, int64_t multiplicadorFase) {
	struct Valor *value = MY_MALLOC_NOINIT(1, struct Valor);
	value->weights = MY_MALLOC_NOINIT(hst->numPesos, double);
	for (int64_t i = 0; i < hst->numPesos; ++i)
	value->weights[i] = weights[i];
	value->indicador = indicador;
	value->multiplicadorFase = multiplicadorFase;
	my_vectorObj_add(hst->historia, value);
}
static double priv_promediar_historia(struct Historia *hst, double *weights) {
	MyVectorDouble *indicadores = my_vectorDouble_new();
	int64_t cont = 0;
	for (int64_t i = 0; i < my_vectorObj_size(hst->historia); ++i) {
		struct Valor *value = my_vectorObj_get(hst->historia, i);
		bool todoIgual = 1;
		for (int64_t j = 0; j < hst->numPesos; ++j) {
			if (value->weights[j] != weights[j]) {
				todoIgual = 0;
				break;
			}
		}
		if (todoIgual) {
			cont++;
			for (int64_t j = 0; j < value->multiplicadorFase; ++j)
			my_vectorDouble_add(indicadores, value->indicador);
		}
	}
	my_assert_greaterInt("value history", my_vectorDouble_size(indicadores), 0);
	double avgIndicador = my_math_averageDoubleArray(
			my_vectorDouble_size(indicadores),
			my_vectorDouble_array(indicadores));
	if (cont > 1)
	my_log_info("average %"PRIi64" (%"PRIi64" vals) => %lf\n",
			my_vectorDouble_size(indicadores), cont, avgIndicador);
	my_vectorDouble_release(indicadores);
	return avgIndicador;
}
#define IND_INVALID -DBL_MAX
static double priv_calcularIndicador(Distance *fd, int64_t idPeso,
		int64_t direccion, char *nomIndicador, char *st_fase,
		struct Historia *historia, int64_t numSamples,
		int64_t multiplicadorFase, double epsilonFase) {
	struct State_Multimetric *es = fd->state_dist;
	double *prev_ponder = MY_MALLOC_NOINIT(es->numDistances, double);
	for (int64_t i = 0; i < es->numDistances; ++i)
	prev_ponder[i] = es->fraction[i];
	double newIndic = IND_INVALID;
	if (priv_adjustFractions(es, idPeso, direccion, epsilonFase)) {
		double indicador = priv_calcularValorIndicador(fd, nomIndicador,
				numSamples, multiplicadorFase);
		my_log_info("original %s => %lf\n", nomIndicador, indicador);
		add_to_historia(historia, es->fraction, indicador, multiplicadorFase);
		newIndic = priv_promediar_historia(historia, es->fraction);
		priv_printIndicator(es, nomIndicador, st_fase, newIndic);
		for (int64_t i = 0; i < es->numDistances; ++i) {
			es->fraction[i] = prev_ponder[i];
			es->weights[i] = es->normalization[i] * es->fraction[i];
		}
	}
	MY_FREE(prev_ponder);
	return newIndic;
}
static char priv_getMejor(double ind1, double ind2, double ind3,
		char *paramMinMax) {
	if (my_string_equals(paramMinMax, "MIN")) {
		if (ind1 == IND_INVALID)
		ind1 = DBL_MAX;
		if (ind2 == IND_INVALID)
		ind2 = DBL_MAX;
		if (ind3 == IND_INVALID)
		ind3 = DBL_MAX;
		if (ind1 <= ind2 && ind1 <= ind3) {
			return 1;
		} else if (ind2 <= ind1 && ind2 <= ind3) {
			return 2;
		} else if (ind3 <= ind1 && ind3 <= ind2) {
			return 3;
		}
	} else if (my_string_equals(paramMinMax, "MAX")) {
		if (ind1 == IND_INVALID)
		ind1 = -DBL_MAX;
		if (ind2 == IND_INVALID)
		ind2 = -DBL_MAX;
		if (ind3 == IND_INVALID)
		ind3 = -DBL_MAX;
		if (ind1 >= ind2 && ind1 >= ind3) {
			return 1;
		} else if (ind2 >= ind1 && ind2 >= ind3) {
			return 2;
		} else if (ind3 >= ind1 && ind3 >= ind2) {
			return 3;
		}
	}
	my_log_error("unknown criterium %s (MIN|MAX)\n", paramMinMax);
	return 0;
}
static double priv_mejorarUnPeso(Distance *fd, int64_t idPeso,
		double ind_inicio, char *paramMinMax, char *nomIndicador, char *st_fase,
		struct Historia *historia, int64_t numSamples,
		int64_t multiplicadorFase, double epsilonFase) {
	struct State_Multimetric *es = fd->state_dist;
	double ind_actual = ind_inicio, ind_arriba = 0, ind_abajo = 0;
	int64_t cont = 0;
	for (;;) {
		if (ind_actual == 0)
		ind_actual = priv_calcularIndicador(fd, idPeso, 0, nomIndicador,
				st_fase, historia, numSamples, multiplicadorFase,
				epsilonFase);
		if (ind_arriba == 0)
		ind_arriba = priv_calcularIndicador(fd, idPeso, 1, nomIndicador,
				st_fase, historia, numSamples, multiplicadorFase,
				epsilonFase);
		if (ind_abajo == 0)
		ind_abajo = priv_calcularIndicador(fd, idPeso, -1, nomIndicador,
				st_fase, historia, numSamples, multiplicadorFase,
				epsilonFase);
		char mejor = priv_getMejor(ind_actual, ind_arriba, ind_abajo,
				paramMinMax);
		if (mejor == 1) {
			return ind_actual;
		} else if (mejor == 2) {
			ind_abajo = ind_actual;
			ind_actual = ind_arriba;
			ind_arriba = 0;
			if (!priv_adjustFractions(es, idPeso, 1, epsilonFase))
			my_log_error("invalid weights\n");
		} else if (mejor == 3) {
			ind_arriba = ind_actual;
			ind_actual = ind_abajo;
			ind_abajo = 0;
			if (!priv_adjustFractions(es, idPeso, -1, epsilonFase))
			my_log_error("invalid weights\n");
		}
		//version 3, mejoro un peso y trato de cambiar el sgte
		//if (params->numDistancias > 2)
		//cont++;
		if (cont > 3)
		return ind_actual;
	}
	return ind_actual;
}
static double priv_ajuste_por_fase(Distance* fd, char *paramMinMax,
		char *nomIndicador, struct Historia *historia, char *st_fase,
		int64_t numSamples, int64_t multiplicadorFase, char *st_epsilon,
		MyTimer *timer) {
	struct State_Multimetric *es = fd->state_dist;
	double dPrev = 0, epsilon = my_parse_fraction(st_epsilon);
	int64_t numSinCambios = 0, idPeso = 0;
	while (numSinCambios < 2 * es->numDistances) {
		double newPrev = priv_mejorarUnPeso(fd, idPeso, dPrev, paramMinMax,
				nomIndicador, st_fase, historia, numSamples, multiplicadorFase,
				epsilon);
		if (dPrev == newPrev)
		numSinCambios++;
		else
		numSinCambios = 0;
		dPrev = newPrev;
		idPeso = (idPeso + 1) % es->numDistances;
	}
	my_log_info(
			"[%s] e=%s, smp=%"PRIi64" * %"PRIi64", %1.1lf seconds, final=> ",
			st_fase, st_epsilon, numSamples, multiplicadorFase,
			my_timer_getSeconds(timer));
	priv_printIndicator(es, nomIndicador, st_fase, dPrev);
	return dPrev;
}
static void priv_automatic_weighting(Distance* fd) {
	struct State_Multimetric *es = fd->state_dist;
	char *filename = my_newString_format("AutoWeight.txt");
	if (!my_io_existsFile2(es->path_profile, filename)) {
		my_log_info("AutoWeight %s...\n", es->autoWeight);
		int64_t samplesAutoWeight = my_env_getInt(
				"PVCD_MULTIMETRIC_SAMPLES_AUTOWEIGHT", 0);
		if (samplesAutoWeight == 0) {
			my_log_info("AutoWeight requires to set environment variable:\n");
			my_log_info(
					"PVCD_MULTIMETRIC_SAMPLES_AUTOWEIGHT  : numSamples for histograms of AutoWeight\n");
			return;
		}
		MyTimer *timer = my_timer_new();
		char *paramMinMax = my_subStringI_fromTo(es->autoWeight, 0, 3);
		char *nomIndicador = my_subStringI_fromEnd(es->autoWeight, 3);
		struct Historia *historia = newHistoria(es->numDistances);
		double indicador = 0;
		indicador = priv_ajuste_por_fase(fd, paramMinMax, nomIndicador,
				historia, "1/4", samplesAutoWeight, 1, "0.05", timer);
		indicador = priv_ajuste_por_fase(fd, paramMinMax, nomIndicador,
				historia, "2/4", samplesAutoWeight, 5, "0.02", timer);
		indicador = priv_ajuste_por_fase(fd, paramMinMax, nomIndicador,
				historia, "3/4", samplesAutoWeight, 10, "0.006", timer);
		indicador = priv_ajuste_por_fase(fd, paramMinMax, nomIndicador,
				historia, "4/4", samplesAutoWeight, 20, "0.002", timer);
		releaseHistoria(historia);
		my_timer_release(timer);
		FILE *out = my_io_openFileWrite2(es->path_profile, filename);
		fprintf(out, "#%s sobre dist %s\n", es->autoWeight, fd->name);
		fprintf(out, "#distancia %s\n", fd->nombreOriginal);
		fprintf(out, "#%s value para %s => %1.6lf\n", paramMinMax, nomIndicador,
				indicador);
		for (int64_t i = 0; i < es->numDistances; ++i) {
			char *st = my_newString_double(es->fraction[i]);
			fprintf(out, "%s\n", st);
			MY_FREE(st);
		}
		fclose(out);
		MY_FREE_MULTI(paramMinMax, nomIndicador);
	}
	MyLineReader *reader = my_lreader_open(
			my_io_openFileRead2(es->path_profile, filename, 1));
	for (int64_t i = 0; i < es->numDistances; ++i)
	es->fraction[i] = my_parse_double(my_lreader_readLine(reader));
	my_lreader_close(reader, true);
	priv_correctWeights(es);
	MY_FREE(filename);
}
static void print_help_multimetric() {
	my_log_info("MULTI,[aw_]func1+func2+...\n");
	my_log_info("  where: funcN   => distN*normalization*fraction\n");
	my_log_info(
			"         distN   => distance between the Nth descriptor of collections\n");
	my_log_info(
			"         normalization => decimal or num/denom or AutoNorm=A[alpha]\n");
	my_log_info("         alpha => 1|0.5|0.1|0.01|0.001|0.0001|0.00001\n");
	my_log_info("         fraction => decimal or num/denom\n");
	my_log_info(
			"         aw => AutoWeight=[MIN|MAX][indicator]       Optional.\n");
	my_log_info("         indicator => VAR|RHO|A[alpha]\n");
	my_log_info("MULTIOBJ,[aw_]funcObj+funcObj+...\n");
	my_log_info("  where: funcObj => distObj*normalization*fraction\n");
	my_log_info("         distObj => any distance between objects\n");
	my_log_info(" e.g. MULTI,L1*1/100*1/2+L2*1/2500*1/2\n");
	my_log_info(
			"      MULTI,L1*AutoNorm=A0.01*1/3+L1*AutoNorm=A0.01*1/3+L1*AutoNorm=A0.01*1/3 (alpha-normalization)\n");
	my_log_info(
			"      MULTI,AutoWeight=MAXA0.1_L1*AutoNorm=A0.1*0.5+L1*AutoNorm=A0.1*0.5  (weighting by max-tau)\n");
	my_log_info(
			"      MULTI,AutoWeight=MAXRHO_L1*AutoNorm=A0.1*0.5+L1*AutoNorm=A0.1*0.5   (weighting by max-rho)\n");
	my_log_info(
			"      MULTIOBJ,DESCNUM,0_L1*1/100*1/2+TEMPORAL,5_DESCNUM,3_L2*1/2500*1/2\n");
	pvcd_system_exit_error();
}

#endif
