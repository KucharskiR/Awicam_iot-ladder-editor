package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Font;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

public class ImportExportLadderProgram extends JFrame {

	private static final long serialVersionUID = 1L;

	public ImportExportLadderProgram() {
		setAlwaysOnTop(true);
		setTitle("Import/Export ladder program");
		setSize(400, 176);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);
		
		JPanel panel_0 = new JPanel();
		getContentPane().add(panel_0, BorderLayout.SOUTH);
		
		JButton btnImport = new JButton("Import");
		btnImport.setHorizontalTextPosition(SwingConstants.LEADING);
		btnImport.setFont(new Font("Tahoma", Font.PLAIN, 14));
		
		JButton btnExport = new JButton("Export");
		btnExport.setHorizontalTextPosition(SwingConstants.LEADING);
		btnExport.setFont(new Font("Tahoma", Font.PLAIN, 14));
		
		panel_0.add(btnImport);
		panel_0.add(btnExport);
	}

}
