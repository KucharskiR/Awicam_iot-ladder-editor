package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.Compiler;
import com.github.leofds.iotladdereditor.i18n.Strings;


public class ESPUpload extends JFrame {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JComboBox<String> comPortComboBox;
	private boolean uploadingStart;
	private String portName;
	private Compiler compilation;

    public boolean isUploadingStart() {
		return uploadingStart;
	}

	public String getPortName() {
		return portName;
	}

	public ESPUpload() {
		setAlwaysOnTop(true);
		this.uploadingStart = false;
		this.portName = null;
		this.compilation = new Compiler();
		
		Color veryDarkGreen = new Color(0, 153 ,0);

		setTitle("ESP Upload");
		setSize(350, 191);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);

		comPortComboBox = new JComboBox<>();
		comPortComboBox.setEnabled(false);
		comPortComboBox.setFont(new Font("Tahoma", Font.PLAIN, 11));
		populateComPortComboBox();
		JButton connectButton = new JButton(Strings.upload());
		connectButton.setEnabled(false);
		connectButton.setFont(new Font("Tahoma", Font.PLAIN, 11));
		
		// sharing thread data
		SharedResource sharedResource = new SharedResource();
		

//		
//		Thread uploadingWaitingBar = new Thread(() -> {
//			UploadingWaitingBar uploadWaitingBar = new UploadingWaitingBar();
//			
//			while (!sharedResource.getData())
//				;
//			uploadWaitingBar.close();
//			
//		});


		JPanel panel = new JPanel();
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		JLabel label = new JLabel(Strings.selectComPort());
		label.setFont(new Font("Tahoma", Font.PLAIN, 11));
		panel.add(label);
		panel.add(comPortComboBox);
		panel.add(connectButton);

		getContentPane().add(panel);
		
		JPanel panel_1 = new JPanel();
		panel_1.setBorder(new EmptyBorder(5, 0, 5, 0));
		getContentPane().add(panel_1, BorderLayout.NORTH);
		panel_1.setLayout(new BorderLayout(0, 0));
		
		JButton btnCompile = new JButton(Strings.runCompilation());
		btnCompile.setHorizontalTextPosition(SwingConstants.LEADING);
		btnCompile.setPreferredSize(new Dimension(110, 40));
		btnCompile.setFont(new Font("Tahoma", Font.PLAIN, 14));
		panel_1.add(btnCompile);
		
		JPanel panel_2 = new JPanel();
		getContentPane().add(panel_2, BorderLayout.SOUTH);
		panel_2.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_3 = new JPanel();
		panel_2.add(panel_3, BorderLayout.SOUTH);
		
		JLabel lblStatusUpload = new JLabel(Strings.uploadingStatus());
		lblStatusUpload.setHorizontalAlignment(SwingConstants.LEFT);
		lblStatusUpload.setFont(new Font("Tahoma", Font.PLAIN, 11));
		panel_3.add(lblStatusUpload);
		
		JLabel lblOutputUpload = new JLabel(Strings.notUploaded());
		lblOutputUpload.setHorizontalAlignment(SwingConstants.LEFT);
		lblOutputUpload.setFont(new Font("Tahoma", Font.ITALIC, 11));
		lblOutputUpload.setBorder(new EmptyBorder(0, 10, 0, 15));
		panel_3.add(lblOutputUpload);
		
		JPanel panel_4 = new JPanel();
		panel_2.add(panel_4, BorderLayout.NORTH);
		
		JLabel lblStatus = new JLabel(Strings.compilationStatus());
		panel_4.add(lblStatus);
		lblStatus.setHorizontalAlignment(SwingConstants.LEFT);
		lblStatus.setFont(new Font("Tahoma", Font.PLAIN, 11));
		
		JLabel lblOutputCompile = new JLabel(Strings.notCompiled());
		panel_4.add(lblOutputCompile);
		lblOutputCompile.setBorder(new EmptyBorder(0, 10, 0, 15));
		lblOutputCompile.setHorizontalAlignment(SwingConstants.LEFT);
		lblOutputCompile.setFont(new Font("Tahoma", Font.ITALIC, 11));
		
		
		// upload action listener
		connectButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				String selectedPort = (String) comPortComboBox.getSelectedItem();
				if (selectedPort != null) {
					
					// uploading thread
					Thread uploadingTerminalThread = new Thread(() -> {
						sharedResource.setData(false);

						Compiler uploadingCompiler = new Compiler();

						lblSet(lblOutputUpload, Strings.uploadingDots(), "Tahoma", Font.ITALIC, 11, Color.BLACK);
						
						uploadingCompiler.upload(portName);
						
						if (uploadingCompiler.getUploadingStatus() == 0) {
							lblSet(lblOutputUpload, Strings.uploadingCompleted(), "Tahoma", Font.BOLD, 11, veryDarkGreen);

						} else {
							consoleOutput(Strings.uploadingError());
							lblSet(lblOutputUpload, Strings.uploadingError(), "Tahoma", Font.BOLD, 11, Color.RED);
						}

						sharedResource.setData(true);

					});
					
					
					// Perform actions to connect using the selected COM port
					// For example: Open and configure the serial port
//                	Compiler uploadCompiler = new Compiler();
//                	uploadCompiler.upload(selectedPort);
					uploadingStart = true;
					portName = selectedPort;

					System.out.println("Connecting to " + selectedPort);

					uploadingTerminalThread.start();
//					uploadingWaitingBar.start();
				}
			}
		});
		
		// compile action listener
		btnCompile.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				compilation.setCompilationStatus(5);
				// compile Thread2
				Thread compileThread = new Thread(() -> {
					// Operation 2 code here
					sharedResource.setData(false);

					lblSet(lblOutputCompile, Strings.compilationDots(), "Tahoma", Font.ITALIC, 11, Color.BLACK);
					lblSet(lblOutputUpload, Strings.notUploaded(),"Tahoma", Font.ITALIC, 11, Color.BLACK);
					compilation.compile();

					if (compilation.getCompilationStatus() == 0) {
						lblSet(lblOutputCompile, Strings.compilationCompleted(), "Tahoma", Font.BOLD, 11, veryDarkGreen);
						connectButton.setEnabled(true);
						comPortComboBox.setEnabled(true);
					} else {
						lblSet(lblOutputCompile, Strings.compilationError(), "Tahoma", Font.BOLD, 11, Color.RED);
					}
					sharedResource.setData(true);
					sharedResource.setCompilationStatus(compilation.getCompilationStatus());
				});
				compileThread.start();
			}
		});
	}
	
	private void lblSet(JLabel label, String text, String font, int fontType, int fontSize,
			Color fontColor) {
		
		label.setText(text);
		label.setFont(new Font(font, fontType, fontSize));
		label.setForeground(fontColor);
		
	}
	
	private void populateComPortComboBox() {
		
		SerialPort[] ports = SerialPort.getCommPorts();
		for (SerialPort port : ports) {
			String portName = port.getSystemPortName();
			comPortComboBox.addItem(portName);
		}
	}
	
	private void consoleOutput(String msg) {
		// output console 
		Mediator.getInstance().outputConsoleMessage(msg);
	}
	
	class SharedResource {
	    private boolean isFinished;
	    private int compilationStatus; // 0 - success, 1 - error

	    public int getCompilationStatus() {
			return compilationStatus;
		}

		public void setCompilationStatus(int compilationStatus) {
			this.compilationStatus = compilationStatus;
		}

		synchronized void setData(boolean isFinished) {
	        this.isFinished = isFinished;
	    }

	    synchronized boolean getData() {
	        return isFinished;
	    }
	}
}
