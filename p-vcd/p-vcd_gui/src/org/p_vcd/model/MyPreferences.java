/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

import java.io.File;
import java.util.prefs.Preferences;

public class MyPreferences {
	public static synchronized String getParameter(String keyname, String default_value) {
		Preferences pref = getUserPreferences();
		String value = pref.get(keyname, null);
		return (value == null || value.trim().length() == 0) ? default_value : value.trim();
	}

	public static synchronized void setParameter(String keyname, String value) {
		Preferences pref = getUserPreferences();
		if (value == null || value.trim().length() == 0)
			pref.remove(keyname);
		else
			pref.put(keyname, value.trim());
	}

	public static synchronized File getParameterFile(String keyname) {
		String value = getParameter(keyname, null);
		return (value == null) ? null : new File(value);
	}

	public static synchronized void setParameterFile(String keyname, File dir) {
		setParameter(keyname, (dir == null) ? null : dir.getAbsolutePath());
	}

	private static Preferences getUserPreferences() {
		// under HKEY_LOCAL_MACHINE/software/JavaSoft/Prefs
		// under HKEY_CURRENT_USER/software/JavaSoft/Prefs
		return Preferences.userRoot().node("PVCD.gui");
	}

}
