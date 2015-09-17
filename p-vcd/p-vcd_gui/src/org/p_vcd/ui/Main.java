/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.EventQueue;
import java.io.File;

import javax.swing.UIManager;

import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;

public class Main {
	public static void main(String[] args) throws Exception {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			if (args.length < 2)
				throw new IllegalArgumentException("The command line is: java " + Main.class.getCanonicalName()
						+ " [bitness] [base_path]  , where [bitness] is the JVM bitness (32 or 64) and [base_path] is the installation root path.");
			int bitness = Integer.parseInt(args[0], 10);
			Parameters.initParameters(new File(args[1]), bitness);
			System.out.println(MyUtil.getFormateDate() + "P-VCD started.");
		} catch (Throwable tr) {
			SwingUtil.showError("Error at starting P-VCD", tr);
			System.exit(1);
			return;
		}
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MainFrame frame = new MainFrame();
					// frame.setLocationByPlatform(true);
					frame.setLocationRelativeTo(null);
					frame.setVisible(true);
				} catch (Throwable tr) {
					tr.printStackTrace();
				}
			}
		});
	}
}
