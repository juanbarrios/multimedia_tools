/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

import java.io.File;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.io.FileUtils;

public class PVCDObj {

	public static Map<String, PVCDObj> parseDataset(File datasetDir) throws Exception {
		Map<String, PVCDObj> objects = new TreeMap<String, PVCDObj>();
		List<String> lines = FileUtils.readLines(new File(datasetDir, "files.txt"), "UTF8");
		for (String line : lines) {
			if (line.isEmpty() || line.startsWith("#"))
				continue;
			String[] parts = line.split("\t");
			PVCDObj p = new PVCDObj("I".equals(parts[0]), "V".equals(parts[0]), parts[1], parts[2].replace('\\', '/'),
					Long.parseLong(parts[3]), Integer.parseInt(parts[4]), Integer.parseInt(parts[5]));
			if (p.isVideo()) {
				p.setSecondsLength(Double.parseDouble(parts[7]) - Double.parseDouble(parts[6]));
				p.setFps(Double.parseDouble(parts[8]));
			}
			if (objects.put(p.getObjId(), p) != null)
				throw new Exception("duplicated object " + p.getObjId());
		}
		return objects;
	}

	private boolean isImage, isVideo;
	private String objId, filePath, filename;
	private double fps, secondsLength;
	private int width, height;
	private long filesize;

	public PVCDObj(boolean isImage, boolean isVideo, String objId, String filePath, long filesize, int width,
			int height) {
		this.isImage = isImage;
		this.isVideo = isVideo;
		this.objId = objId;
		this.filePath = filePath;
		this.filename = filePath.substring(filePath.lastIndexOf('/') + 1);
		this.filesize = filesize;
		this.width = width;
		this.height = height;
	}

	public String getTimeText() {
		return MyUtil.getSecondsToHHMMSS(secondsLength);
	}

	public String getSizeText() {
		return MyUtil.getSizeToText(filesize);
	}

	public String getObjId() {
		return objId;
	}

	public String getFilePath() {
		return filePath;
	}

	public double getFps() {
		return fps;
	}

	public void setFps(double fps) {
		this.fps = fps;
	}

	public double getSecondsLength() {
		return secondsLength;
	}

	public void setSecondsLength(double secondsLength) {
		this.secondsLength = secondsLength;
	}

	public boolean isImage() {
		return isImage;
	}

	public boolean isVideo() {
		return isVideo;
	}

	public String getFilename() {
		return filename;
	}

	public long getFilesize() {
		return filesize;
	}

	public int getWidth() {
		return width;
	}

	public int getHeight() {
		return height;
	}

}
