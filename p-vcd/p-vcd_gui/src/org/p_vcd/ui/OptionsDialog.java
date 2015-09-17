/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JSpinner;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;

import net.miginfocom.swing.MigLayout;

public class OptionsDialog extends JDialog {
	private static final long serialVersionUID = 1L;

	private final JPanel contentPanel = new JPanel();

	public OptionsDialog() {
		setSize(400, 300);
		setTitle("P-VCD - Options");
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setLayout(new FlowLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton okButton = new JButton("OK");
				okButton.setActionCommand("OK");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						OptionsDialog.this.dispose();
					}
				});
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
			{
				JButton cancelButton = new JButton("Cancel");
				cancelButton.setActionCommand("Cancel");
				cancelButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						OptionsDialog.this.dispose();
					}
				});
				buttonPane.add(cancelButton);
			}
		}
		{

			JPanel panel_4 = new JPanel();
			getContentPane().add(panel_4, BorderLayout.CENTER);
			panel_4.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));

			JPanel panel = new JPanel();
			panel.setBorder(new TitledBorder(null, "Descriptor", TitledBorder.LEADING, TitledBorder.TOP, null, null));
			panel_4.add(panel);
			panel.setLayout(new MigLayout("", "[][][grow][grow]", "[][][][][][][][]"));

			JLabel lblGlobalDescriptors = new JLabel("Global Descriptors:");
			panel.add(lblGlobalDescriptors, "cell 0 0 3 1");

			JCheckBox chckbxNewCheckBox = new JCheckBox("Spatio-Temporal");
			panel.add(chckbxNewCheckBox, "cell 3 0");

			JRadioButton rdbtnEdgeHistogramglobal = new JRadioButton("Edge Histogram");
			panel.add(rdbtnEdgeHistogramglobal, "cell 2 1 2 1");

			JLabel lblLocalDescriptors = new JLabel("Local Descriptors:");
			panel.add(lblLocalDescriptors, "cell 0 3 3 1");

			JRadioButton rdbtnSiftlocal = new JRadioButton("SIFT");
			panel.add(rdbtnSiftlocal, "cell 2 4 2 1");

			JComboBox<String> comboBox_1 = new JComboBox<String>();
			panel.add(comboBox_1, "cell 3 5,growx");

			JLabel lblNewLabel_1 = new JLabel("Video Segmentation:");
			panel.add(lblNewLabel_1, "cell 1 6 2 1");

			JComboBox<String> comboBox = new JComboBox<String>();
			comboBox.setModel(new DefaultComboBoxModel<String>(new String[] { "1 second", "2 seconds" }));
			panel.add(comboBox, "cell 2 7,growx");

			JPanel panel_1 = new JPanel();
			panel_1.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Similarity Search",
					TitledBorder.LEADING, TitledBorder.TOP, null, null));
			panel_4.add(panel_1);
			panel_1.setLayout(new MigLayout("gapy 10", "[150][grow]", "[][][][][]"));

			JRadioButton rdbtnNewRadioButton = new JRadioButton("New radio button");
			panel_1.add(rdbtnNewRadioButton, "cell 0 0");

			JRadioButton rdbtnNewRadioButton_1 = new JRadioButton("New radio button");
			panel_1.add(rdbtnNewRadioButton_1, "cell 0 1");

			JRadioButton rdbtnNewRadioButton_2 = new JRadioButton("New radio button");
			panel_1.add(rdbtnNewRadioButton_2, "cell 0 2");

			JRadioButton rdbtnNewRadioButton_3 = new JRadioButton("New radio button");
			panel_1.add(rdbtnNewRadioButton_3, "cell 0 3");

			JRadioButton rdbtnNewRadioButton_4 = new JRadioButton("New radio button");
			panel_1.add(rdbtnNewRadioButton_4, "cell 0 4");

			JPanel panel_2 = new JPanel();
			panel_2.setBorder(
					new TitledBorder(null, "Copy Localization", TitledBorder.LEADING, TitledBorder.TOP, null, null));
			panel_4.add(panel_2);
			panel_2.setLayout(new MigLayout("", "[][]", "[]"));

			JLabel lblMinimumLength = new JLabel("Minimum Length:");
			panel_2.add(lblMinimumLength, "cell 0 0");

			JSpinner spinner = new JSpinner();
			panel_2.add(spinner, "cell 1 0");
		}
	}

}
