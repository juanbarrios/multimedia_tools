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
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSpinner;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.UIManager;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;

import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;

import net.miginfocom.swing.MigLayout;

public class ConfigDialog extends JDialog {
	private static final long serialVersionUID = 1L;

	private final JPanel contentPanel = new JPanel();
	private JTextField user_data_dir;
	private JTextField vlc_path;
	private JTextField libvlc_path;
	private JTextField wget_bin;
	private JTextField youtubedl_path;
	private JTextField pvcd_path;
	private JSpinner max_cores;
	private JTextArea txtrMessage;

	public ConfigDialog(String errorMessage) {
		setBounds(100, 100, 600, 650);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		contentPanel.setLayout(new MigLayout("", "[grow,center]", "[103px][117px][117px][93px][100px][100px]"));
		{
			JPanel panel = new JPanel();
			panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "User Data",
					TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
			contentPanel.add(panel, "cell 0 0,alignx center,aligny top");
			panel.setLayout(new MigLayout("", "[100px:100px:100px][400px:400px:400px]", "[][]"));
			{
				JLabel lblNewLabel = new JLabel(
						"<html>The path to store the metadata produced by searches. This directory is created on startup (if it does not exist). Absolute and relative (to this file) paths are supported. Feel free to change it to your own preference.</html>\r\n");
				lblNewLabel.setSize(lblNewLabel.getPreferredSize());
				panel.add(lblNewLabel, "cell 0 0 2 1");
			}
			{
				JLabel lblPath = new JLabel("Data Folder");
				panel.add(lblPath, "cell 0 1");
			}
			{
				user_data_dir = new JTextField();
				user_data_dir.setText(Parameters.get().getUserDataPath().toString());
				panel.add(user_data_dir, "cell 1 1,growx");
				user_data_dir.setColumns(10);
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "P-VCD configuration",
					TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
			contentPanel.add(panel, "cell 0 1,alignx center,aligny top");
			panel.setLayout(new MigLayout("", "[100px:100px:100px][400px:400px:400px]", "[][][]"));
			{
				JLabel lblNewLabel_7 = new JLabel("<html>Path to P-VCD command line.</html>");
				panel.add(lblNewLabel_7, "cell 0 0 2 1");
			}
			{
				JLabel lblNewLabel_1 = new JLabel("PVCD path");
				panel.add(lblNewLabel_1, "cell 0 1");
			}
			{
				pvcd_path = new JTextField();
				pvcd_path.setText(Parameters.get().getPvcdPath());
				panel.add(pvcd_path, "cell 1 1,growx");
				pvcd_path.setColumns(10);
			}
			{
				JLabel lblNewLabel_4 = new JLabel("<html>Maximum number of threads to use:</html>");
				panel.add(lblNewLabel_4, "flowx,cell 0 2");
			}
			{
				max_cores = new JSpinner();
				max_cores.setModel(new SpinnerNumberModel(Parameters.get().getPvcdMaxCores(), 0, 64, 1));
				panel.add(max_cores, "cell 1 2");
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBorder(
					new TitledBorder(null, "VideoLAN Player", TitledBorder.LEADING, TitledBorder.TOP, null, null));
			contentPanel.add(panel, "cell 0 2,alignx center,aligny top");
			panel.setLayout(new MigLayout("", "[100px:100px:100px][400px:400px:400px]", "[][][][]"));
			{
				JLabel lblNewLabel_5 = new JLabel(
						"<html>Path to VideoLan player. Note: to avoid crashes the libvlc bitness must match JVM's: <b>"
								+ Parameters.get().getSystemBitness() + " bits</b>.</html>");
				panel.add(lblNewLabel_5, "cell 0 0 2 1");
			}
			{
				JLabel lblNewLabel_2 = new JLabel("VideoLAN command");
				panel.add(lblNewLabel_2, "cell 0 1");
			}
			{
				vlc_path = new JTextField();
				vlc_path.setText(Parameters.get().getVlcExePath());
				panel.add(vlc_path, "cell 1 1,growx");
				vlc_path.setColumns(10);
			}
			{
				JLabel lblNewLabel_3 = new JLabel("VideoLAN libvlc path");
				panel.add(lblNewLabel_3, "cell 0 2");
			}
			{
				libvlc_path = new JTextField();
				libvlc_path.setText(Parameters.get().getLibVlcPath());
				panel.add(libvlc_path, "cell 1 2,growx");
				libvlc_path.setColumns(10);
			}
			{
				JLabel lblNewLabel_12 = new JLabel(
						"<html>Download VideoLAN player from http://www.videolan.org/vlc/</html>");
				panel.add(lblNewLabel_12, "cell 0 3 2 1");
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBorder(new TitledBorder(null, "Wget", TitledBorder.LEADING, TitledBorder.TOP, null, null));
			contentPanel.add(panel, "cell 0 3,alignx center,aligny top");
			panel.setLayout(new MigLayout("", "[100px:100px:100px][400px:400px:400px]", "[][][]"));
			{
				JLabel lblNewLabel_9 = new JLabel("<html>Path to GNU's wget command line.</html>");
				panel.add(lblNewLabel_9, "cell 0 0 2 1");
			}
			{
				JLabel lblWgetBin = new JLabel("wget command");
				panel.add(lblWgetBin, "cell 0 1");
			}
			{
				wget_bin = new JTextField();
				wget_bin.setText(Parameters.get().getWgetExePath());
				panel.add(wget_bin, "cell 1 1,growx");
				wget_bin.setColumns(10);
			}
			{
				JLabel lblNewLabel_11 = new JLabel("<html>Download wget from http://www.gnu.org/software/wget/</html>");
				panel.add(lblNewLabel_11, "cell 0 2 2 1");
			}
		}
		{
			JPanel panel = new JPanel();
			panel.setBorder(
					new TitledBorder(null, "Youtube-Dl download", TitledBorder.LEADING, TitledBorder.TOP, null, null));
			contentPanel.add(panel, "cell 0 4,alignx center,aligny top");
			panel.setLayout(new MigLayout("", "[100px:100px:100px][400px:400px:400px]", "[][][]"));
			{
				JLabel lblNewLabel_10 = new JLabel("<html>Path to youtube-dl command line.</html>");
				panel.add(lblNewLabel_10, "cell 0 0 2 1");
			}
			{
				JLabel lblYoutubedl = new JLabel("youtube-dl");
				panel.add(lblYoutubedl, "cell 0 1");
			}
			{
				youtubedl_path = new JTextField();
				youtubedl_path.setText(Parameters.get().getYoutubedlExePath());
				panel.add(youtubedl_path, "cell 1 1,growx");
				youtubedl_path.setColumns(10);
			}
			{
				JLabel lblNewLabel_8 = new JLabel(
						"<html>Download Youtube-DL from http://rg3.github.io/youtube-dl/</html>");
				panel.add(lblNewLabel_8, "cell 0 2 2 1");
			}
		}
		{
			txtrMessage = new JTextArea();
			txtrMessage.setRows(2);
			txtrMessage.setColumns(60);
			txtrMessage.setForeground(Color.RED);
			txtrMessage.setLineWrap(true);
			txtrMessage.setFont(new Font("Tahoma", Font.PLAIN, 11));
			txtrMessage.setEditable(false);
			txtrMessage.setOpaque(false);
			if (errorMessage != null)
				txtrMessage.setText(errorMessage);
			contentPanel.add(txtrMessage, "cell 0 5,alignx center,aligny center");
		}
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton okButton = new JButton("OK");
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						actionOkButton();
					}
				});

			}
			{
				JButton cancelButton = new JButton("Cancel");
				cancelButton.setActionCommand("Cancel");
				buttonPane.add(cancelButton);
				cancelButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						ConfigDialog.this.dispose();
					}
				});
			}
		}
	}

	private boolean validateText(JTextField field, String name) {
		if (field.getText() == null || field.getText().trim().length() == 0) {
			txtrMessage.setText("invalid " + name);
			field.requestFocus();
			field.setBackground(Color.RED);
			System.out.println(MyUtil.getFormateDate() + "P-VCD configuration invalid " + name);
			return false;
		}
		txtrMessage.setText("");
		field.setBackground(Color.WHITE);
		return true;
	}

	public void actionOkButton() {
		try {
			if (!validateText(user_data_dir, "user data dir"))
				return;
			if (!validateText(vlc_path, "vlc path"))
				return;
			if (!validateText(libvlc_path, "libvlc path"))
				return;
			if (!validateText(wget_bin, "wget"))
				return;
			if (!validateText(youtubedl_path, "youtubedl"))
				return;
			if (!validateText(pvcd_path, "pvcd"))
				return;
			Parameters.get().updateUserDataPath(user_data_dir.getText());
			Parameters.get().updateVlcExePath(vlc_path.getText());
			Parameters.get().updateLibVlcPath(libvlc_path.getText());
			Parameters.get().updateWgetExePath(wget_bin.getText());
			Parameters.get().updateYoutubedlExePath(youtubedl_path.getText());
			Parameters.get().updatePvcdPath(pvcd_path.getText());
			Parameters.get().updatePvcdMaxCores(Integer.parseInt(max_cores.getValue().toString()));
			System.out.println(MyUtil.getFormateDate() + "P-VCD configuration updated OK.");
			this.dispose();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
