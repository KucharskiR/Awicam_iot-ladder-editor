package com.github.leofds.iotladdereditor.ladder.symbol.instruction.source.view;

import java.awt.Dimension;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.border.EmptyBorder;

import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.ladder.view.DialogScreen;

public class SourcePropertyScreen extends DialogScreen {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public SourcePropertyScreen(String title) {
		super(title);
		
		JPanel panel = new JPanel(null);
		panel.setPreferredSize(new Dimension(200, 160));
		panel.setBorder(new EmptyBorder(5, 5, 5, 5));
		
//		JLabel openFileLbl = new JLabel("Open file:", SwingConstants.RIGHT);
//		panel.add(openFileLbl, BorderLayout.NORTH);
		
		JSeparator separator = new JSeparator();
		separator.setBounds(5, 110, 190, 2);
		panel.add(separator);

		JButton btnSaveButton = new JButton(Strings.save());
		btnSaveButton.setBounds(10, 120, 89, 23);
		btnSaveButton.addActionListener(getSaveAction());
		panel.add(btnSaveButton);

		JButton btnCancelButton = new JButton(Strings.cancel());
		btnCancelButton.setBounds(100, 120, 89, 23);
		btnCancelButton.addActionListener(getCancelAction());
		panel.add(btnCancelButton);
		
		setContentPane(panel);
	}
	
}
