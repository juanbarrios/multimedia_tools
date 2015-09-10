/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingConstants;
import javax.swing.WindowConstants;

import net.miginfocom.swing.MigLayout;

import org.p_vcd.model.Parameters;

public class AboutDialog extends JDialog {
	private static final long serialVersionUID = 1L;

	public AboutDialog() {
		getContentPane().setBackground(Color.WHITE);
		setTitle("About P-VCD");
		setSize(500, 560);
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		getContentPane().setLayout(new BorderLayout());
		{
			JPanel panel = new JPanel();
			panel.setBackground(Color.WHITE);
			getContentPane().add(panel, BorderLayout.SOUTH);
			{
				JButton btnOk = new JButton("OK");
				btnOk.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						AboutDialog.this.dispose();
					}
				});
				panel.add(btnOk);
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBackground(Color.WHITE);
			getContentPane().add(panel, BorderLayout.NORTH);
			FlowLayout fl_panel = new FlowLayout(FlowLayout.CENTER, 5, 5);
			fl_panel.setAlignOnBaseline(true);
			panel.setLayout(fl_panel);
			{
				JLabel lblNewLabel = new JLabel("P-VCD");
				lblNewLabel.setHorizontalAlignment(SwingConstants.CENTER);
				lblNewLabel.setForeground(Color.RED);
				lblNewLabel.setFont(new Font("Dialog", Font.BOLD, 24));
				panel.add(lblNewLabel);
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBackground(Color.WHITE);
			getContentPane().add(panel, BorderLayout.CENTER);
			panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
			{
				JPanel panel_1 = new JPanel();
				panel_1.setBackground(Color.WHITE);
				panel.add(panel_1);
				panel_1.setLayout(new MigLayout("", "[grow,trailing][grow]",
						"[]20px[]20px[]20px[]20px[]"));
				{
					JLabel lblPvcdVersion = new JLabel("P-VCD version:");
					lblPvcdVersion.setFont(new Font("Tahoma", Font.PLAIN, 14));
					panel_1.add(lblPvcdVersion, "cell 0 0");
				}
				{
					JLabel lblNewLabel_2 = new JLabel();
					lblNewLabel_2.setText(Parameters.VERSION_NAME);
					lblNewLabel_2.setFont(new Font("Tahoma", Font.BOLD, 14));
					panel_1.add(lblNewLabel_2, "cell 1 0");
				}
				{
					JLabel lblHomepage = new JLabel("Home Page:");
					lblHomepage.setFont(new Font("Tahoma", Font.PLAIN, 14));
					panel_1.add(lblHomepage, "cell 0 1");
				}
				{
					JLabel lblhomepage = SwingUtil.createLink(
							"http://p-vcd.org/",
							"http://p-vcd.org/");
					lblhomepage.setFont(new Font("Tahoma", Font.PLAIN, 14));
					panel_1.add(lblhomepage, "cell 1 1");
				}
				{
					JLabel lblPvcdIsA = new JLabel("Components:");
					panel_1.add(lblPvcdIsA, "cell 0 2");
					lblPvcdIsA.setFont(new Font("Tahoma", Font.PLAIN, 14));
				}
				{
					JLabel lbltheNativeCore = new JLabel(
							"<html><body style='width:100%'>\r\n<b>libpvcd</b>: The native core (written in C).<br>\r\n<b>CLI</b>: The command line interface (written in C).<br>\r\n<b>GUI</b>: This graphic user interface (written in Java).\r\n");
					panel_1.add(lbltheNativeCore, "cell 1 2");
					lbltheNativeCore
							.setFont(new Font("Tahoma", Font.PLAIN, 14));
				}
				{
					JLabel lblTheAuthorsAre = new JLabel("Authors:");
					panel_1.add(lblTheAuthorsAre, "cell 0 3");
					lblTheAuthorsAre
							.setFont(new Font("Tahoma", Font.PLAIN, 14));
				}
				{
					JLabel lblJuan = SwingUtil.createLink(
							"Juan Manuel Barrios",
							"http://www.dcc.uchile.cl/~jbarrios/");
					lblJuan.setText(lblJuan.getText() + " (libpvcd, CLI, GUI).");
					panel_1.add(lblJuan, "flowy,cell 1 3");
					lblJuan.setFont(new Font("Tahoma", Font.PLAIN, 14));

					JLabel lblOrand = SwingUtil.createLink("Orand S.A. Chile",
							"http://www.orand.cl/");
					lblOrand.setText(lblOrand.getText() + " (GUI).");
					panel_1.add(lblOrand, "cell 1 3");
					lblOrand.setFont(new Font("Tahoma", Font.PLAIN, 14));

					JLabel lblLicense = new JLabel("License:");
					lblLicense.setFont(new Font("Tahoma", Font.PLAIN, 14));
					panel_1.add(lblLicense, "cell 0 4,aligny baseline");

					JLabel lblThisIsFree = new JLabel(
							"<html><body style='width:100%'><b>P-VCD is free software (BSD License)</b>.");
					panel_1.add(lblThisIsFree, "flowx,cell 1 4,aligny baseline");
					lblThisIsFree.setFont(new Font("Tahoma", Font.PLAIN, 14));
					JButton btnSeeLicense = new JButton("License");
					btnSeeLicense.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							LicenseDialog.openLicenseWindow(AboutDialog.this,
									"P-VCD " + Parameters.VERSION_NAME
											+ " (libpvcd, CLI, GUI)",
									"http://p-vcd.org/",
									"BSD 2-Clause License", "pvcd.license.txt");
						}
					});
					panel_1.add(btnSeeLicense, "cell 1 4,aligny baseline");
				}
			}
			{
				JPanel panel_1 = new JPanel();
				panel_1.setBackground(Color.WHITE);
				panel_1.setPreferredSize(new Dimension(450, 190));
				panel.add(panel_1);
				panel_1.setLayout(new BorderLayout(0, 0));
				{
					JLabel lblThisIsFree = new JLabel(
							"<html><body style='width:100%'>P-VCD uses the following third-party open source software:");
					panel_1.add(lblThisIsFree, BorderLayout.NORTH);
					lblThisIsFree.setFont(new Font("Tahoma", Font.PLAIN, 11));
				}
				{
					JScrollPane scrollPane = new JScrollPane();
					panel_1.add(scrollPane, BorderLayout.CENTER);
					JPanel panel_2 = new JPanel();
					panel_2.setBackground(Color.WHITE);
					scrollPane.setViewportView(panel_2);
					int row = 0;
					panel_2.setLayout(new MigLayout("",
							"[80px,center][grow,center][grow][grow,center]",
							"[][][][][][][]"));
					createLicenseRow(panel_2, row++, "OpenCV", "2.4.9",
							"http://www.opencv.org/", "BSD license",
							"opencv.license.txt");
					createLicenseRow(panel_2, row++, "VLFeat", "0.9.20",
							"http://www.vlfeat.org/", "BSD license",
							"vlfeat.license.txt");
					createLicenseRow(panel_2, row++, "FLANN", "1.8.4",
							"http://www.cs.ubc.ca/research/flann/",
							"BSD license", "flann.license.txt");
					createLicenseRow(panel_2, row++, "FFmpeg", "2.6.1",
							"http://www.ffmpeg.org/",
							"GNU General Public License version 3",
							"ffmpeg.license.txt");
					createLicenseRow(panel_2, row++, "wget", "1.11.4",
							"http://www.gnu.org/software/wget/",
							"GNU General Public License version 3",
							"wget.license.txt");
					createLicenseRow(panel_2, row++, "youtube-dl",
							"2015.04.09", "http://rg3.github.io/youtube-dl/",
							"Public Domain", "youtube-dl.license.txt");
					createLicenseRow(panel_2, row++, "VideoLan", "2.1.5",
							"http://www.videolan.org/vlc/",
							"GNU General Public License version 2",
							"vlc.license.txt");
					createLicenseRow(panel_2, row++, "vlcj", "3.6.0",
							"https://code.google.com/p/vlcj/",
							"GNU General Public License version 3",
							"vlcj.license.txt");
				}
			}
		}
	}

	private static void createLicenseRow(final JPanel panel, int rowId,
			final String libName, final String libVersion, final String libUrl,
			final String licenseName, final String filename) {
		JLabel ln = new JLabel("<html><b>" + libName + "</b>");
		JLabel lv = new JLabel("<html><b>" + libVersion + "</b>");
		JLabel lu = SwingUtil.createLink(libUrl, libUrl);
		JLabel ll = new JLabel("<html><a href='#'>License</a>");
		ll.setCursor(new Cursor(Cursor.HAND_CURSOR));
		ll.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				try {
					LicenseDialog.openLicenseWindow(panel, libName + " "
							+ libVersion, libUrl, licenseName, filename);
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		});
		panel.add(ln, "cell 0 " + rowId);
		panel.add(lv, "cell 1 " + rowId);
		panel.add(lu, "cell 2 " + rowId);
		panel.add(ll, "cell 3 " + rowId);
	}

}
