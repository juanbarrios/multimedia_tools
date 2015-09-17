/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.Cursor;
import java.awt.Desktop;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.net.URI;

import javax.swing.JLabel;
import javax.swing.JOptionPane;

import org.p_vcd.model.MyUtil;

public class SwingUtil {
	public static boolean showConfirm(String message) {
		return JOptionPane.YES_OPTION == JOptionPane.showConfirmDialog(null, message, "Warning",
				JOptionPane.YES_NO_OPTION);
	}

	public static void showMessage(String message) {
		JOptionPane.showMessageDialog(null, message, "Warning", JOptionPane.WARNING_MESSAGE);
	}

	public static void showError(String title, Throwable tr) {
		System.out.println(MyUtil.getFormateDate() + title + " " + tr.toString());
		tr.printStackTrace();
		String txt = tr.toString();
		StringBuffer sb = new StringBuffer();
		while (txt.length() > 100) {
			sb.append(txt.substring(0, 100)).append("\n");
			txt = txt.substring(100);
		}
		sb.append(txt);
		JOptionPane.showMessageDialog(null, sb.toString(), title, JOptionPane.ERROR_MESSAGE);
	}

	public static JLabel createLink(String name, final String url) {
		JLabel lu = new JLabel("<html><a href='" + url + "'>" + name + "</a>");
		lu.setCursor(new Cursor(Cursor.HAND_CURSOR));
		lu.setToolTipText(url);
		lu.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				try {
					Desktop.getDesktop().browse(new URI(url));
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		});
		return lu;
	}

}
