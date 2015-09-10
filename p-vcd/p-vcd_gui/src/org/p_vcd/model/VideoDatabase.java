/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class VideoDatabase implements Comparable<VideoDatabase> {
	private static List<VideoDatabase> loadedDatabases = new ArrayList<VideoDatabase>();

	public static synchronized List<VideoDatabase> getReferenceDatabases() {
		List<VideoDatabase> arrayList = new ArrayList<VideoDatabase>();
		File baseDir = Parameters.get().DATABASES_DIR;
		if (!baseDir.exists())
			return arrayList;
		try {
			for (File d : baseDir.listFiles()) {
				if (!isDirDb(d))
					continue;
				String uid = d.getAbsolutePath();
				VideoDatabase db = null;
				for (VideoDatabase v : loadedDatabases) {
					if (v.uid.equals(uid)) {
						db = v;
						break;
					}
				}
				if (db == null) {
					db = new VideoDatabase(d);
					loadedDatabases.add(db);
				}
				arrayList.add(db);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return arrayList;
	}

	public static synchronized VideoDatabase getQueryDatabase(String dbName)
			throws Exception {
		File d = new File(Parameters.get().QUERIES_DIR, dbName);
		if (isDirDb(d))
			return new VideoDatabase(d);
		return null;
	}

	private static boolean isDirDb(File d) {
		return d.isDirectory() && new File(d, "files.txt").exists();
	}

	private String uid;
	private String name;
	private long totalBytesVideos;
	private double totalSeconds;
	private Map<String, PVCDObj> fileMap;
	private List<PVCDObj> fileList;
	private File dbDir;

	private VideoDatabase(File dbDir) throws Exception {
		this.dbDir = dbDir;
		this.uid = dbDir.getAbsolutePath().toLowerCase();
		this.name = dbDir.getName();
		this.fileMap = PVCDObj.parseDataset(dbDir);
		this.fileList = new ArrayList<PVCDObj>(this.fileMap.values());
		this.totalBytesVideos = 0;
		this.totalSeconds = 0;
		for (PVCDObj p : fileList) {
			this.totalBytesVideos += p.getFilesize();
			this.totalSeconds += p.getSecondsLength();
		}
		// metadataSize=FileUtils.sizeOfDirectory(dbDir);
	}

	@Override
	public int hashCode() {
		return uid.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		return (obj != null) && (obj instanceof VideoDatabase)
				&& ((VideoDatabase) obj).uid.equals(uid);
	}

	@Override
	public String toString() {
		return uid.toString();
	}

	@Override
	public int compareTo(VideoDatabase db) {
		return uid.compareTo(db.uid);
	}

	public File getDbDir() {
		return dbDir;
	}

	public String getUid() {
		return uid;
	}

	public String getName() {
		return name;
	}

	public Map<String, PVCDObj> getFileMap() {
		return fileMap;
	}

	public List<PVCDObj> getFileList() {
		return fileList;
	}

	public long getTotalBytesVideos() {
		return totalBytesVideos;
	}

	public double getTotalSeconds() {
		return totalSeconds;
	}

}
