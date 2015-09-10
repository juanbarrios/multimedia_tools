/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.SystemUtils;

public class ConfigFile {
	private Map<String, String> map;
	private File currentPath;

	public ConfigFile(File iniFile) throws Exception {
		File absolute = iniFile.getAbsoluteFile();
		// The configuration file is a text file with default encoding.
		// IT IS NOT a JAVA PROPERTIES FILE.
		// (properties require ISO encoding and '\' must be escaped, while
		// xml file is too complicated for common users.)
		this.map = new HashMap<String, String>();
		List<String> lines = FileUtils.readLines(absolute);
		for (String line : lines) {
			line = line.trim();
			if (line.isEmpty() || line.startsWith("#"))
				continue;
			int n = line.indexOf("=");
			if (n <= 0)
				continue;
			String name = line.substring(0, n).trim();
			String value = line.substring(n + 1).trim();
			if (!name.isEmpty())
				map.put(name, value);
		}
		this.currentPath = absolute.getParentFile();
	}

	public String getString(String name) {
		return map.get(name);
	}

	public int getInt(String name) {
		String value = getString(name);
		try {
			if (value != null && !value.isEmpty())
				return Integer.parseInt(value, 10);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return 0;
	}

	public File getFile(String name) {
		String value = getString(name);
		if (value == null || value.isEmpty())
			throw new RuntimeException("'" + name + "' is empty");
		if (SystemUtils.IS_OS_WINDOWS) {
			value = value.replace('/', '\\');
		} else {
			value = value.replace('\\', '/');
		}
		File file = new File(value);
		if (file.isAbsolute())
			return file;
		else
			return new File(this.currentPath, value).getAbsoluteFile();
	}

}
