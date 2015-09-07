#include "evaluation.h"

void evaluarFramesSS(const char *fileFrames, struct SearchProfile *profile,
		struct GroundTruthFrames *gtf, char *stCommandLine) {
	struct ArchivoFrames *af = loadFileSS(fileFrames, 0, 0, profile);
	MknnEvaluation_s *mev = GroundTruthFrames_evaluateFile(gtf, af);
	char *stMap = my_newString_doubleDec(mknn_evaluation_getMAP_s(mev), 4);
	my_log_info("%s\tMAP=%s\n", fileFrames, stMap);
	free(stMap);
	char *filename = my_newString_format("%s.eval", fileFrames);
	mknn_evaluation_print_s(mev, filename, stCommandLine);
	free(filename);
	mknn_evaluation_releaseEvaluation_s(mev);
}
void evaluate_similaritySearch(CmdParams *cmd_params, const char *argOption) {
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s %s\n", getBinaryName(cmd_params), argOption);
		my_log_info("   -ground_truth_frames file.txt\n");
		my_log_info("   -ground_truth_segments file.txt\n");
		my_log_info(
				"   -ground_truth_categories fileFrames.txt fileCategories.txt\n");
		my_log_info("   [frames-resolverSS]+...\n");
		return;
	}
	const char *ground_truth_frames = NULL;
	const char *ground_truth_segments = NULL;
	const char *ground_truth_categories_1 = NULL;
	const char *ground_truth_categories_2 = NULL;
	MyVectorString *files = my_vectorString_new();
	while (hasNextParam(cmd_params)) {
		if (isNextParam(cmd_params, "-ground_truth_frames")) {
			ground_truth_frames = nextParam(cmd_params);
			my_assert_fileExists(ground_truth_frames);
		} else if (isNextParam(cmd_params, "-ground_truth_segments")) {
			ground_truth_segments = nextParam(cmd_params);
			my_assert_fileExists(ground_truth_segments);
		} else if (isNextParam(cmd_params, "-ground_truth_categories")) {
			ground_truth_categories_1 = nextParam(cmd_params);
			ground_truth_categories_2 = nextParam(cmd_params);
			my_assert_fileExists(ground_truth_categories_1);
			my_assert_fileExists(ground_truth_categories_2);
		} else {
			const char *file_ss = nextParam(cmd_params);
			my_assert_fileExists(file_ss);
			my_vectorStringConst_add(files, file_ss);
		}
	}
	if (ground_truth_frames == NULL && ground_truth_segments == NULL
			&& ground_truth_categories_1 == NULL
			&& ground_truth_categories_2 == NULL)
		my_log_error("ground_truth is null\n");
	char *stCommandLine = get_command_line(cmd_params);
	for (int64_t i = 0; i < my_vectorString_size(files); ++i) {
		const char *file_ss = my_vectorString_get(files, i);
		struct SearchProfile *profile = loadProfileFromFile(file_ss);
		struct GroundTruthFrames *gtf = NULL;
		if (ground_truth_frames != NULL)
			gtf = newGroundTruthFileFrames(ground_truth_frames,
					profile->colQuery, profile->colReference);
		else if (ground_truth_segments != NULL)
			gtf = newGroundTruthFileSegments(ground_truth_segments,
					profile->colQuery, profile->colReference);
		else if (ground_truth_categories_1 != NULL
				&& ground_truth_categories_2 != NULL)
			gtf = newGroundTruthFilesFramesCategories(ground_truth_categories_1,
					ground_truth_categories_2, profile->colQuery,
					profile->colReference);
		my_assert_notNull("ground_truth", gtf);
		evaluarFramesSS(file_ss, profile, gtf, stCommandLine);
		releaseGroundTruthFrames(gtf);
		releaseProfile(profile);
	}
	free(stCommandLine);
}

int pvcd_evaluate(int argc, char **argv) {
	CmdParams *cmd_params = pvcd_system_start(argc, argv);
	if (!hasNextParam(cmd_params)) {
		my_log_info("%s -ss ...\n", getBinaryName(cmd_params));
		return pvcd_system_exit_error();
	}
	const char *option = nextParam(cmd_params);
	if (my_string_equals(option, "-ss")) {
		evaluate_similaritySearch(cmd_params, option);
	} else {
		my_log_error("unknown parameter %s\n", option);
	}
	return pvcd_system_exit_ok(cmd_params);
}
