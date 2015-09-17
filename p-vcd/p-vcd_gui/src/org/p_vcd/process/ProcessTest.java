/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

import org.p_vcd.model.Parameters;

public abstract class ProcessTest extends ProcessBase {

	public static String validateAll() throws Exception {
		ProcessTest test1 = new ProcessTestPvcd();
		if (!test1.validate())
			return test1.getErrorMessage();
		ProcessTest test2 = new ProcessTestWget();
		if (!test2.validate())
			return test2.getErrorMessage();
		ProcessTest test3 = new ProcessTestYoutubeDl();
		if (!test3.validate())
			return test3.getErrorMessage();
		return null;
	}

	private String errorMessage;

	private boolean validate() {
		this.errorMessage = null;
		this.asyncRun(null);
		this.waitProcess();
		return (this.errorMessage == null);
	}

	@Override
	protected void runProcess(StatusListener status) throws Exception {
		try {
			runTestProcess();
		} catch (Exception e) {
			e.printStackTrace();
			errorMessage = "error running " + getTestName() + ". " + e.getMessage() + " (see log for details)";
		}
	}

	private String getErrorMessage() {
		return errorMessage;
	}

	protected abstract void runTestProcess() throws Exception;

	protected abstract String getTestName();

}

class ProcessTestPvcd extends ProcessTest {
	@Override
	protected void runTestProcess() throws Exception {
		runPvcdDb(new ProcessArguments("-version"));
		runPvcdSearch(new ProcessArguments("-version"));
		runPvcdMerge(new ProcessArguments("-version"));
		runPvcdDetect(new ProcessArguments("-version"));
	}

	@Override
	protected String getTestName() {
		return "P-VCD";
	}
}

class ProcessTestWget extends ProcessTest {
	@Override
	protected void runTestProcess() throws Exception {
		runWget(new ProcessArguments("--version"), Parameters.get().getUserDataPath(), null);
	}

	@Override
	protected String getTestName() {
		return "WGet";
	}
}

class ProcessTestYoutubeDl extends ProcessTest {
	@Override
	protected void runTestProcess() throws Exception {
		runYoutubeDl(new ProcessArguments("--version"), Parameters.get().getUserDataPath());
	}

	@Override
	protected String getTestName() {
		return "Youtube-Dl";
	}
}
