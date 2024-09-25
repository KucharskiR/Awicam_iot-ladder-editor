package com.github.leofds.iotladdereditor.view;

import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.LayoutManager;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.util.SerialCommunication;

public class ConnectionPanel extends JPanel {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private JPanel bottomPanel;
	private JComboBox<String> comPortComboBox;
	private JLabel lblLightLabel;
	private JLabel lblDeviceNameLabel;
	private JButton btnConnect;
	
	public JPanel getPanel() {
		return this.bottomPanel;
	}

	public ConnectionPanel() {
		JPanel bottomPanel = new JPanel();
		bottomPanel.setBorder(new EmptyBorder(0, 30, 0, 30));
		bottomPanel.setPreferredSize(new Dimension(454, 40));
		bottomPanel.setMinimumSize(new Dimension(10, 45));
		bottomPanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 10, 7));
		
		this.bottomPanel = bottomPanel;
		
		lblDeviceNameLabel = new JLabel("Connect to the device on port");
		lblDeviceNameLabel.setFont(new Font("Tahoma", Font.PLAIN, 12));
		bottomPanel.add(lblDeviceNameLabel);
		
		comPortComboBox = new JComboBox<>();
		comPortComboBox.setPreferredSize(new Dimension(220, 24));
		comPortComboBox.setBorder(new EmptyBorder(4, 4, 4, 4));
		comPortComboBox.setMaximumSize(new Dimension(220, 24));
		bottomPanel.add(comPortComboBox);
		
		populateComPortComboBox();
		
		btnConnect = new JButton(Strings.connect());
		btnConnect.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				btnConnectAction();
			}
		});
		
		btnConnect.setPreferredSize(new Dimension(90, 21));
		btnConnect.setFont(new Font("Tahoma", Font.PLAIN, 14));
		btnConnect.setMaximumSize(new Dimension(70, 24));
		btnConnect.setMinimumSize(new Dimension(70, 24));
		btnConnect.setBorder(new EmptyBorder(4, 4, 4, 4));
		bottomPanel.add(btnConnect);
		
		lblLightLabel = new JLabel("");
		lblLightLabel.setIcon(new ImageIcon(ContentPanel.class.getResource("/images/indicator_red_light_18x18.png")));
		bottomPanel.add(lblLightLabel);
	}

	public ConnectionPanel(LayoutManager layout) {
		super(layout);
	}

	public ConnectionPanel(boolean isDoubleBuffered) {
		super(isDoubleBuffered);
	}

	public ConnectionPanel(LayoutManager layout, boolean isDoubleBuffered) {
		super(layout, isDoubleBuffered);
	}
	
	private void btnConnectAction() {
		
		// Get global connection 
		SerialCommunication connect = Mediator.getInstance().getConnection();
		
		// Get whole port name
		String portName = comPortComboBox.getSelectedItem().toString();
		
		// Get only COM<number> from whole COM port name
		String[] parts = portName.split(":");
		portName = parts[0];
		
		if (btnConnect.getText() == Strings.connect()) {
			
			// Set Connecting... text
			btnConnect.setText(Strings.connecting());
			
			if (portName != null) {
				if(connect.start(portName, 9600) == 0) {
					
					// Set green label
					lblLightLabel.setIcon(new ImageIcon(ContentPanel.class.getResource("/images/indicator_green_light_18x18.png")));
					
					// Set Disconnect text on button
					btnConnect.setText(Strings.disconnect());
					
					// Get controller name
					String controllerName = connect.controllerInfo();
					lblDeviceNameLabel.setText(Strings.connectedToTheDevice() + " " + controllerName);
					
				} else {
					// Set red label
					lblLightLabel.setIcon(new ImageIcon(ContentPanel.class.getResource("/images/indicator_red_light_18x18.png")));
					lblDeviceNameLabel.setText(Strings.connectToOnPort());
					
					// Set Connect text on button
					btnConnect.setText(Strings.connect());
				}
			}
			
		} else if (btnConnect.getText() == Strings.disconnect()) {
			lblLightLabel.setIcon(new ImageIcon(ContentPanel.class.getResource("/images/indicator_red_light_18x18.png")));
			btnConnect.setText(Strings.connect());
			try {
				connect.closeCOM();
				
				// Clear combo list and populate all ports
				comPortComboBox.removeAllItems();
				populateComPortComboBox();
				
				// Output Disconnected to console
				consoleOutput(Strings.disconnected());
				lblDeviceNameLabel.setText(Strings.connectToOnPort());
			} catch (IOException e) {
				e.printStackTrace();
				
				// Output Disconnection error to console
				consoleOutput(Strings.disconnectionError() + e.getMessage());
			}
		}
		
	}
	
	private void populateComPortComboBox() {
		
		SerialPort[] ports = SerialPort.getCommPorts();
		for (SerialPort port : ports) {
			String portName = port.getSystemPortName();
			String portExtend = port.getPortDescription();
			comPortComboBox.addItem(portName + ": " + portExtend);
		}
	}
	
	private void consoleOutput(String msg) {
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
