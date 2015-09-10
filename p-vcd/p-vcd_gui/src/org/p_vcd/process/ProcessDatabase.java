/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

import java.io.File;
import java.util.Collection;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.io.FileUtils;

public class ProcessDatabase extends ProcessBase {

	private File dataDir;
	private String dbName;
	private Collection<String> files;

	public ProcessDatabase(File dataDir, String dbName, Collection<String> files) {
		this.dataDir = dataDir;
		this.dbName = dbName;
		this.files = files;
	}

	public String getDbName() {
		return dbName;
	}

	@Override
	protected void runProcess(StatusListener status) throws Exception {
		status.setPctProgress("Creating database...", 0);
		Set<String> set = new TreeSet<String>();
		set.addAll(files);
		FileUtils.forceMkdir(dataDir);
		File dbRef = new File(dataDir, dbName);
		if (!dbRef.exists())
			runPvcdDb(new ProcessArguments("-new", "-db", dbRef,
					"-recursiveDirs", "-fileType", "video", set));
		status.setPctProgress("Database " + dbName + " created OK", 1);
	}
}
