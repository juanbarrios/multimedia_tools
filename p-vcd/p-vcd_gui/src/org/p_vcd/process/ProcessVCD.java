/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.net.URL;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;
import org.p_vcd.model.VideoCopy;
import org.p_vcd.model.VideoDatabase;

public class ProcessVCD extends ProcessBase {

	private File queryFile;
	private URL queryUrl;
	private VideoDatabase queryDb;
	private Collection<VideoDatabase> referenceDbs;
	private List<VideoCopy> detections;
	private ProcessOptions options;
	private StatusListener status;
	private int currentStep, totalSteps;

	public ProcessVCD(Collection<VideoDatabase> referenceDbs, File queryFile, URL queryUrl, VideoDatabase queryDb,
			ProcessOptions options) {
		this.referenceDbs = referenceDbs;
		this.queryFile = queryFile;
		this.queryUrl = queryUrl;
		this.queryDb = queryDb;
		this.options = options;
	}

	@Override
	protected void runProcess(StatusListener status) throws Exception {
		this.totalSteps = 3 + 4 * referenceDbs.size();
		this.status = status;
		this.totalSteps += referenceDbs.size();
		long st = System.currentTimeMillis();
		stepCompleted("Creating Query Database");
		if (queryDb == null) {
			if (queryUrl != null)
				downloadQueryDB();
			if (queryFile == null)
				throw new Exception("can't find a query video");
			createQueryDB();
		}
		computeDescriptorsDB("query", queryDb, options.descriptorQuery);
		for (VideoDatabase dbr : referenceDbs) {
			computeDescriptorsDB("reference", dbr, options.descriptorRef);
		}
		this.detections = new ArrayList<VideoCopy>();
		for (VideoDatabase dbr : referenceDbs) {
			stepCompleted("Similarity Search " + dbr.getName());
			File searchProfile = createSearchProfile(dbr);
			File filesNN = similaritySearch(searchProfile, dbr);
			stepCompleted("Copy Localization " + dbr.getName());
			File fileDetections = copyLocalization(filesNN);
			List<VideoCopy> dets = parseDetectionsFile(fileDetections, dbr);
			this.detections.addAll(dets);
		}
		Collections.sort(this.detections, new Comparator<VideoCopy>() {
			@Override
			public int compare(VideoCopy vc1, VideoCopy vc2) {
				return Double.compare(vc2.getScore(), vc1.getScore());
			}
		});
		long en = System.currentTimeMillis();
		stepCompleted("Finished OK. Total time: " + MyUtil.getSecondsToHHMMSS((en - st) / 1000.0));
	}

	private void downloadQueryDB() throws Exception {
		FileUtils.forceMkdir(Parameters.get().getDownloadsPath());
		File tmpDir = Files.createTempDirectory(Parameters.get().getDownloadsPath().toPath(), "download-").toFile();
		downloadImageOrVideo(queryUrl, tmpDir);
		for (File file : tmpDir.listFiles()) {
			if (MyUtil.fileHasExtension(file, Parameters.get().getImageAndVideoExtensions())) {
				queryFile = file;
				break;
			}
		}
	}

	protected void downloadImageOrVideo(URL imageOrVideoUrl, File workingDir) throws Exception {
		ProcessArguments commandLine = new ProcessArguments();
		commandLine.add("--server-response", "--spider", "--no-check-certificate", imageOrVideoUrl);
		StringBuffer stderr = new StringBuffer();
		runWget(commandLine, workingDir, stderr);
		boolean isImage = false, isVideo = false;
		for (String line : stderr.toString().split("\n")) {
			String l = line.toLowerCase();
			if (l.startsWith("  content-type: video/")) {
				isVideo = true;
				break;
			} else if (line.startsWith("  content-type: image/")) {
				isImage = true;
				break;
			}
		}
		commandLine.clear();
		if (isImage || isVideo) {
			commandLine.add("--no-directories", "--no-check-certificate", imageOrVideoUrl);
			runWget(commandLine, workingDir, null);
		} else {
			commandLine.add("--restrict-filenames", imageOrVideoUrl);
			runYoutubeDl(commandLine, workingDir);
		}
	}

	private void createQueryDB() throws Exception {
		FileUtils.forceMkdir(Parameters.get().getQueriesPath());
		String dbName = MyUtil.getMd5(queryFile) + queryFile.length();
		VideoDatabase qdb = VideoDatabase.getQueryDatabase(dbName);
		if (qdb == null) {
			File newDb = new File(Parameters.get().getQueriesPath(), dbName);
			runPvcdDb(new ProcessArguments("-new", "-db", newDb, queryFile));
		}
		qdb = VideoDatabase.getQueryDatabase(dbName);
		if (qdb == null)
			throw new Exception("error at creating query db");
		this.queryDb = qdb;
	}

	private void computeDescriptorsDB(String name, VideoDatabase videoDb, ProcessOptions.OneDescriptorOptions optD)
			throws Exception {
		int num = videoDb.getFileList().size();
		stepCompleted("Segmenting " + num + " " + name + " video" + (num == 1 ? "" : "s"));
		if (!new File(new File(videoDb.getDbDir(), "segmentations"), optD.segAlias).exists())
			runPvcdDb(new ProcessArguments("-segment", "-db", videoDb, "-seg", optD.seg, "-alias", optD.segAlias));
		stepCompleted("Computing descriptors for " + num + " " + name + " video" + (num == 1 ? "" : "s"));
		if (!new File(new File(videoDb.getDbDir(), "descriptors"), optD.desAlias).exists())
			runPvcdDb(new ProcessArguments("-extract", "-db", videoDb, "-seg", optD.segAlias, "-desc", optD.des,
					"-alias", optD.desAlias));
	}

	private File createSearchProfile(VideoDatabase dbr) throws Exception {
		FileUtils.forceMkdir(Parameters.get().getSearchesPath());
		String searchName = dbr.getName() + "-" + queryDb.getName();
		File searchProfile = new File(Parameters.get().getSearchesPath(), searchName);
		if (searchProfile.exists())
			FileUtils.forceDelete(searchProfile);
		// if (!this.searchProfile.exists())
		runPvcdSearch(new ProcessArguments("-new", "-profile", searchProfile, "-query", queryDb, "-descQ",
				options.descriptorQuery.desAlias, "-reference", dbr, "-descR", options.descriptorRef.desAlias,
				"-distance", options.searchOptions.distance));
		return searchProfile;
	}

	private File similaritySearch(File searchProfile, VideoDatabase dbr) throws Exception {
		String searchId = "search";
		File filesNN = new File(searchProfile, "ss," + searchId + ".txt");
		ProcessArguments params = new ProcessArguments("-ss", "-profile", searchProfile, "-searchName", searchId);
		if (options.searchOptions.isSearchByLocal)
			params.add("-searchByLocalVectors");
		params.add("-index", options.searchOptions.indexBuildOptions);
		if (options.searchOptions.indexFilename != null) {
			File desDir = new File(new File(dbr.getDbDir(), "descriptors"), options.descriptorRef.desAlias);
			File indexFile = new File(desDir, options.searchOptions.indexFilename);
			params.add("-load_index_path", indexFile, "-save_index_path", indexFile);
		}
		params.add("-knn", options.searchOptions.knnSearch);
		if (options.searchOptions.indexSearchOptions != null)
			params.add("-searchOptions", options.searchOptions.indexSearchOptions);
		runPvcdSearch(params);
		if (options.searchOptions.isSearchByLocal) {
			File filesNNLocal = new File(searchProfile, "ssVector," + searchId + ".txt");
			runPvcdMerge(new ProcessArguments("-ss", filesNNLocal, "-out", filesNN, "-maxVectorsIn",
					options.searchOptions.knnSearch, "-maxNNOut", options.searchOptions.knnSearchAfterMerge));
		}
		return filesNN;
	}

	private File copyLocalization(File filesNN) throws Exception {
		File fileDetections = new File(filesNN.getParent(), "detections_" + filesNN.getName());
		runPvcdDetect(new ProcessArguments("-detect", "-ss", filesNN, "-out", fileDetections,
				options.localizationOptions.getOptions()));
		return fileDetections;
	}

	private List<VideoCopy> parseDetectionsFile(File fileDetections, VideoDatabase dbr) throws Exception {
		List<VideoCopy> detections = new ArrayList<VideoCopy>();
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new FileReader(fileDetections));
			String line;
			while ((line = reader.readLine()) != null) {
				if (line.startsWith("#"))
					continue;
				String[] parts = line.split("\t");
				VideoCopy vc = new VideoCopy();
				vc.setScore(Double.parseDouble(parts[0]));
				vc.setVideoQ(queryDb.getFileMap().get(parts[1]));
				vc.setFromQ(MyUtil.parseHHMMSSToSeconds(parts[2]));
				vc.setToQ(MyUtil.parseHHMMSSToSeconds(parts[3]));
				vc.setVideoR(dbr.getFileMap().get(parts[4]));
				vc.setFromR(MyUtil.parseHHMMSSToSeconds(parts[5]));
				vc.setToR(MyUtil.parseHHMMSSToSeconds(parts[6]));
				if (vc.getVideoQ() == null)
					throw new Exception("can't find " + parts[1]);
				if (vc.getVideoR() == null)
					throw new Exception("can't find " + parts[4]);
				detections.add(vc);
			}
		} finally {
			IOUtils.closeQuietly(reader);
		}
		return detections;
	}

	private void stepCompleted(String nextStepName) {
		status.setPctProgress(nextStepName, currentStep / ((double) totalSteps));
		currentStep++;
	}

	public List<VideoCopy> getDetections() {
		return detections;
	}
}