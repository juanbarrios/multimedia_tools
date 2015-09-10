/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.p_vcd.model.VideoDatabase;

public class ProcessArguments {
	private List<String> params;

	public ProcessArguments(Object... arguments) {
		this.params = new ArrayList<String>();
		add(arguments);
	}

	public void add(Object... arguments) {
		List<String> list = new ArrayList<String>();
		for (Object o : arguments)
			converArgsToString(list, o);
		if (list.size() == 0)
			return;
		this.params.addAll(list);
	}

	public void insertFirst(Object... arguments) {
		List<String> list = new ArrayList<String>();
		for (Object o : arguments)
			converArgsToString(list, o);
		if (list.size() == 0)
			return;
		for (int i = list.size() - 1; i >= 0; --i) {
			this.params.add(0, list.get(i));
		}
	}

	public List<String> getCommands() {
		return params;
	}

	private static void converArgsToString(List<String> commandLine, Object arg) {
		if (arg == null) {
			return;
		} else if (arg instanceof Collection) {
			for (Object s : (Collection<?>) arg)
				converArgsToString(commandLine, s);
		} else if (arg instanceof Object[]) {
			for (Object s : (Object[]) arg)
				converArgsToString(commandLine, s);
		} else if (arg instanceof String[]) {
			for (String s : (String[]) arg)
				converArgsToString(commandLine, s);
		} else if (arg instanceof File) {
			commandLine.add(((File) arg).getAbsolutePath());
		} else if (arg instanceof URL) {
			commandLine.add(((URL) arg).toString());
		} else if (arg instanceof VideoDatabase) {
			commandLine.add(((VideoDatabase) arg).getDbDir().getAbsolutePath());
		} else
			commandLine.add(arg.toString());
	}

	public void addToLog(StringBuffer sbLog) {
		boolean first = true;
		for (String p : params) {
			if (first)
				first = false;
			else
				sbLog.append(" ");
			if (p.indexOf(' ') > 0)
				sbLog.append("\"").append(p).append("\"");
			else
				sbLog.append(p);
		}

	}

	public void clear() {
		this.params.clear();
	}
}
