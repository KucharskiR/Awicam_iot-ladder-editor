package com.github.leofds.iotladdereditor.view.event;

import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.Compiler;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.util.bars.UploadingWaitingBar;


public class ComPortChooser extends JFrame {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JComboBox<String> comPortComboBox;
	private boolean uploadingStart;
	private String portName;

    public boolean isUploadingStart() {
		return uploadingStart;
	}

	public String getPortName() {
		return portName;
	}

	public ComPortChooser() {
		this.uploadingStart = false;
		this.portName = null;

		setTitle("ESP Upload");
		setSize(350, 100);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);

		comPortComboBox = new JComboBox<>();
		comPortComboBox.setFont(new Font("Tahoma", Font.PLAIN, 11));
		populateComPortComboBox();
		JButton connectButton = new JButton(Strings.upload());
		connectButton.setFont(new Font("Tahoma", Font.PLAIN, 13));

		// sharing thread data
		SharedResource sharedResource = new SharedResource();
		
		Thread uploadingTerminalThread = new Thread(() -> {
			sharedResource.setData(false);
			
			Compiler uploadingCompiler = new Compiler();
			
			uploadingCompiler.upload(portName);
			
			if (uploadingCompiler.getUploadingStatus() == 1) {
				consoleOutput("Uploading error");
			}
			
			sharedResource.setData(true);
			
		});
		
		Thread uploadingWaitingBar = new Thread(() -> {
			UploadingWaitingBar uploadWaitingBar = new UploadingWaitingBar();
			
			while (!sharedResource.getData())
				;
			uploadWaitingBar.close();
			
		});

		connectButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				String selectedPort = (String) comPortComboBox.getSelectedItem();
				if (selectedPort != null) {
					// Perform actions to connect using the selected COM port
					// For example: Open and configure the serial port
//                	Compiler uploadCompiler = new Compiler();
//                	uploadCompiler.upload(selectedPort);
					uploadingStart = true;
					portName = selectedPort;

					System.out.println("Connecting to " + selectedPort);

					uploadingTerminalThread.start();
					uploadingWaitingBar.start();
				}
			}
		});

		JPanel panel = new JPanel();
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		JLabel label = new JLabel("Select COM Port:");
		label.setFont(new Font("Tahoma", Font.PLAIN, 13));
		panel.add(label);
		panel.add(comPortComboBox);
		panel.add(connectButton);

		getContentPane().add(panel);
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
