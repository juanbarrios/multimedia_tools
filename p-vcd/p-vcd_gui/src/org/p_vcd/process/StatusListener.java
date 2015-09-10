/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.process;

public interface StatusListener {

	public void appendOutputLine(String line);

	public void setPctProgress(String stepName, double pct);

	public void callbackOnEnd(ProcessBase process, boolean wasSuccessful);

}
