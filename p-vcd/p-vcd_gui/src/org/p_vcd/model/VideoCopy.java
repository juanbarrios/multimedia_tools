/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.model;

public class VideoCopy {
	private double score, fromQ, toQ, fromR, toR;
	private PVCDObj videoQ, videoR;

	public String getFromQtxt() {
		return MyUtil.getSecondsToMMSS(fromQ);
	}

	public String getToQtxt() {
		return MyUtil.getSecondsToMMSS(toQ);
	}

	public String getFromRtxt() {
		return MyUtil.getSecondsToMMSS(fromR);
	}

	public String getToRtxt() {
		return MyUtil.getSecondsToMMSS(toR);
	}

	public String getLengthTxt() {
		double l1 = toQ - fromQ;
		double l2 = toR - fromR;
		return MyUtil.getSecondsToMMSS((l1 > l2) ? l1 : l2);
	}

	public double getScore() {
		return score;
	}

	public void setScore(double score) {
		this.score = score;
	}

	public double getFromQ() {
		return fromQ;
	}

	public void setFromQ(double fromQ) {
		this.fromQ = fromQ;
	}

	public double getToQ() {
		return toQ;
	}

	public void setToQ(double toQ) {
		this.toQ = toQ;
	}

	public double getFromR() {
		return fromR;
	}

	public void setFromR(double fromR) {
		this.fromR = fromR;
	}

	public double getToR() {
		return toR;
	}

	public void setToR(double toR) {
		this.toR = toR;
	}

	public PVCDObj getVideoR() {
		return videoR;
	}

	public void setVideoR(PVCDObj videoR) {
		this.videoR = videoR;
	}

	public PVCDObj getVideoQ() {
		return videoQ;
	}

	public void setVideoQ(PVCDObj videoQ) {
		this.videoQ = videoQ;
	}

}
