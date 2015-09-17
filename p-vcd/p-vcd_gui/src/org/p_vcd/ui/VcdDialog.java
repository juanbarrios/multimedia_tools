/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.BorderLayout;
import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import javax.swing.ButtonGroup;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import javax.swing.WindowConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.filechooser.FileNameExtensionFilter;

import org.apache.commons.io.FileUtils;
import org.apache.commons.validator.routines.UrlValidator;
import org.p_vcd.model.MyUtil;
import org.p_vcd.model.PVCDObj;
import org.p_vcd.model.Parameters;
import org.p_vcd.model.VideoCopy;
import org.p_vcd.model.VideoDatabase;
import org.p_vcd.process.ProcessBase;
import org.p_vcd.process.ProcessExternalVlcViewer;
import org.p_vcd.process.ProcessOptions;
import org.p_vcd.process.ProcessVCD;
import org.p_vcd.process.StatusListener;

import net.miginfocom.swing.MigLayout;

public class VcdDialog extends JDialog implements StatusListener {
	private static final long serialVersionUID = 1L;

	private final JPanel contentPanel = new JPanel();

	// step 1 reference
	private JLabel lbl_refDbFiles = new JLabel();
	private JLabel lbl_refDbMetadata = new JLabel();
	private JLabel lbl_refDbTitle = new JLabel();
	private Map<VideoDatabase, JCheckBox> map_refDbCheck = new TreeMap<VideoDatabase, JCheckBox>();
	private JPanel panel_refDatabasesList = new JPanel();

	// step 2 query
	private JRadioButton radio_queryFile = new JRadioButton("Image or Video File");
	private JLabel lbl_queryFile = new JLabel();
	private JRadioButton radio_queryUrl = new JRadioButton("Image or Video URL");
	private JTextField txt_queryUrl = new JTextField();
	private JRadioButton radio_queryDb = new JRadioButton("Video Database");
	private JComboBox<String> comboBox_queryDb = new JComboBox<String>();
	private JLabel lbl_queryDb = new JLabel();
	private File queryFile;
	private URL queryUrl;
	private VideoDatabase queryDb;

	// step 3 options
	private JRadioButton radio_searchByGlobal = new JRadioButton("Search based on global descriptor (recommended)");
	private JRadioButton radio_searchByLocal = new JRadioButton("Search based on local descriptors");

	// step 4 run
	private JTextArea textConsole = new JTextArea();
	private JProgressBar progressBar = new JProgressBar();
	private JLabel lblProgress = new JLabel();

	// step 5 results
	private JPanel panelResults = new JPanel();
	private List<VideoCopy> resultDetections;

	// global
	private int currentStep;
	private ProcessVCD currentSearch;

	public VcdDialog() {
		setSize(700, 450);
		setTitle("P-VCD - Video Copy Detection");
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		this.currentStep = 1;

		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		contentPanel.setLayout(new CardLayout(0, 0));
		{
			JPanel panel_Step1 = new JPanel();
			contentPanel.add(panel_Step1, "card_step1");
			panel_Step1.setLayout(new BorderLayout(0, 0));
			JLabel lblTitle = new JLabel("STEP 1 - Video Database (the \"known\")");
			panel_Step1.add(lblTitle, BorderLayout.NORTH);
			lblTitle.setHorizontalAlignment(SwingConstants.CENTER);
			lblTitle.setFont(new Font("Tahoma", Font.PLAIN, 18));

			JPanel panel_1 = new JPanel();
			panel_Step1.add(panel_1, BorderLayout.CENTER);
			panel_1.setLayout(new MigLayout("", "[250px,grow][20px][250px,grow]", "[][][230px,grow][][][]"));

			JLabel lblNewLabel = new JLabel("Please select the video databases to search in:");
			panel_1.add(lblNewLabel, "cell 0 0");

			panel_1.add(lbl_refDbTitle, "cell 2 1");

			JScrollPane scrollPane_1 = new JScrollPane();
			panel_1.add(scrollPane_1, "cell 0 1 1 3,grow");
			panel_refDatabasesList.setBackground(Color.WHITE);

			scrollPane_1.setViewportView(panel_refDatabasesList);
			panel_refDatabasesList.setLayout(new MigLayout("gapy 10", "[200px]", "[][]"));

			JScrollPane scrollPane = new JScrollPane();
			scrollPane.setBorder(null);
			panel_1.add(scrollPane, "cell 2 2,grow");

			scrollPane.setViewportView(lbl_refDbFiles);

			panel_1.add(lbl_refDbMetadata, "cell 2 3");

			JButton btnNewDatabase = new JButton("New database...");
			panel_1.add(btnNewDatabase, "cell 0 4");
			btnNewDatabase.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					newDatabaseButton();
				}
			});
		}
		{
			JPanel panel_Step2 = new JPanel();
			contentPanel.add(panel_Step2, "card_step2");
			panel_Step2.setLayout(new BorderLayout(0, 0));
			JLabel lblTitle = new JLabel("STEP 2 - Query (the \"unknown\")");
			panel_Step2.add(lblTitle, BorderLayout.NORTH);
			lblTitle.setHorizontalAlignment(SwingConstants.CENTER);
			lblTitle.setFont(new Font("Tahoma", Font.PLAIN, 18));

			JPanel panel = new JPanel();
			panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Query", TitledBorder.LEADING,
					TitledBorder.TOP, null, null));
			panel_Step2.add(panel, BorderLayout.CENTER);
			panel.setLayout(
					new MigLayout("", "[160px][grow]", "[25px][grow,top][25px][grow,top][25px][grow,top][grow]"));

			ButtonGroup queryButtonGroup = new ButtonGroup();

			queryButtonGroup.add(radio_queryFile);
			radio_queryFile.setFont(new Font("Tahoma", Font.PLAIN, 13));
			panel.add(radio_queryFile, "cell 0 0");

			lbl_queryFile.addMouseListener(new MouseAdapter() {
				@Override
				public void mouseClicked(MouseEvent e) {
					radio_queryFile.setSelected(true);
				}
			});
			panel.add(lbl_queryFile, "flowy,cell 1 0");

			JButton btnSelectFile = new JButton("Select File...");
			btnSelectFile.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					selectFileButton();
				}
			});
			panel.add(btnSelectFile, "cell 1 0");

			JLabel lblIumageOrVideo = new JLabel("Select an image or video in the local machine.");
			panel.add(lblIumageOrVideo, "cell 1 1");

			queryButtonGroup.add(radio_queryUrl);
			radio_queryUrl.setFont(new Font("Tahoma", Font.PLAIN, 13));
			panel.add(radio_queryUrl, "cell 0 2");

			txt_queryUrl.addKeyListener(new KeyAdapter() {
				@Override
				public void keyReleased(KeyEvent e) {
					radio_queryUrl.setSelected(true);
				}
			});
			txt_queryUrl.addMouseListener(new MouseAdapter() {
				@Override
				public void mouseClicked(MouseEvent e) {
					radio_queryUrl.setSelected(true);
				}
			});
			txt_queryUrl.addFocusListener(new FocusAdapter() {
				@Override
				public void focusGained(FocusEvent e) {
					radio_queryUrl.setSelected(true);
				}
			});
			txt_queryUrl.setText("http://");
			panel.add(txt_queryUrl, "flowy,cell 1 2");
			txt_queryUrl.setColumns(50);

			JLabel lblUrlToA = new JLabel(
					"<html>Enter the URL to download an image or video.<br>You can use a URL like: http://www.youtube.com/watch?v=... </html>");
			panel.add(lblUrlToA, "flowy,cell 1 3");

			queryButtonGroup.add(radio_queryDb);
			radio_queryDb.setFont(new Font("Tahoma", Font.PLAIN, 13));
			panel.add(radio_queryDb, "cell 0 4");

			comboBox_queryDb.setMaximumRowCount(12);
			comboBox_queryDb.setPreferredSize(new Dimension(100, 20));
			comboBox_queryDb.setMinimumSize(new Dimension(100, 20));
			comboBox_queryDb.addPropertyChangeListener(new PropertyChangeListener() {
				public void propertyChange(PropertyChangeEvent evt) {
					if (comboBox_queryDb.getSelectedIndex() > 0)
						radio_queryDb.setSelected(true);
					updateQueryDbDetail();
				}
			});
			comboBox_queryDb.addKeyListener(new KeyAdapter() {
				@Override
				public void keyReleased(KeyEvent e) {
					if (comboBox_queryDb.getSelectedIndex() > 0)
						radio_queryDb.setSelected(true);
					updateQueryDbDetail();
				}
			});

			panel.add(comboBox_queryDb, "flowx,cell 1 4");

			panel.add(lbl_queryDb, "cell 1 4");

			JLabel lblPleaseNopteThe = new JLabel("<html>A search is run for each video in the database.</html>");
			panel.add(lblPleaseNopteThe, "cell 1 5");

		}
		{
			JPanel panel_Step3 = new JPanel();
			contentPanel.add(panel_Step3, "card_step3");
			panel_Step3.setLayout(new BorderLayout(0, 0));

			JLabel lblTitle = new JLabel("STEP 3 - Search Options");
			lblTitle.setFont(new Font("Tahoma", Font.PLAIN, 18));
			lblTitle.setHorizontalAlignment(SwingConstants.CENTER);
			panel_Step3.add(lblTitle, BorderLayout.NORTH);

			JPanel panel2 = new JPanel();
			panel2.setLayout(new FlowLayout());
			panel_Step3.add(panel2, BorderLayout.CENTER);
			JPanel panel = new JPanel();
			panel2.add(panel);
			panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Basic Options",
					TitledBorder.LEADING, TitledBorder.TOP, null, null));
			panel.setLayout(new MigLayout("gapy 5px", "[30px][101px]", "[][][][][][][20px]"));

			ButtonGroup buttonGroup = new ButtonGroup();

			buttonGroup.add(radio_searchByGlobal);
			radio_searchByGlobal.setFont(new Font("Tahoma", Font.PLAIN, 13));
			radio_searchByGlobal.setSelected(true);
			panel.add(radio_searchByGlobal, "cell 0 0 2 1,alignx left,aligny top");

			JLabel lblNewLabel_2 = new JLabel("Detects most of the copies that are visually alike to the original.");
			panel.add(lblNewLabel_2, "cell 1 1");

			JButton btnOptions = new JButton("Advanced Options...");
			btnOptions.setEnabled(false);
			panel.add(btnOptions, "cell 1 2");

			buttonGroup.add(radio_searchByLocal);
			radio_searchByLocal.setFont(new Font("Tahoma", Font.PLAIN, 13));
			panel.add(radio_searchByLocal, "cell 0 4 2 1,alignx left,aligny top");

			JLabel lblNewLabel_3 = new JLabel(
					"Requires more resources (disk space, search time, memory) but can detect more copies.");
			panel.add(lblNewLabel_3, "cell 1 5");

			JButton btnOptions_1 = new JButton("Advanced Options...");
			btnOptions_1.setEnabled(false);
			panel.add(btnOptions_1, "cell 1 6");

		}
		{
			JPanel panel_Step4 = new JPanel();
			contentPanel.add(panel_Step4, "card_step4");
			panel_Step4.setLayout(new BorderLayout(0, 0));

			JLabel lblTitle = new JLabel("STEP 4 - Search");
			lblTitle.setHorizontalAlignment(SwingConstants.CENTER);
			lblTitle.setFont(new Font("Tahoma", Font.PLAIN, 18));
			panel_Step4.add(lblTitle, BorderLayout.NORTH);

			JScrollPane scrollPane = new JScrollPane();
			panel_Step4.add(scrollPane, BorderLayout.CENTER);

			textConsole.setFont(new Font("Monospaced", Font.PLAIN, 11));
			textConsole.setForeground(Color.WHITE);
			textConsole.setBackground(Color.BLACK);
			textConsole.setEditable(false);
			textConsole.setCursor(new Cursor(Cursor.TEXT_CURSOR));
			textConsole.setText(
					"Press 'Next' button to start the search...\nNote: Depending on the selected options, the search may take up to several hours.");
			scrollPane.setViewportView(textConsole);

			JPanel panel = new JPanel();
			panel_Step4.add(panel, BorderLayout.SOUTH);
			panel.setLayout(new GridLayout(0, 1, 0, 0));

			JSeparator separator = new JSeparator();
			separator.setPreferredSize(new Dimension(0, 1));
			panel.add(separator);
			progressBar.setStringPainted(true);
			progressBar.setPreferredSize(new Dimension(350, 20));
			panel.add(progressBar);

			lblProgress.setHorizontalAlignment(SwingConstants.CENTER);
			lblProgress.setFont(new Font("Tahoma", Font.PLAIN, 14));
			panel.add(lblProgress);
		}
		{
			JPanel panel_Step5 = new JPanel();
			contentPanel.add(panel_Step5, "card_step5");
			panel_Step5.setLayout(new BorderLayout(0, 0));

			JLabel lblTitle = new JLabel("STEP 5 - Results");
			lblTitle.setHorizontalAlignment(SwingConstants.CENTER);
			lblTitle.setFont(new Font("Tahoma", Font.PLAIN, 18));
			panel_Step5.add(lblTitle, BorderLayout.NORTH);

			JScrollPane scrollPane = new JScrollPane();
			panel_Step5.add(scrollPane, BorderLayout.CENTER);

			scrollPane.setViewportView(panelResults);
			panelResults.setLayout(new MigLayout("gapy 10, gapx 20",
					"[120px,center][150px,center,grow][150px,center,grow]", "[25px][]"));

		}
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton prevButton = new JButton("Previous");
				prevButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						previousButton();
					}
				});
				prevButton.setActionCommand("Previous");
				buttonPane.add(prevButton);
			}
			{
				JButton okButton = new JButton("Next");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						nextButton();
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
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosed(WindowEvent e) {
				closeWindow();
			}
		});
		updateDatabases(null);
	}

	public void updateDatabases(String defaultValue) {
		List<VideoDatabase> refDbs = VideoDatabase.getReferenceDatabases();
		// remove from map
		List<VideoDatabase> entriesToRemove = new ArrayList<VideoDatabase>();
		for (VideoDatabase edb : map_refDbCheck.keySet()) {
			if (!refDbs.contains(edb))
				entriesToRemove.add(edb);
		}
		for (VideoDatabase edb : entriesToRemove)
			map_refDbCheck.remove(edb);
		// add to map
		List<VideoDatabase> entriesToAdd = new ArrayList<VideoDatabase>();
		for (VideoDatabase db : refDbs) {
			if (!map_refDbCheck.containsKey(db))
				entriesToAdd.add(db);
		}
		for (VideoDatabase db : entriesToAdd) {
			JCheckBox chkbox = new JCheckBox(db.getName() + "  (" + db.getFileList().size() + " files, "
					+ MyUtil.getSecondsToHHMMSS(db.getTotalSeconds()) + " length)");
			chkbox.setBackground(Color.WHITE);
			chkbox.setFont(new Font("Tahoma", Font.PLAIN, 13));
			chkbox.addChangeListener(new ChangeListener() {
				@Override
				public void stateChanged(ChangeEvent e) {
					updateReferenceDbDetail();
				}
			});
			map_refDbCheck.put(db, chkbox);
		}
		// draw checkboxes
		panel_refDatabasesList.removeAll();
		int cont = 0;
		for (JCheckBox chkbox : map_refDbCheck.values()) {
			panel_refDatabasesList.add(chkbox, "cell 0 " + cont);
			cont++;
		}
		panel_refDatabasesList.validate();
		// default value
		if (defaultValue != null) {
			for (Map.Entry<VideoDatabase, JCheckBox> entry : map_refDbCheck.entrySet()) {
				if (!defaultValue.equals(entry.getKey().getName()))
					continue;
				JCheckBox chkbox = entry.getValue();
				chkbox.setSelected(true);
				chkbox.requestFocus();
				break;
			}
		}
		List<String> listQ = new ArrayList<String>();
		listQ.add("-- select a db --");
		for (VideoDatabase db : refDbs) {
			listQ.add(db.getName());
		}
		comboBox_queryDb.setModel(new DefaultComboBoxModel<String>(listQ.toArray(new String[0])));
		updateReferenceDbDetail();
		updateQueryDbDetail();
	}

	private List<VideoDatabase> getSelectedRefDbs() {
		List<VideoDatabase> selectedRefDbs = new ArrayList<VideoDatabase>();
		for (VideoDatabase db : map_refDbCheck.keySet()) {
			JCheckBox chkbox = map_refDbCheck.get(db);
			if (chkbox.isSelected())
				selectedRefDbs.add(db);
		}
		return selectedRefDbs;
	}

	private void updateReferenceDbDetail() {
		try {
			List<VideoDatabase> selectedRefDbs = getSelectedRefDbs();
			if (selectedRefDbs.size() == 0) {
				lbl_refDbTitle.setText("");
				lbl_refDbFiles.setText("");
				lbl_refDbMetadata.setText("");
				return;
			}
			lbl_refDbTitle.setText("Files in " + selectedRefDbs.size() + " selected databases:");
			int contFiles = 0;
			long contSize = 0;
			double contTime = 0;
			StringBuffer sb = new StringBuffer("<html>");
			for (VideoDatabase dbr : selectedRefDbs) {
				for (PVCDObj p : dbr.getFileList()) {
					sb.append(p.getFilePath()).append("<br>");
				}
				contFiles += dbr.getFileList().size();
				contSize += dbr.getTotalBytesVideos();
				contTime += dbr.getTotalSeconds();
			}
			String txtSize = FileUtils.byteCountToDisplaySize(contSize);
			String txtTime = MyUtil.getSecondsToHHMMSS(contTime);
			lbl_refDbFiles.setText(sb.toString());
			lbl_refDbMetadata.setText(contFiles + " files, " + txtSize + " total size, " + txtTime + " total length.");
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	private void updateQueryDbDetail() {
		try {
			if (comboBox_queryDb.getSelectedIndex() <= 0) {
				queryDb = null;
			} else {
				String dbName = (String) comboBox_queryDb.getSelectedItem();
				if (queryDb == null || !queryDb.getName().equals(dbName)) {
					queryDb = null;
					for (VideoDatabase db : VideoDatabase.getReferenceDatabases()) {
						if (db.getName().equals(dbName)) {
							queryDb = db;
							break;
						}
					}
				}
			}
			if (queryDb == null) {
				lbl_queryDb.setText("");
			} else {
				lbl_queryDb.setText(queryDb.getFileList().size() + " files, "
						+ MyUtil.getSecondsToHHMMSS(queryDb.getTotalSeconds()) + " total time.");
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private void newDatabaseButton() {
		DatabaseDialog v = new DatabaseDialog(VcdDialog.this);
		v.setModalityType(ModalityType.APPLICATION_MODAL);
		v.setLocationRelativeTo(VcdDialog.this);
		v.setVisible(true);
	}

	private void selectFileButton() {
		JFileChooser fc = new JFileChooser();
		fc.setMultiSelectionEnabled(false);
		fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		fc.setFileFilter(
				new FileNameExtensionFilter("images and videos", Parameters.get().getImageAndVideoExtensions()));
		File def = Parameters.get().getDefaultDir_QuerySelection();
		if (def != null)
			fc.setCurrentDirectory(def);
		int returnVal = fc.showOpenDialog(this);
		if (returnVal == JFileChooser.APPROVE_OPTION) {
			Parameters.get().updateDefaultDir_QuerySelection(fc.getCurrentDirectory());
			queryFile = fc.getSelectedFile();
			lbl_queryFile.setText(queryFile.getAbsolutePath());
			radio_queryFile.setSelected(true);
		}
	}

	private void previousButton() {
		if (currentStep > 1) {
			currentStep--;
			CardLayout cl = (CardLayout) contentPanel.getLayout();
			cl.show(contentPanel, "card_step" + currentStep);
		}
	}

	private boolean validateCurrentStep() {
		if (currentStep == 1) {
			List<VideoDatabase> selectedRefDbs = getSelectedRefDbs();
			if (selectedRefDbs.size() == 0) {
				SwingUtil.showMessage("Must select a reference database");
				return false;
			}
		} else if (currentStep == 2) {
			if (radio_queryFile.isSelected()) {
				if (queryFile == null || !queryFile.isFile()) {
					SwingUtil.showMessage("Invalid query file");
					return false;
				}
			} else if (radio_queryUrl.isSelected()) {
				try {
					String value = txt_queryUrl.getText();
					if (UrlValidator.getInstance().isValid(value))
						queryUrl = new URL(value);
					else
						throw new Exception();
				} catch (Throwable t) {
					SwingUtil.showMessage("Invalid URL");
					txt_queryUrl.requestFocus();
					queryUrl = null;
					return false;
				}
			} else if (radio_queryDb.isSelected()) {
				if (queryDb == null) {
					SwingUtil.showMessage("Must select a query database");
					comboBox_queryDb.requestFocus();
					return false;
				}
			} else {
				SwingUtil.showMessage("Must select the query type: File, URL or Database.");
				return false;
			}
		} else if (currentStep == 3) {
			if (!radio_searchByGlobal.isSelected() && !radio_searchByLocal.isSelected()) {
				SwingUtil.showMessage("Must select the search type.");
				return false;
			}
		}
		return true;
	}

	private void nextButton() {
		if (currentStep < 4) {
			if (validateCurrentStep()) {
				currentStep++;
				CardLayout cl = (CardLayout) contentPanel.getLayout();
				cl.show(contentPanel, "card_step" + currentStep);
			}
		} else if (currentStep == 4) {
			if (currentSearch == null || currentSearch.hasEnded()) {
				textConsole.setText("");
				this.sbOutput = new StringBuffer();
				ProcessOptions options = null;
				if (radio_searchByGlobal.isSelected()) {
					options = ProcessOptions.getDefaultGlobalOptions();
				} else if (radio_searchByLocal.isSelected()) {
					options = ProcessOptions.getDefaultLocalOptions();
				}
				List<VideoDatabase> selectedRefDbs = getSelectedRefDbs();
				this.currentSearch = new ProcessVCD(selectedRefDbs, radio_queryFile.isSelected() ? queryFile : null,
						radio_queryUrl.isSelected() ? queryUrl : null, radio_queryDb.isSelected() ? queryDb : null,
						options);
				this.currentSearch.asyncRun(this);
			}
		}
	}

	private void cancelButton() {
		if (currentStep == 4 && (currentSearch != null && !currentSearch.hasEnded())) {
			if (SwingUtil.showConfirm("A search is running. Do you want to cancel the process?"))
				currentSearch.sendKillProcess();
		} else {
			this.dispose();
		}
	}

	private void closeWindow() {
		ProcessExternalVlcViewer.closeExternalWindows();
		InternalVlcViewerDialog.closeWindow();
	}

	private StringBuffer sbOutput = new StringBuffer();

	@Override
	public synchronized void appendOutputLine(String line) {
		sbOutput.append(line).append("\n");
		textConsole.setText(this.sbOutput.toString());
		textConsole.setCaretPosition(textConsole.getDocument().getLength());
	}

	@Override
	public synchronized void setPctProgress(String stepName, double pct) {
		int num = (int) Math.round(100 * pct);
		progressBar.setValue(num);
		lblProgress.setText(stepName);
	}

	@Override
	public void callbackOnEnd(ProcessBase process, boolean wasSuccessful) {
		if (!wasSuccessful || process != this.currentSearch)
			return;
		this.resultDetections = ((ProcessVCD) process).getDetections();
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				drawDetectionResult();
			}
		});
	}

	private void drawDetectionResult() {
		panelResults.removeAll();
		if (this.resultDetections == null || this.resultDetections.size() == 0) {
			JLabel ll1 = new JLabel("Search did not produce any result!");
			ll1.setFont(new Font("Tahoma", Font.PLAIN, 14));
			JLabel ll2 = new JLabel("Press the 'Previous' button and try with other parameters.");
			ll2.setFont(new Font("Tahoma", Font.PLAIN, 14));
			panelResults.add(ll1, "cell 0 0 3 1");
			panelResults.add(ll2, "cell 0 1 3 1");
		} else {
			JLabel lblQ = new JLabel("Query Video");
			lblQ.setFont(new Font("Tahoma", Font.BOLD, 13));
			JLabel lblR = new JLabel("Original Video");
			lblR.setFont(new Font("Tahoma", Font.BOLD, 13));
			panelResults.add(lblQ, "cell 1 0");
			panelResults.add(lblR, "cell 2 0");
			for (int i = 0; i < this.resultDetections.size(); ++i) {
				VideoCopy vc = this.resultDetections.get(i);
				final int rowPos = i + 1;
				JButton butt = new JButton("<html><body style='width:100%;text-align:center'>Watch Video Copy<br>("
						+ vc.getLengthTxt() + " length)");
				// butt.setHorizontalAlignment(SwingConstants.CENTER);
				butt.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						launchCopy("" + rowPos);
					}
				});
				JLabel timQ = new JLabel("<html><body style='width:100%;text-align:center'>"
						+ vc.getVideoQ().getFilename() + "<br><i>" + vc.getFromQtxt() + " - " + vc.getToQtxt());
				JLabel timR = new JLabel("<html><body style='width:100%;text-align:center'>"
						+ vc.getVideoR().getFilename() + "<br><i>" + vc.getFromRtxt() + " - " + vc.getToRtxt());
				panelResults.add(butt, "cell 0 " + rowPos);
				panelResults.add(timQ, "cell 1 " + rowPos);
				panelResults.add(timR, "cell 2 " + rowPos);
			}
		}
		panelResults.validate();
		currentStep = 5;
		CardLayout cl = (CardLayout) contentPanel.getLayout();
		cl.show(contentPanel, "card_step" + currentStep);
	}

	public void launchCopy(String id) {
		int num = Integer.parseInt(id) - 1;
		if (this.resultDetections == null || this.resultDetections.size() <= num)
			return;
		VideoCopy vc = this.resultDetections.get(num);
		InternalVlcViewerDialog.playVideoCopy(vc);
	}

}
