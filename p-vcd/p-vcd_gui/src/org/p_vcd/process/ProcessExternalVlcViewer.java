/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

import java.io.File;

import org.p_vcd.model.VideoCopy;

public class ProcessExternalVlcViewer extends ProcessBase {

	private static ProcessExternalVlcViewer externalVlc;

	public static synchronized void playVideoCopy(VideoCopy vc) {
		if (externalVlc != null)
			externalVlc.sendKillProcess();
		externalVlc = new ProcessExternalVlcViewer(vc);
		externalVlc.asyncRun(null);
	}

	public static synchronized void closeExternalWindows() {
		if (externalVlc != null)
			externalVlc.sendKillProcess();
		externalVlc = null;
	}

	private ProcessSingleViewer viewerQ;
	private ProcessSingleViewer viewerR;

	public ProcessExternalVlcViewer(VideoCopy vc) {
		int w1 = vc.getVideoQ().getWidth();
		int w2 = vc.getVideoR().getWidth();
		int h1 = vc.getVideoQ().getHeight();
		int h2 = vc.getVideoR().getHeight();
		this.viewerQ = new ProcessSingleViewer(vc.getVideoQ().getFilePath(), vc.getFromQ(), vc.getToQ(), 20, 20, w1 / 2,
				h1 / 2);
		this.viewerR = new ProcessSingleViewer(vc.getVideoR().getFilePath(), vc.getFromR(), vc.getToR(), w1 / 2, h1 / 2,
				w2 / 2, h2 / 2);
	}

	@Override
	protected void runProcess(StatusListener status) throws Exception {
		this.viewerQ.asyncRun(null);
		this.viewerR.asyncRun(null);
		this.viewerQ.waitProcess();
		this.viewerR.waitProcess();
	}

	@Override
	public void sendKillProcess() {
		this.viewerQ.sendKillProcess();
		this.viewerR.sendKillProcess();
		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		super.sendKillProcess();
	}
}

class ProcessSingleViewer extends ProcessBase {
	private String filePath;
	private double secondsStart;
	private double secondsEnd;
	private int windowX, windowY, windowWidth, windowHeight;

	public ProcessSingleViewer(String filePath, double secondsStart, double secondsEnd, int windowX, int windowY,
			int windowWidth, int windowHeight) {
		this.filePath = filePath;
		this.secondsStart = secondsStart;
		this.secondsEnd = secondsEnd;
		this.windowX = windowX;
		this.windowY = windowY;
		this.windowWidth = windowWidth;
		this.windowHeight = windowHeight;
	}

	@Override
	protected void runProcess(StatusListener status) throws Exception {
		File videoFile = new File(filePath);
		double from = Math.round(secondsStart * 10) / 10.0;
		double length = Math.round((secondsEnd - secondsStart) * 10) / 10.0;
		ProcessArguments commandLine = new ProcessArguments();
		commandLine.add(videoFile.getAbsolutePath());
		commandLine.add("--start-time=" + from);
		commandLine.add("--run-time=" + length);
		commandLine.add("--play-and-pause");
		commandLine.add("--quiet");
		commandLine.add("--video-x=" + windowX);
		commandLine.add("--video-y=" + windowY);
		commandLine.add("--width=" + windowWidth);
		commandLine.add("--height=" + windowHeight);
		runVlc(commandLine);
	}

}