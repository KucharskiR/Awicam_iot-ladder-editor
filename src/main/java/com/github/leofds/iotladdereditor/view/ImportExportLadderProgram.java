package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.view.event.SerialCommunication;

import java.awt.Color;

public class ImportExportLadderProgram extends JFrame {

	private static final long serialVersionUID = 1L;

	private static ArrayList<String> comPortsSaved = new ArrayList<String>();
	private String exportStatus = "";
	private String importStatus = "";
	private String setStatus = "";
	
	private static Color green = new Color(0, 128, 0);
	private static Color red = new Color(128, 0, 0);
	private static Color black = new Color(0, 0, 0);

	public ImportExportLadderProgram() {
		setAlwaysOnTop(true);
		setTitle("Import/Export ladder program");
		setSize(510, 230);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);

		JPanel panel_0 = new JPanel();
		panel_0.setBorder(new EmptyBorder(10, 10, 10, 10));
		getContentPane().add(panel_0, BorderLayout.NORTH);
		panel_0.setLayout(new BorderLayout(0, 0));

		JLabel lblDescription = new JLabel("1. Export (upload) current ladder program to the hardware device \r\n");
		lblDescription.setFont(new Font("Tahoma", Font.PLAIN, 12));
		panel_0.add(lblDescription, BorderLayout.NORTH);

		JLabel lblDescriptionDown = new JLabel(
				"2. Import (download) ladder program saved in hardware memory if available");
		lblDescriptionDown.setFont(new Font("Tahoma", Font.PLAIN, 12));
		panel_0.add(lblDescriptionDown, BorderLayout.SOUTH);

		JPanel panel_2 = new JPanel();
		getContentPane().add(panel_2, BorderLayout.SOUTH);
		panel_2.setLayout(new BorderLayout(0, 0));
		
		JPanel panelSet = new JPanel();
		panelSet.setBorder(new EmptyBorder(10, 10, 10, 10));
		getContentPane().add(panelSet, BorderLayout.WEST);

		JLabel lblSetCom = new JLabel("Set COM Port:");
		lblSetCom.setFont(new Font("Tahoma", Font.PLAIN, 14));
		panelSet.add(lblSetCom);

		JComboBox<String> comboBoxCOM = new JComboBox<>();
		populateComPortComboBox(comboBoxCOM);
		comboBoxCOM.setPreferredSize(new Dimension(60, 21));
		comboBoxCOM.setMinimumSize(new Dimension(60, 21));
		comboBoxCOM.setFont(new Font("Tahoma", Font.PLAIN, 10));

		panelSet.add(comboBoxCOM);

		JButton btnCheck = new JButton("Check");
		btnCheck.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				populateComPortComboBox(comboBoxCOM);
	
			}
		});
		btnCheck.setFont(new Font("Tahoma", Font.PLAIN, 14));
		panelSet.add(btnCheck);
		
		JPanel panel_3 = new JPanel();
		panel_3.setBorder(new EmptyBorder(10, 10, 10, 10));
		panel_2.add(panel_3);
		panel_3.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_up = new JPanel();
		FlowLayout flowLayout_1 = (FlowLayout) panel_up.getLayout();
		flowLayout_1.setAlignment(FlowLayout.LEFT);
		panel_3.add(panel_up, BorderLayout.NORTH);
				
				JPanel panel_up_divide = new JPanel();
				panel_up.add(panel_up_divide);
				panel_up_divide.setLayout(new BorderLayout(0, 0));
				
						JButton btnExport = new JButton("Export");
						panel_up_divide.add(btnExport, BorderLayout.SOUTH);
						btnExport.setHorizontalTextPosition(SwingConstants.LEADING);
						btnExport.setFont(new Font("Tahoma", Font.PLAIN, 14));
						
						JLabel lblExport = new JLabel("Export status:");
						lblExport.setFont(new Font("Tahoma", Font.PLAIN, 14));
						panel_up.add(lblExport);
						
						JLabel lblExportStatus = new JLabel("");
						lblExportStatus.setFont(new Font("Tahoma", Font.PLAIN, 11));
						lblExportStatus.setText(exportStatus);
						panel_up.add(lblExportStatus);
						
		
		JPanel panel_down = new JPanel();
		FlowLayout flowLayout = (FlowLayout) panel_down.getLayout();
		flowLayout.setAlignment(FlowLayout.LEFT);
		panel_3.add(panel_down, BorderLayout.SOUTH);
		
				JButton btnImport = new JButton("Import");
				btnImport.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent arg0) {
						String selectedPort = (String) comboBoxCOM.getSelectedItem();
						
						SerialCommunication serial = new SerialCommunication(selectedPort, 57600);
						serial.receive();
					}
				});
				panel_down.add(btnImport);
				btnImport.setHorizontalTextPosition(SwingConstants.LEADING);
				btnImport.setFont(new Font("Tahoma", Font.PLAIN, 14));
				
				JLabel lblImport = new JLabel("Import status:");
				lblImport.setFont(new Font("Tahoma", Font.PLAIN, 14));
				panel_down.add(lblImport);
				
				JLabel lblImportStatus = new JLabel("");
				lblImportStatus.setFont(new Font("Tahoma", Font.PLAIN, 11));
				lblImportStatus.setText(importStatus);
				panel_down.add(lblImportStatus);
				
				JSeparator separator_1 = new JSeparator();
				panel_2.add(separator_1, BorderLayout.NORTH);
		
		JLabel lblStatus = new JLabel("");
		lblStatus.setFont(new Font("Tahoma", Font.PLAIN, 14));
		lblStatus.setText(setStatus);
		panelSet.add(lblStatus);

		setVisible(true);
	}

	private void populateComPortComboBox(JComboBox<String> inputComboBox) {

		SerialPort[] ports = SerialPort.getCommPorts();
		for (SerialPort port : ports) {
			String portName = port.getSystemPortName();

			if (!comPortsSaved.contains(portName)) {
				inputComboBox.addItem(portName);
				comPortsSaved.add(portName);
			}
		}
	}

}
