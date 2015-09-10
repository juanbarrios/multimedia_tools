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
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ProcessBuilder.Redirect;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.SystemUtils;
import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;

public abstract class ProcessBase {

	private StatusListener status;
	private Thread processingThread;
	private boolean processRunning;
	private Process currentSystemProcess;

	public void asyncRun(StatusListener status) {
		if (status != null) {
			this.status = status;
		} else {
			this.status = new StatusListener() {
				@Override
				public void appendOutputLine(String line) {
				}

				@Override
				public void setPctProgress(String stepName, double pct) {
				}

				@Override
				public void callbackOnEnd(ProcessBase process,
						boolean wasSuccessful) {
				}
			};
		}
		this.processingThread = new Thread() {
			@Override
			public void run() {
				try {
					ProcessBase.this.runProcess(ProcessBase.this.status);
					ProcessBase.this.status.callbackOnEnd(ProcessBase.this,
							true);
				} catch (Throwable tr) {
					tr.printStackTrace();
					ProcessBase.this.status.appendOutputLine("\n\n"
							+ tr.toString());
					ProcessBase.this.status.setPctProgress(
							"ERROR: " + tr.toString(), 1);
					ProcessBase.this.status.callbackOnEnd(ProcessBase.this,
							false);
				}
			}
		};
		this.processingThread.start();
	}

	public void sendKillProcess() {
		if (!this.processingThread.isAlive())
			return;
		Thread kthread = new Thread() {
			@Override
			public void run() {
				while (ProcessBase.this.processRunning) {
					System.out.println("\ntrying to kill process...\n");
					ProcessBase.this.status
							.appendOutputLine("\ntrying to kill process...\n");
					System.out.println(ProcessBase.this.currentSystemProcess
							.getClass());
					ProcessBase.this.currentSystemProcess.destroy();
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				while (ProcessBase.this.processingThread.isAlive()) {
					System.out.println("\ninterrupting process...\n");
					ProcessBase.this.status
							.appendOutputLine("\ninterrupting process...\n");
					ProcessBase.this.processingThread.interrupt();
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		};
		kthread.setDaemon(true);
		kthread.start();
	}

	public void waitProcess() throws Exception {
		this.processingThread.join();
	}

	public synchronized boolean hasEnded() {
		return !this.processingThread.isAlive();
	}

	protected abstract void runProcess(StatusListener status) throws Exception;

	protected void runPvcdDb(ProcessArguments arguments) throws Exception {
		runPvcd("pvcd_db", arguments);
	}

	protected void runPvcdSearch(ProcessArguments arguments) throws Exception {
		runPvcd("pvcd_search", arguments);
	}

	protected void runPvcdMerge(ProcessArguments arguments) throws Exception {
		runPvcd("pvcd_mergeLocalToGlobal", arguments);
	}

	protected void runPvcdDetect(ProcessArguments arguments) throws Exception {
		runPvcd("pvcd_detect", arguments);
	}

	private void runPvcd(String command, ProcessArguments arguments)
			throws Exception {
		if (Parameters.get().MAX_CORES > 1)
			arguments.add("-num_cores", Parameters.get().MAX_CORES);
		if (SystemUtils.IS_OS_WINDOWS)
			command += ".exe";
		File binfile = new File(Parameters.get().PVCD_DIR, command);
		runCommandInternal(binfile, arguments, Parameters.get().USER_DATA_DIR,
				null, null);
	}

	protected void runCommandInternal(File command,
			ProcessArguments commandArgs, File workingDir,
			StringBuffer sbSaveStdout, StringBuffer sbSaveStderr)
			throws Exception {
		commandArgs.insertFirst(command);
		FileUtils.forceMkdir(workingDir);
		StringBuffer sbLog = new StringBuffer();
		sbLog.append("\n").append(MyUtil.getFormateDate());
		commandArgs.addToLog(sbLog);
		System.out.println(sbLog.toString());
		this.status.appendOutputLine(sbLog.toString());
		ProcessBuilder pb = new ProcessBuilder(commandArgs.getCommands());
		pb.directory(workingDir);
		pb.redirectInput(Redirect.INHERIT);
		pb.redirectOutput(Redirect.PIPE);
		pb.redirectError(Redirect.PIPE);
		long init = System.currentTimeMillis();
		this.currentSystemProcess = pb.start();
		PrintThreadWithStatus thStdout = new PrintThreadWithStatus(
				this.currentSystemProcess.getInputStream(), command.getName(),
				this.status, sbSaveStdout);
		PrintThreadWithStatus thStderr = new PrintThreadWithStatus(
				this.currentSystemProcess.getErrorStream(), command.getName(),
				this.status, sbSaveStderr);
		this.currentSystemProcess.getOutputStream().close();
		thStdout.start();
		thStderr.start();
		int ret = -1;
		try {
			this.processRunning = true;
			ret = this.currentSystemProcess.waitFor();
		} catch (InterruptedException ex) {
			ex.printStackTrace();
		} finally {
			this.processRunning = false;
		}
		try {
			thStderr.join();
		} catch (InterruptedException ex) {
			ex.printStackTrace();
		}
		try {
			thStdout.join();
		} catch (InterruptedException ex) {
			ex.printStackTrace();
		}
		long milis = System.currentTimeMillis() - init;
		if (ret != 0) {
			throw new Exception("command error code=" + ret + " (" + milis
					+ " ms)");
		}
		sbLog = new StringBuffer();
		sbLog.append(MyUtil.getFormateDate()).append("command ")
				.append(command.getName()).append(" ok (").append(milis)
				.append(" ms)");
		System.out.println(sbLog.toString());
		this.status.appendOutputLine(sbLog.toString());
	}
}

class PrintThreadWithStatus extends Thread {

	private InputStream is;
	private String commandName;
	private StatusListener status;
	private StringBuffer sbOutput;

	public PrintThreadWithStatus(InputStream is, String commandName,
			StatusListener status, StringBuffer sbOutput) {
		this.is = is;
		String s = commandName;
		if (s.endsWith(".exe") || s.endsWith(".EXE"))
			s = s.substring(0, s.length() - 4);
		this.commandName = "[" + s + "] ";
		this.status = status;
		this.sbOutput = sbOutput;
	}

	@Override
	public void run() {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new InputStreamReader(is, "UTF-8"));
			String line;
			while ((line = br.readLine()) != null) {
				System.out.println(commandName + line);
				this.status.appendOutputLine(line);
				if (this.sbOutput != null)
					this.sbOutput.append(line).append("\n");
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			IOUtils.closeQuietly(br);
			IOUtils.closeQuietly(is);
		}
	}

	public String getSavedOutput() {
		return sbOutput.toString();
	}
}
