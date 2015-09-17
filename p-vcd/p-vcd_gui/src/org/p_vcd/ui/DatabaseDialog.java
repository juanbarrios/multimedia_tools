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
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListModel;
import javax.swing.SwingConstants;
import javax.swing.WindowConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.filechooser.FileNameExtensionFilter;

import org.p_vcd.model.MyUtil;
import org.p_vcd.model.Parameters;
import org.p_vcd.process.ProcessBase;
import org.p_vcd.process.ProcessDatabase;
import org.p_vcd.process.StatusListener;

import net.miginfocom.swing.MigLayout;

public class DatabaseDialog extends JDialog implements StatusListener {
	private static final long serialVersionUID = 1L;

	private final JPanel contentPanel = new JPanel();
	private JList<String> fileList = new JList<String>();
	private JTextField textField = new JTextField();
	private JLabel labelNumber = new JLabel();
	private JLabel lblWarning = new JLabel();
	private JLabel lblWait = new JLabel();
	private JTextArea textArea = new JTextArea();
	private VcdDialog vcdDialog;

	private String validateDbName(String name) {
		if (name == null || name.length() == 0) {
			return "required.";
		}
		for (char c : name.toCharArray()) {
			if ((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '_' && c != '-' && c != '='
					&& c != '.' && c != ',')
				return "invalid char '" + c + "'.";
		}
		File f = new File(Parameters.get().getDatabasesPath(), name);
		if (f.exists())
			return "name already used.";
		return null;
	}

	public boolean validateDbName() {
		String txt = textField.getText();
		String error = validateDbName(txt);
		if (error == null) {
			textField.setBackground(Color.WHITE);
			lblWarning.setText("");
			return true;
		} else {
			textField.setBackground(Color.RED);
			lblWarning.setText(error);
			return false;
		}
	}

	public DatabaseDialog(VcdDialog vcdDialog) {
		this.vcdDialog = vcdDialog;
		setSize(650, 400);
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		getContentPane().setLayout(new BorderLayout());
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		contentPanel.setLayout(new BorderLayout(0, 0));
		{
			JLabel lblNewDataset = new JLabel("New Dataset");
			lblNewDataset.setFont(new Font("Tahoma", Font.PLAIN, 16));
			lblNewDataset.setHorizontalAlignment(SwingConstants.CENTER);
			contentPanel.add(lblNewDataset, BorderLayout.NORTH);
		}
		{
			JPanel panel = new JPanel();
			contentPanel.add(panel, BorderLayout.CENTER);
			panel.setLayout(new MigLayout("", "[][grow]", "[][grow][][]"));
			{
				JLabel lblDatabaseName = new JLabel("Database name:");
				panel.add(lblDatabaseName, "cell 0 0,alignx trailing");
			}
			{
				textField.setHorizontalAlignment(SwingConstants.LEFT);
				textField.setColumns(20);
				textField.addKeyListener(new KeyAdapter() {
					@Override
					public void keyReleased(KeyEvent e) {
						validateDbName();
					}
				});
				panel.add(textField, "flowx,cell 1 0");
			}
			{
				JLabel lblListOfFiles = new JLabel("List of Files:");
				panel.add(lblListOfFiles, "cell 0 1");
			}
			{
				JScrollPane scrollPane = new JScrollPane();
				panel.add(scrollPane, "cell 1 1,grow");
				scrollPane.setViewportView(fileList);
			}
			{
				JLabel lblNumberOfFiles = new JLabel("Number of files:");
				panel.add(lblNumberOfFiles, "cell 0 2");
				panel.add(labelNumber, "cell 1 2");
			}
			{
				JButton btnAddFiles = new JButton("Add Files...");
				btnAddFiles.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						addVideosButton();
					}
				});
				panel.add(btnAddFiles, "flowx,cell 1 3");
			}
			{
				JButton btnRemoveFile = new JButton("Remove selected files");
				btnRemoveFile.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						removeSelectedFilesButton();
					}
				});
				panel.add(btnRemoveFile, "cell 1 3");
			}
			{
				lblWarning.setForeground(Color.RED);
				panel.add(lblWarning, "cell 1 0");
				lblWait.setForeground(Color.RED);
				panel.add(lblWait, "flowx,cell 1 3");
			}
		}
		{
			JScrollPane scrollPane = new JScrollPane();
			scrollPane.setPreferredSize(new Dimension(2, 80));
			contentPanel.add(scrollPane, BorderLayout.SOUTH);
			textArea.setFont(new Font("Monospaced", Font.PLAIN, 10));
			textArea.setForeground(Color.WHITE);
			textArea.setBackground(Color.BLACK);
			textArea.setEditable(false);
			textArea.setCursor(new Cursor(Cursor.TEXT_CURSOR));
			scrollPane.setViewportView(textArea);
		}
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton okButton = new JButton("OK");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						okButton();
					}
				});
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
			{
				JButton cancelButton = new JButton("Cancel");
				cancelButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						cancelButton();
					}
				});
				cancelButton.setActionCommand("Cancel");
				buttonPane.add(cancelButton);
			}
		}
	}

	private void addFilesRecursive(Set<String> sortedList, File f) {
		if (f.isDirectory()) {
			for (File ff : f.listFiles()) {
				addFilesRecursive(sortedList, ff);
			}
		} else if (f.isFile() && MyUtil.fileHasExtension(f, Parameters.get().getVideoExtensions())) {
			sortedList.add(f.getAbsolutePath().toString());
		}
	}

	private void addVideosButton() {
		JFileChooser fc = new JFileChooser();
		fc.setMultiSelectionEnabled(true);
		fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
		fc.setFileFilter(new FileNameExtensionFilter("videos files", Parameters.get().getVideoExtensions()));
		File def = Parameters.get().getDefaultDir_DatabaseFiles();
		if (def != null)
			fc.setCurrentDirectory(def);
		int returnVal = fc.showOpenDialog(this);
		if (returnVal == JFileChooser.APPROVE_OPTION) {
			Parameters.get().updateDefaultDir_DatabaseFiles(fc.getCurrentDirectory());
			Set<String> sortedMap = new TreeSet<String>();
			File[] ff = fc.getSelectedFiles();
			for (File f : ff) {
				addFilesRecursive(sortedMap, f);
			}
			ListModel<String> ll = fileList.getModel();
			for (int i = 0; i < ll.getSize(); i++) {
				sortedMap.add(ll.getElementAt(i));
			}
			DefaultListModel<String> dlm = new DefaultListModel<String>();
			for (String s : sortedMap)
				dlm.addElement(s);
			fileList.setModel(dlm);
			labelNumber.setText(dlm.size() + "");
		}
	}

	private void removeSelectedFilesButton() {
		int[] idxs = fileList.getSelectedIndices();
		if (idxs.length == 0)
			return;
		DefaultListModel<String> dlm = (DefaultListModel<String>) fileList.getModel();
		for (int i = idxs.length - 1; i >= 0; i--) {
			dlm.remove(idxs[i]);
		}
		labelNumber.setText(dlm.size() + "");
	}

	private void cancelButton() {
		this.dispose();
	}

	private void okButton() {
		try {
			if (!validateDbName()) {
				textField.requestFocus();
				return;
			}
			String dbName = textField.getText();
			ListModel<String> lm = fileList.getModel();
			List<String> list = new ArrayList<String>();
			for (int i = 0; i < lm.getSize(); i++) {
				list.add(lm.getElementAt(i));
			}
			sbOutput = null;
			textArea.setText("");
			lblWait.setText("");
			ProcessDatabase proc = new ProcessDatabase(Parameters.get().getDatabasesPath(), dbName, list);
			proc.asyncRun(this);
		} catch (Exception e1) {
			e1.printStackTrace();
		}
	}

	private StringBuffer sbOutput;

	@Override
	public synchronized void appendOutputLine(String line) {
		if (sbOutput == null)
			sbOutput = new StringBuffer();
		sbOutput.append(line).append("\n");
		textArea.setText(this.sbOutput.toString());
		textArea.setCaretPosition(textArea.getDocument().getLength());
	}

	@Override
	public void setPctProgress(String name, double pct) {
		lblWait.setText(name);
	}

	@Override
	public void callbackOnEnd(final ProcessBase process, boolean wasSuccessful) {
		if (!wasSuccessful)
			return;
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				DatabaseDialog.this.dispose();
				DatabaseDialog.this.vcdDialog.updateDatabases(((ProcessDatabase) process).getDbName());
			}
		});
	}
}
