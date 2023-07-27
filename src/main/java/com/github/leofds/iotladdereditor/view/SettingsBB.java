package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JPanel;

public class SettingsBB extends JDialog{

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private final JPanel panel = new JPanel();

    public SettingsBB() {
    }
    
	public SettingsBB(int deviceId, String deviceName) {
		super();
		
		try {
			
			setPreferredSize(new Dimension(400, 500));
			setTitle("Settings of "+ deviceName +" device ID: "+ String.valueOf(deviceId));
			
			setModal(true);
			setResizable(true);
			
			setBounds(100, 100, 400, 500);
			getContentPane().setLayout(null);
			
//			JPanel panel = new JPanel();
			getContentPane().add(panel, BorderLayout.CENTER);
			
			JButton btnSave = new JButton("Save");
			btnSave.setBounds(229, 427, 57, 23);
			getContentPane().add(btnSave);
			
			JButton btnCancel = new JButton("Cancel");
			btnCancel.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					SettingsBB.this.dispose();
				}
			});
			btnCancel.setBounds(293, 427, 65, 23);
			getContentPane().add(btnCancel);
			
			JCheckBox chckbxSetting = new JCheckBox("Setting 1");
			chckbxSetting.setBounds(20, 17, 97, 23);
			getContentPane().add(chckbxSetting);
			
			JCheckBox chckbxSetting_1 = new JCheckBox("Setting 2");
			chckbxSetting_1.setBounds(20, 43, 97, 23);
			getContentPane().add(chckbxSetting_1);
			
			JCheckBox chckbxSetting_2 = new JCheckBox("Setting 3");
			chckbxSetting_2.setBounds(20, 69, 97, 23);
			getContentPane().add(chckbxSetting_2);
			
			
//			Border border = BorderFactory.createTitledBorder("Settings");
//			contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
//			contentPanel.setBorder(border);
//			contentPanel.setPreferredSize(new Dimension(400, 500));
			
		} catch (Exception e) {
			// TODO: handle exception
			System.out.println("Settings window error");
		}
		
		
	}

}
