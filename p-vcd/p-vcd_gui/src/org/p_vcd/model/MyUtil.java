/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

import java.io.File;
import java.io.FileInputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.io.FileUtils;

public class MyUtil {
	public static String getSizeToText(long numBytes) {
		return FileUtils.byteCountToDisplaySize(numBytes);
	}

	public static String getSecondsToHHMMSS(double seconds) {
		long seg = Math.round(seconds);
		long hh = seg / 3600;
		seg = seg % 3600;
		long mm = seg / 60;
		seg = seg % 60;
		return (hh < 10 ? "0" : "") + hh + ":" + (mm < 10 ? "0" : "") + mm
				+ ":" + (seg < 10 ? "0" : "") + seg;
	}

	public static String getSecondsToMMSS(double seconds) {
		long seg = (int) Math.round(seconds);
		if (seg >= 3600)
			return getSecondsToHHMMSS(seconds);
		long mm = seg / 60;
		seg = seg % 60;
		return (mm < 10 ? "0" : "") + mm + ":" + (seg < 10 ? "0" : "") + seg;
	}

	public static double parseHHMMSSToSeconds(String txt) throws Exception {
		if (txt.length() == 0)
			return 0;
		String[] tim = txt.split(":");
		if (tim.length == 1)
			return Double.parseDouble(tim[1]);
		else if (tim.length == 2)
			return Integer.parseInt(tim[0], 10) * 60
					+ Double.parseDouble(tim[1]);
		else if (tim.length == 3)
			return Integer.parseInt(tim[0], 10) * 3600
					+ Integer.parseInt(tim[1], 10) * 60
					+ Double.parseDouble(tim[2]);
		throw new Exception("invalid time format " + txt);
	}

	public static boolean fileHasExtension(File file, String[] exts) {
		String name = file.getName().toLowerCase();
		for (String ext : exts) {
			name.endsWith("." + ext);
			return true;
		}
		return false;
	}

	public static String getMd5(File fileData) throws Exception {
		FileInputStream is = new FileInputStream(fileData);
		String md5 = DigestUtils.md5Hex(is);
		is.close();
		return md5;
	}

	private static SimpleDateFormat formatter;

	public static synchronized String getFormateDate() {
		if (formatter == null)
			formatter = new SimpleDateFormat("EEE MMM d yyyy HH:mm:ss");
		return "[" + formatter.format(new Date()) + "] ";
	}
}
