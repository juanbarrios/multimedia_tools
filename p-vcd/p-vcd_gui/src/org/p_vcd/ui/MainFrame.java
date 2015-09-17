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
import java.awt.Component;
import java.awt.Dialog.ModalityType;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Label;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JSeparator;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;

import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;

import net.miginfocom.swing.MigLayout;

public class MainFrame extends JFrame {
	private static final long serialVersionUID = 1L;

	private JPanel contentPane = new JPanel();
	private JRadioButton rdbtnVCD = new JRadioButton("Video Copy Detection");
	private JRadioButton rdbtnIS = new JRadioButton("Image Search");
	private JRadioButton rdbtnINS = new JRadioButton("Object Localization");

	public MainFrame() {
		setSize(400, 330);
		setTitle("P-VCD");
		setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		setResizable(false);

		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);

		JMenu mnFile = new JMenu("File");
		menuBar.add(mnFile);

		JMenuItem mntmConfig = new JMenuItem("Configuration");
		mntmConfig.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				configMenu(null);
			}
		});
		mnFile.add(mntmConfig);

		JSeparator menuBar_1 = new JSeparator();
		mnFile.add(menuBar_1);

		JMenuItem mntmExit = new JMenuItem("Exit");
		mntmExit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				exitMenu();
			}
		});

		mnFile.add(mntmExit);

		Component glue = Box.createGlue();
		menuBar.add(glue);

		JMenu mnHelp = new JMenu("Help");
		menuBar.add(mnHelp);

		JMenuItem mntmAbout = new JMenuItem("About");
		mntmAbout.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				aboutMenu();
			}
		});
		mnHelp.add(mntmAbout);

		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(new BorderLayout(0, 0));

		Label label = new Label("P-VCD");
		label.setAlignment(Label.CENTER);
		label.setForeground(Color.RED);
		label.setFont(new Font("Dialog", Font.BOLD, 24));
		contentPane.add(label, BorderLayout.NORTH);

		JPanel panel = new JPanel();
		panel.setPreferredSize(new Dimension(100, 0));
		panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Applications",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		contentPane.add(panel, BorderLayout.CENTER);
		panel.setLayout(new MigLayout("", "[25px][grow]", "[grow][grow][grow][grow][grow][grow]"));

		ButtonGroup buttonGroup = new ButtonGroup();
		buttonGroup.add(rdbtnVCD);
		buttonGroup.add(rdbtnINS);
		buttonGroup.add(rdbtnIS);
		JLabel lblVCD = new JLabel("Locate the ocurrence of a given clip or frame in a video database.");
		JLabel lblINS = new JLabel("Locate the ocurrence of an given object in a video database.");
		JLabel lblIS = new JLabel("Retrieve similar images in a image database.");

		JPanel panel_1 = new JPanel();

		rdbtnVCD.setSelected(true);
		rdbtnVCD.setFont(new Font("Tahoma", Font.PLAIN, 13));
		rdbtnVCD.setEnabled(true);
		panel.add(rdbtnVCD, "cell 0 0 2 1,alignx left");
		panel.add(lblVCD, "cell 1 1");

		rdbtnINS.setFont(new Font("Tahoma", Font.PLAIN, 13));
		rdbtnINS.setEnabled(false);
		panel.add(rdbtnINS, "cell 0 2 2 1,alignx left");
		lblINS.setEnabled(false);
		panel.add(lblINS, "cell 1 3");

		rdbtnIS.setFont(new Font("Tahoma", Font.PLAIN, 13));
		rdbtnIS.setEnabled(false);
		panel.add(rdbtnIS, "cell 0 4 2 1,alignx left");
		lblIS.setEnabled(false);
		panel.add(lblIS, "cell 1 5");

		contentPane.add(panel_1, BorderLayout.SOUTH);

		JButton btnNext = new JButton("Start");
		btnNext.setFont(new Font("Tahoma", Font.PLAIN, 12));
		btnNext.setHorizontalTextPosition(SwingConstants.CENTER);
		panel_1.add(btnNext);

		btnNext.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				startButton();
			}
		});

	}

	private void exitMenu() {
		this.dispose();
	}

	private void aboutMenu() {
		AboutDialog v = new AboutDialog();
		v.setModalityType(ModalityType.APPLICATION_MODAL);
		v.setLocationRelativeTo(MainFrame.this);
		v.setVisible(true);
	}

	private void configMenu(String errorMessage) {
		ConfigDialog v = new ConfigDialog(errorMessage);
		v.setModalityType(ModalityType.APPLICATION_MODAL);
		v.setLocationRelativeTo(MainFrame.this);
		v.setVisible(true);
	}

	private void vcdDialog() {
		VcdDialog v = new VcdDialog();
		v.setModalityType(ModalityType.APPLICATION_MODAL);
		v.setLocationRelativeTo(MainFrame.this);
		v.setVisible(true);
	}

	private void invalidConfigMessage() {
		SwingUtil.showMessage("Invalid configuration. The configuration menu will be shown.");
	}

	private void startButton() {
		String errorMessage = Parameters.get().validate();
		if (errorMessage != null) {
			System.out.println(MyUtil.getFormateDate() + "P-VCD error in configuration.");
			invalidConfigMessage();
			configMenu(errorMessage);
		} else if (rdbtnVCD.isSelected()) {
			vcdDialog();
		}
	}

}
