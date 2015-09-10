/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;
import javax.swing.WindowConstants;
import javax.swing.border.EmptyBorder;

import net.miginfocom.swing.MigLayout;

import org.apache.commons.io.FileUtils;
import org.p_vcd.model.Parameters;

public class LicenseDialog extends JDialog {
	private static final long serialVersionUID = 1L;

	public LicenseDialog(String libName, String libHomepage,
			String licenseName, String licenseFilename) {
		setSize(680, 480);
		setTitle(libName);
		getContentPane().setLayout(new BorderLayout());
		{
			JPanel panel = new JPanel();
			getContentPane().add(panel, BorderLayout.NORTH);
			panel.setLayout(new MigLayout("", "[grow,trailing][grow]",
					"[20px][][]"));
			{
				JLabel lblSoft = new JLabel(libName);
				lblSoft.setFont(new Font("Tahoma", Font.BOLD, 16));
				panel.add(lblSoft, "center,cell 0 0 2 1");
			}
			{
				JLabel lblHomePage = new JLabel("Home page:");
				lblHomePage.setFont(new Font("Tahoma", Font.BOLD, 14));
				panel.add(lblHomePage, "cell 0 1");
			}
			{
				JLabel lblHome = SwingUtil.createLink(libHomepage, libHomepage);
				lblHome.setFont(new Font("Tahoma", Font.PLAIN, 14));
				panel.add(lblHome, "cell 1 1");
			}
			{
				JLabel lblNewLabel = new JLabel("License:");
				lblNewLabel.setFont(new Font("Tahoma", Font.BOLD, 14));
				panel.add(lblNewLabel, "cell 0 2");
			}
			{
				JLabel lblLicense = new JLabel(licenseName);
				lblLicense.setFont(new Font("Tahoma", Font.PLAIN, 14));
				panel.add(lblLicense, "cell 1 2");
			}
		}
		{
			JPanel panel = new JPanel();
			getContentPane().add(panel, BorderLayout.CENTER);
			panel.setLayout(new FlowLayout());
			panel.setBorder(new EmptyBorder(5, 5, 5, 5));
			JScrollPane scrollPane = new JScrollPane();
			scrollPane.setPreferredSize(new Dimension(600, 300));
			scrollPane
					.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
			panel.add(scrollPane);
			{
				JTextArea txtLicense = new JTextArea();
				txtLicense.setEditable(false);
				txtLicense.setFont(new Font("Monospaced", Font.PLAIN, 11));
				txtLicense.setWrapStyleWord(true);
				txtLicense.setLineWrap(true);
				try {
					File lic = new File(Parameters.get().LICENSES_DIR,
							licenseFilename);
					if (lic.exists() && lic.isFile() && lic.canRead()) {
						String text = FileUtils.readFileToString(lic, "UTF-8");
						txtLicense.setText(text);
					}
					txtLicense.setCaretPosition(0);
				} catch (Exception e) {
					e.printStackTrace();
				}
				scrollPane.setViewportView(txtLicense);
			}
		}
		{
			JPanel panel = new JPanel();
			getContentPane().add(panel, BorderLayout.SOUTH);
			panel.setLayout(new FlowLayout(FlowLayout.CENTER));
			JButton okButton = new JButton("OK");
			okButton.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					LicenseDialog.this.dispose();
				}
			});
			okButton.setActionCommand("OK");
			panel.add(okButton);
			getRootPane().setDefaultButton(okButton);
		}
	}

	public static void openLicenseWindow(Component windowPanel, String libName,
			String libHomepage, String licenseName, String licenseFilename) {
		LicenseDialog v = new LicenseDialog(libName, libHomepage, licenseName,
				licenseFilename);
		v.setModalityType(ModalityType.APPLICATION_MODAL);
		v.setLocationRelativeTo(windowPanel);
		v.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		v.setVisible(true);
	}
}
