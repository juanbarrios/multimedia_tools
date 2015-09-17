/*
 * Copyright (C) 2012-2015, Juan Manuel Barrios <http://juan.cl/>
 * All rights reserved.
 *
 * This file is part of P-VCD. http://p-vcd.org/
 * P-VCD is made available under the terms of the BSD 2-Clause License.
 */
package org.p_vcd.ui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.WindowConstants;

import org.p_vcd.model.Parameters;
import org.p_vcd.model.VideoCopy;
import org.p_vcd.process.ProcessExternalVlcViewer;

import com.sun.jna.Native;
import com.sun.jna.NativeLibrary;

import net.miginfocom.swing.MigLayout;
import uk.co.caprica.vlcj.binding.LibVlc;
import uk.co.caprica.vlcj.component.EmbeddedMediaPlayerComponent;
import uk.co.caprica.vlcj.runtime.RuntimeUtil;

public class InternalVlcViewerDialog extends JDialog {
	private static final long serialVersionUID = 1L;

	private static boolean isInit = false;
	private static EmbeddedMediaPlayerComponent player1;
	private static EmbeddedMediaPlayerComponent player2;
	private static InternalVlcViewerDialog dialog;
	private static VideoCopy lastVC;

	public static synchronized void initLibVlc() {
		if (isInit)
			return;
		File libVlc = new File(Parameters.get().getLibVlcPath());
		NativeLibrary.addSearchPath(RuntimeUtil.getLibVlcLibraryName(), libVlc.getParentFile().getAbsolutePath());
		Native.loadLibrary(RuntimeUtil.getLibVlcLibraryName(), LibVlc.class);
		player1 = new EmbeddedMediaPlayerComponent();
		player2 = new EmbeddedMediaPlayerComponent();
		player1.setPreferredSize(new Dimension(400, 300));
		player2.setPreferredSize(new Dimension(400, 300));
		player1.setMaximumSize(new Dimension(800, 600));
		player2.setMaximumSize(new Dimension(800, 600));
		player1.setMinimumSize(new Dimension(200, 150));
		player2.setMinimumSize(new Dimension(200, 150));
		isInit = true;
	}

	public static synchronized void playVideoCopy(VideoCopy vc) {
		initLibVlc();
		if (dialog == null) {
			dialog = new InternalVlcViewerDialog();
			dialog.setModalityType(ModalityType.MODELESS);
			dialog.setModalExclusionType(ModalExclusionType.APPLICATION_EXCLUDE);
		}
		if (!dialog.isVisible()) {
			dialog.setVisible(true);
		}
		lastVC = vc;
		startPlayback();
	}

	private static void startPlayback() {
		try {
			VideoCopy vc = lastVC;
			playVideo(player1, vc.getVideoQ().getFilePath(), vc.getFromQ(), vc.getToQ());
			playVideo(player2, vc.getVideoR().getFilePath(), vc.getFromR(), vc.getToR());
			dialog.name1.setText(vc.getVideoQ().getFilename());
			dialog.name2.setText(vc.getVideoR().getFilename());
			dialog.time1.setText("<html><i>" + vc.getFromQtxt() + " - " + vc.getToQtxt() + " ");
			dialog.time2.setText("<html><i>" + vc.getFromRtxt() + " - " + vc.getToRtxt() + " ");
		} catch (Throwable tr) {
			tr.printStackTrace();
		}
	}

	private static void stopPlayback() {
		try {
			if (player1 != null)
				player1.getMediaPlayer().stop();
			if (player2 != null)
				player2.getMediaPlayer().stop();
		} catch (Throwable tr) {
			tr.printStackTrace();
		}
	}

	public static synchronized void closeWindow() {
		if (dialog == null)
			return;
		dialog.dispose();
		if (player1 != null)
			player1.release();
		if (player2 != null)
			player2.release();
		player1 = player2 = null;
		dialog = null;
	}

	private static void playVideo(EmbeddedMediaPlayerComponent player, String filePath, double secondsStart,
			double secondsEnd) {
		double from = Math.round(secondsStart * 10) / 10.0;
		double length = Math.round(secondsEnd * 10) / 10.0;
		String file = new File(filePath).getAbsolutePath();
		player.getMediaPlayer().playMedia(file, ":start-time=" + from, ":stop-time=" + length);
	}

	private JLabel name1 = new JLabel();
	private JLabel time1 = new JLabel();
	private JLabel name2 = new JLabel();
	private JLabel time2 = new JLabel();

	public InternalVlcViewerDialog() {
		setSize(850, 300);
		setTitle("P-VCD - Video Copy Viewer");
		setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
		getContentPane().setLayout(new BorderLayout());
		{
			JPanel p = new JPanel();
			getContentPane().add(p, BorderLayout.CENTER);
			p.setLayout(
					new MigLayout("gapy 1, gapx 10", "[400px,grow,center][400px,grow,center]", "[grow][20px][20px]"));
			if (player1 != null)
				p.add(player1, "cell 0 0");
			if (player2 != null)
				p.add(player2, "cell 1 0");
			p.add(name1, "cell 0 1");
			p.add(name2, "cell 1 1");
			p.add(time1, "cell 0 2");
			p.add(time2, "cell 1 2");
		}
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.CENTER));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			JButton btnReplay = new JButton("Play Again");
			buttonPane.add(btnReplay);
			btnReplay.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					startPlayback();
				}
			});

			JButton btnViewer = new JButton("Open External Viewer...");
			buttonPane.add(btnViewer);
			btnViewer.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					openExternalButton();
				}
			});

			JButton closeButton = new JButton("Close");
			buttonPane.add(closeButton);
			getRootPane().setDefaultButton(closeButton);
			closeButton.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					closeButton();
				}
			});
		}
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				stopPlayback();
			}
		});
	}

	private void openExternalButton() {
		stopPlayback();
		ProcessExternalVlcViewer.playVideoCopy(lastVC);
	}

	private void closeButton() {
		stopPlayback();
		this.setVisible(false);
	}
}
