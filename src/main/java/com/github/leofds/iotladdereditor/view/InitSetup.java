/*******************************************************************************
 * Copyright (C) 2021 Leonardo Fernandes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Point;
import java.awt.SystemColor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.JTextPane;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;
import javax.swing.table.AbstractTableModel;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.domain.CodeOptionsDevice;
import com.github.leofds.iotladdereditor.compiler.domain.CodeOptionsDevice2;
import com.github.leofds.iotladdereditor.device.Device;
import com.github.leofds.iotladdereditor.device.DeviceFactory;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.ladder.LadderProgram;
import com.github.leofds.iotladdereditor.ladder.ProgramProperties;
import com.github.leofds.iotladdereditor.util.FileUtils;

/**
 * @author kucha
 *
 */
/**
 * @author kucha
 *
 */
public class InitSetup extends JDialog {

	private static final long serialVersionUID = 1L;
	
	private final JPanel contentPanel = new JPanel();
//	private JTextField textFieldSsid;
//	private JTextField textFieldPassword;
//	private JTextField textFieldCAFile;
//	private JTextField textFieldClientCertFile;
//	private JTextField textFieldClientPkFile;
//	private JTextField textFieldBrokerAddress;
//	private JTextField textFieldPubTopic;
//	private JTextField textFieldSubTopic;
//	private JTextField textFieldBokerPort;
//	private JTextField textFieldClientID;
//	private JButton btnChooseCaFile;
//	private JButton btnChooseClientCert;
//	private JButton btnChooseClientPk;
//	private JCheckBox checkBoxUseClientCertificate;
//	private JCheckBox checkBoxEnableSsl;
//	private JTextField textFieldUserUsername;
//	private JTextField textFieldUserPassword;
//	private JTextField textFieldTelemetrySeconds;
//	private JCheckBox checkBoxEnableTelemetry;
//	private JCheckBox checkBoxTelemetryMemory;
//	private JCheckBox checkBoxTelemetryOutput;
//	private JCheckBox checkBoxTelemetryInput;
	private LadderProgram ladderProgram;
	private JComboBox<CodeOptionsDevice> comboBox_code;
	private JTable tablePinMapping;
	private Device device;
	private List<String> devices;

	private JComboBox<CodeOptionsDevice2> comboBox_device2;
	
	

	/**
	 * Create the dialog.
	 */
	public InitSetup() {

		ladderProgram = Mediator.getInstance().getProject().getLadderProgram();
		device = ladderProgram.getDevice().clone();
//		devices = new ArrayList<>();
		devices = ladderProgram.getProperties().getDevices();

		
		setBounds(100, 100, 820, 508);
		getContentPane().setLayout(new BorderLayout());
		Border border = BorderFactory.createTitledBorder("Init setup");
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		contentPanel.setBorder(border);
		contentPanel.setPreferredSize(new Dimension(800, 460));
		getContentPane().add(contentPanel, BorderLayout.NORTH);
		contentPanel.setLayout(null);


		JButton btnSave = new JButton(Strings.save());
		btnSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				save();
			}
		});
		btnSave.setBounds(571, 425, 110, 25);
		contentPanel.add(btnSave);

		JButton btnCancel = new JButton(Strings.cancel());
		btnCancel.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				InitSetup.this.dispose();
			}
		});
		btnCancel.setBounds(693, 425, 97, 25);
		contentPanel.add(btnCancel);
		
		/*
		 * 
		 * Tab panel
		 */

		JTabbedPane tabbedPane = new JTabbedPane(JTabbedPane.TOP);
		tabbedPane.setBounds(12, 132, 778, 285);
		contentPanel.add(tabbedPane);

		
		/*
		 * 
		 * Device 2 List Tab
		 *  
		 * 
		 */
		
		JPanel panel_8 = new JPanel();
		tabbedPane.addTab(Strings.device2Table(), null, panel_8, null);
		panel_8.setLayout(null);

		tablePinMapping = new JTable();
		tablePinMapping.setCellSelectionEnabled(true);
		tablePinMapping.setBounds(649, 146, 1, 1);
//		tablePinMapping.setModel(new Device2Model(device));	//FIXME comment WindowBuilder
		Device2Model tableModel = new Device2Model(devices);
		tablePinMapping.setModel(tableModel);
		tablePinMapping.addMouseListener(new TableMouseListener(tablePinMapping));
		
		/*
		 * 
		 *  Delete, SettingsBB  menu 
		 * 
		 */
		
		JPopupMenu popMenuDevice2Table = new JPopupMenu();
		
		JMenuItem propertiesBB = new JMenuItem(Strings.propertiesBB());
		propertiesBB.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
//				deletePin();
				SettingsBB dialogSettings = new SettingsBB();
				dialogSettings.setModal(true);
				dialogSettings.setResizable(false);
				dialogSettings.pack();
				dialogSettings.setLocationRelativeTo(null);
				dialogSettings.setVisible(true);
				tableModel.fireTableDataChanged();
			}
		});
		
		JMenuItem deleteItemDevice2 = new JMenuItem(Strings.delete());
		deleteItemDevice2.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				deletePin();
				tableModel.fireTableDataChanged();
			}
		});


		popMenuDevice2Table.add(propertiesBB);
		popMenuDevice2Table.add(deleteItemDevice2);
//		popMenuDevice2Table.add(addInputItemDevice2);

		tablePinMapping.setComponentPopupMenu(popMenuDevice2Table);

		JScrollPane scrollPaneDevice2 = new JScrollPane(tablePinMapping);
		scrollPaneDevice2.setBounds(10, 11, 241, 223);
		panel_8.add(scrollPaneDevice2);
		
		JTextPane txtpnTekst = new JTextPane();
		txtpnTekst.setText("Init setup:\r\n- Main Device - select main device\r\n- Additional Device - select add addictional devices (max. 32)\r\n\r\nAdditional devices:\r\n- right mouse clik - delete device or setup properties\r\n");
		txtpnTekst.setBackground(SystemColor.menu);
		txtpnTekst.setBounds(285, 11, 478, 223);
		panel_8.add(txtpnTekst);
		
		/*
		 * 
		 * 
		 *  Device 1  ComboBox
		 * 
		 * 
		 */


		JPanel panel = new JPanel();
		panel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), Strings.mainDevice(), TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		panel.setBounds(10, 15, 369, 115);
		contentPanel.add(panel);
		panel.setLayout(null);

		comboBox_code = new JComboBox<CodeOptionsDevice>();
		comboBox_code.setBounds(72, 40, 229, 21);
		panel.add(comboBox_code);
		comboBox_code.setModel(new DefaultComboBoxModel<CodeOptionsDevice>(CodeOptionsDevice.values()));
		comboBox_code.setSelectedItem(CodeOptionsDevice.getByName(device.getName()));
		
		/*
		 * 
		 * Device 2  ComboBox
		 * 
		 */
		comboBox_code.setFont(new Font("Tahoma", Font.PLAIN, 12));

		JPanel panelDevice2 = new JPanel();
		panelDevice2.setBorder(new TitledBorder(null, Strings.device2(), TitledBorder.LEADING, TitledBorder.TOP, null, null));
		panelDevice2.setBounds(391, 15, 399, 115);
		contentPanel.add(panelDevice2);
		panelDevice2.setLayout(null);
		
		comboBox_device2 = new JComboBox<CodeOptionsDevice2>();
		comboBox_device2.setBounds(72, 40, 229, 21);
		panelDevice2.add(comboBox_device2);
		comboBox_device2.setModel(new DefaultComboBoxModel<CodeOptionsDevice2>(CodeOptionsDevice2.values()));
		comboBox_device2.setFont(new Font("Tahoma", Font.PLAIN, 12));
		
		/*
		 *   BUTTON EXAMPLE
		 * 
		JButton btnGenerateClientID = new JButton( Strings.generate() );
		btnGenerateClientID.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				textFieldClientID.setText(RandomStringUtils.randomAlphabetic(20));
			}
		});
		btnGenerateClientID.setBounds(639, 74, 92, 21);
		panel_2.add(btnGenerateClientID);
		 * 
		 * 
		 * 
		 * 
		 */
		
		/*
		 * 
		 * 
		 * 
		 */
		
		JButton btnAddDevice = new JButton( Strings.addDevice() );
		btnAddDevice.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
				// TODO Auto-generated method stub
				try {
					if(devices.size()>=32) return;
					devices.add(comboBox_device2.getSelectedItem().toString());
					System.out.println(devices.size());
					tableModel.fireTableDataChanged();
				} catch (Exception e2) {
					// TODO: handle exception
					System.out.println("Null pointer exception");
				}
			}
		});
		btnAddDevice.setBounds(72, 70, 229, 21);
		panelDevice2.add(btnAddDevice);
		
//
//		JLabel lblSsid = new JLabel(Strings.ssid());
//		lblSsid.setBounds(12, 31, 131, 16);
//		panel_1.add(lblSsid);
//		lblSsid.setHorizontalAlignment(SwingConstants.RIGHT);
//
//		textFieldSsid = new JTextField();
//		textFieldSsid.setBounds(155, 28, 162, 22);
//		panel_1.add(textFieldSsid);
//		textFieldSsid.setColumns(10);
//
//		JLabel lblPassword = new JLabel(Strings.password());
//		lblPassword.setBounds(12, 56, 131, 16);
//		panel_1.add(lblPassword);
//		lblPassword.setHorizontalAlignment(SwingConstants.RIGHT);
//
//		textFieldPassword = new JTextField();
//		textFieldPassword.setBounds(155, 55, 162, 22);
//		panel_1.add(textFieldPassword);
//		textFieldPassword.setColumns(10);

		loadFields();
	}
	
	/*
	 * 
	 * 
	 *     Methods
	 * 
	 * 
	 * 
	 */
	

	private void loadFields() {
		ProgramProperties properties = ladderProgram.getProperties();
		comboBox_code.setSelectedItem( properties.getCodeOption() );
//		textFieldSsid.setText( properties.getWifiSsid() );
//		textFieldPassword.setText( properties.getWifiPassword() );
//		textFieldBrokerAddress.setText( properties.getBrokerAddress() );
//		textFieldBokerPort.setText( ""+properties.getBrokerPort() );
//		textFieldClientID.setText( properties.getMqttClientID() );
//		textFieldUserUsername.setText( properties.getMqttUsername() );
//		textFieldUserPassword.setText( properties.getMqttPassword() );
//		checkBoxEnableSsl.setSelected( properties.isEnableSsl() );
//		checkBoxUseClientCertificate.setSelected( properties.isUseClientCert() );
//		enableSsl( properties.isEnableSsl() );
//		useClientCert( properties.isUseClientCert() );
//		textFieldCAFile.setText( properties.getMqttCa() );
//		textFieldClientCertFile.setText( properties.getMqttClientCert() );
//		textFieldClientPkFile.setText( properties.getMqttClientPrivateKey() );
//		textFieldPubTopic.setText( properties.getMqttPubTopic() );
//		textFieldSubTopic.setText( properties.getMqttSubTopic() );
//		textFieldTelemetrySeconds.setText( ""+properties.getTelemetrySeconds() );
//		checkBoxEnableTelemetry.setSelected( properties.getEnableTelemetry() );
//		checkBoxTelemetryInput.setSelected( properties.getTelemetryPubInput() );
//		checkBoxTelemetryOutput.setSelected( properties.getTelemetryPutOutput() );
//		checkBoxTelemetryMemory.setSelected( properties.getTelemetryPubMemory() );
//		enableTelemetry( checkBoxEnableTelemetry.isSelected() );
	}
	/*
	 * 
	 * Save
	 * 
	 * 
	 */
	private void save() {
		int dialogResult = JOptionPane.showConfirmDialog(this, Strings.confirmSaveProjectProperties(), Strings.titleSaveProjectProperties(), JOptionPane.YES_NO_OPTION);
		if(dialogResult == 0) {
			
			CodeOptionsDevice codeOpt = (CodeOptionsDevice) comboBox_code.getSelectedItem();
			ProgramProperties properties = ladderProgram.getProperties();
			properties.setCodeOptionDevice(codeOpt);
			properties.setDevices(devices);
			
			device = new DeviceFactory().getDevice();
			
			Mediator me = Mediator.getInstance();
			me.getProject().setChanged(true);
			FileUtils.saveLadderProgram();
			me.clearConsole();
			me.updateDevice(device);
			this.dispose();
		}
	}
	
	
	/*
	private void save() {
		int dialogResult = JOptionPane.showConfirmDialog(this, Strings.confirmSaveProjectProperties(), Strings.titleSaveProjectProperties(), JOptionPane.YES_NO_OPTION);
		if(dialogResult == 0) {
			CodeOptions codeOpt = (CodeOptions) comboBox_code.getSelectedItem();
			ProgramProperties properties = ladderProgram.getProperties();
			properties.setCodeOption(codeOpt);
//			properties.setWifiSsid( textFieldSsid.getText() );
//			properties.setWifiPassword( textFieldPassword.getText() );
			properties.setBrokerAddress( textFieldBrokerAddress.getText() );
			try {
				properties.setBrokerPort( Integer.parseInt( textFieldBokerPort.getText() ) );
			} catch (Exception e) {
				properties.setBrokerPort( 1883 );
			}
//			properties.setMqttClientID( textFieldClientID.getText() );
//			properties.setMqttUsername( textFieldUserUsername.getText() );
//			properties.setMqttPassword( textFieldUserPassword.getText() );
//			properties.setEnableSsl( checkBoxEnableSsl.isSelected() );
//			properties.setUseClientCert( checkBoxUseClientCertificate.isSelected() );
//			properties.setMqttCa( textFieldCAFile.getText() );
//			properties.setMqttClientCert( textFieldClientCertFile.getText() );
//			properties.setMqttClientPrivateKey( textFieldClientPkFile.getText() );
//			properties.setMqttPubTopic( textFieldPubTopic.getText() );
//			properties.setMqttSubTopic( textFieldSubTopic.getText() );
//			properties.setEnableTelemetry( checkBoxEnableTelemetry.isSelected() );
//			properties.setTelemetryPubInput( checkBoxTelemetryInput.isSelected() );
//			properties.setTelemetryPutOutput( checkBoxTelemetryOutput.isSelected() );
//			properties.setTelemetryPubMemory( checkBoxTelemetryMemory.isSelected() );
			String teleSec = textFieldTelemetrySeconds.getText();
			if(teleSec.isEmpty() || teleSec.equals("0")) {
				teleSec = "5";
			}
			properties.setTelemetrySeconds( Integer.parseInt( teleSec ) );

			Mediator me = Mediator.getInstance();
			me.getProject().setChanged(true);
			FileUtils.saveLadderProgram();
			me.clearConsole();
			me.updateDevice(device);
			this.dispose();
		}
	}
	
	*/

	private void deletePin() {
//		int column = 0;
		int row = tablePinMapping.getSelectedRow();
//		String name = (String) tablePinMapping.getModel().getValueAt(row, column);
		devices.remove(row);
		tablePinMapping.repaint();
	}
	
//	private String getAvaliablePinName(IO io) {
//		List<Integer> pinNumbers = new ArrayList<Integer>();
//		for(Peripheral periferal: device.getPeripherals()) {
//			for(PeripheralIO peripheralIO: periferal.getPeripheralItems()) {
//				if(peripheralIO.getIo().equals(io)) {
//					pinNumbers.add( Integer.parseInt(peripheralIO.getName().substring(1) ));
//				}
//			}
//		}
//		Collections.sort(pinNumbers);
//		Integer num;
//		for(num=1; num<=pinNumbers.size(); num++) {
//			if(!pinNumbers.contains(num)) {
//				break;
//			}
//		}
//		switch(io) {
//		case INPUT:
//			return "I"+num;
//		default:
//			return "Q"+num;
//		}
//	}
//	
//	private PeripheralIO createNewPin(IO io) {
//		String pinName = getAvaliablePinName(io);
//		PeripheralIO peripheralIO = null;
//		try {
//			Integer pinNumber = Integer.parseInt( (String) JOptionPane.showInputDialog(null, pinName,Strings.pinNumber(), JOptionPane.INFORMATION_MESSAGE, null, null, null));
//			String pinPath = "";
//			switch(io) {
//			case INPUT:
//				pinPath = "PIN_I"+String.format("%02d", pinNumber);
//				break;
//			default:
//				pinPath = "PIN_Q"+String.format("%02d", pinNumber);	
//			}
//			peripheralIO = new PeripheralIO(pinName, Boolean.class, ""+pinNumber, pinPath, io);
//		}catch (Exception e) {
//			JOptionPane.showMessageDialog(null, Strings.invalidPinNumber());
//		}
//		return peripheralIO;
//	}
//	
//	private void addInput() {
//		PeripheralIO peripheralIO = createNewPin(IO.INPUT);
//		if(peripheralIO != null) {
//			Peripheral peripheral = device.getPeripheralBySymbol("I");
//			if(peripheral != null) {
//				peripheral.addPeripheralItem(peripheralIO);
//				device.sort();
//			}
//		}
//	}
//	
//	private void addOutput() {
//		PeripheralIO peripheralIO = createNewPin(IO.OUTPUT);
//		if(peripheralIO != null) {
//			Peripheral peripheral = device.getPeripheralBySymbol("Q");
//			if(peripheral != null) {
//				peripheral.addPeripheralItem(peripheralIO);
//				device.sort();
//			}
//		}
//	}
	
//	private void enableSsl(boolean enable) {
//		if(enable) {
//			checkBoxUseClientCertificate.setEnabled(true);
//			btnChooseCaFile.setEnabled(true);
//			textFieldCAFile.setEnabled(true);
//		}else{
//			checkBoxUseClientCertificate.setEnabled(false);
//			checkBoxUseClientCertificate.setSelected(false);
//			btnChooseCaFile.setEnabled(false);
//			btnChooseClientCert.setEnabled(false);
//			btnChooseClientPk.setEnabled(false);
//			textFieldCAFile.setEnabled(false);
//			textFieldClientCertFile.setEnabled(false);
//			textFieldClientPkFile.setEnabled(false);
//		}
//	}
//	
//	private void useClientCert(boolean use) {
//		if(use) {
//			btnChooseClientCert.setEnabled(true);
//			btnChooseClientPk.setEnabled(true);
//			textFieldClientCertFile.setEnabled(true);
//			textFieldClientPkFile.setEnabled(true);
//		}else{
//			btnChooseClientCert.setEnabled(false);
//			btnChooseClientPk.setEnabled(false);
//			textFieldClientCertFile.setEnabled(false);
//			textFieldClientPkFile.setEnabled(false);
//		}
//	}
//	
//	private void enableTelemetry(boolean enable) {
//		textFieldTelemetrySeconds.setEnabled(enable);
//		checkBoxTelemetryInput.setEnabled(enable);
//		checkBoxTelemetryOutput.setEnabled(enable);
//		checkBoxTelemetryMemory.setEnabled(enable);
//	}
	
	class TableMouseListener extends MouseAdapter{
		
		private JTable table;
		
		public TableMouseListener(JTable table) {
			this.table = table;
		}
		
		@Override
		public void mousePressed(MouseEvent e) {
			Point point = e.getPoint();
			int currentRow = table.rowAtPoint(point);
	        table.setRowSelectionInterval(currentRow, currentRow);
		}
	}
	
	/*
	 * 
	 * Table model
	 * 
	 */
	class Device2Model extends AbstractTableModel{

		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;
		private String[] columnNames = {"ID",Strings.device2()};
		private List<String> inputList;

		public Device2Model(List<String> inputList) {
			super();
			this.inputList = inputList;
		}
		
		@Override
		public String getColumnName(int column) {
			return columnNames[column];
		}
		@Override
		public int getRowCount() {
			// TODO Auto-generated method stub
			return inputList.size();
		}

		@Override
		public int getColumnCount() {
			return columnNames.length;
		}

		@Override
		public Object getValueAt(int rowIndex, int columnIndex) {
			// TODO Auto-generated method stub
			int count = 0;
				for(String item: inputList) {
					if(rowIndex == count) {
						switch(columnIndex) {
						case 0:
							return rowIndex+1;
						case 1:
							return item;
						case 2:
							return null;
						}
					}
					count++;
				}
			return null;
		}

		@Override
		public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
			// TODO Auto-generated method stub
			super.setValueAt(aValue, rowIndex, columnIndex);
		}

		@Override
		public void fireTableDataChanged() {
			// TODO Auto-generated method stub
			super.fireTableDataChanged();
		}
		
		
	}
}
