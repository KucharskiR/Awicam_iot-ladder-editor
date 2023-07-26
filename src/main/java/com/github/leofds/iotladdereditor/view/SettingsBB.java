package com.github.leofds.iotladdereditor.view;

import java.awt.Dimension;

import javax.swing.JDialog;

public class SettingsBB extends JDialog{

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
//	private final JPanel contentPanel = new JPanel();

	public SettingsBB() {
		super();
		
		try {
			
			setPreferredSize(new Dimension(400, 500));
			setTitle("Settings");
			
			setModal(true);
			setResizable(true);
			
			setBounds(100, 100, 400, 500);
//			getContentPane().setLayout(new BorderLayout());
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
