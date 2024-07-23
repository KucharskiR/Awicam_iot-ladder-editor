package com.github.leofds.iotladdereditor.ladder.symbol.instruction.source.view;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.device.DeviceMemory;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.ladder.view.DialogScreen;

public class SourcePropertyScreen extends DialogScreen {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private String filePath;
	private DeviceMemory memory;
	
	public String getFilePath() {
		return filePath;
	}

	public void setFilePath(String filePath) {
		this.filePath = filePath;
	}

	public SourcePropertyScreen(String title, DeviceMemory memory) {
		super(title);
		this.memory = memory;
		
		JPanel panel = new JPanel(null);
		panel.setPreferredSize(new Dimension(200, 100));
		panel.setBorder(new EmptyBorder(5, 5, 5, 5));
		
		JLabel openFileLbl = new JLabel("Open file:", SwingConstants.RIGHT);
		openFileLbl.setBounds(10, 10, 56, 14);
		panel.add(openFileLbl);
		
		JButton btnOpen = new JButton(Strings.open());
		btnOpen.setBounds(76, 10, 100, 20);
		btnOpen.addActionListener(openFile());
		panel.add(btnOpen);
		
//		filePath = FieldFactory.createLongField(1, 999);
//		filePath.setHorizontalAlignment(SwingConstants.RIGHT);
//		filePath.setBounds(76, 10, 86, 20);
//		panel.add(filePath);
		
		JSeparator separator = new JSeparator();
		separator.setBounds(5, 50, 190, 2);
		panel.add(separator);

		JButton btnSaveButton = new JButton(Strings.save());
		btnSaveButton.setBounds(10, 60, 89, 23);
		btnSaveButton.addActionListener(getSaveAction());
		panel.add(btnSaveButton);

		JButton btnCancelButton = new JButton(Strings.cancel());
		btnCancelButton.setBounds(100, 60, 89, 23);
		btnCancelButton.addActionListener(getCancelAction());
		panel.add(btnCancelButton);
		
		setContentPane(panel);
	}
	
	@Override
	public boolean validateFilds() {
		//TODO: create validate fields here
		return true;
	}
	
	private ActionListener openFile() {
		return new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				// Utworzyć obiekt JFileChooser
				JFileChooser fileChooser = new JFileChooser();

				// Określić domyślny katalog
				// (opcjonalnie)
				// fileChooser.setCurrentDirectory(new File("/ścieżka/do/domyślnego/katalogu"));

				// Ustawić tytuł okna dialogowego
				fileChooser.setDialogTitle(Strings.choose());

				// Określić typy plików do wyświetlenia
				// (opcjonalnie)
				// String[] extensions = new String[] { "txt", "pdf" };
				// fileChooser.setFileFilter(new FileNameExtensionFilter("Pliki tekstowe i PDF",
				// extensions));

				// Wyświetlić okno dialogowe wyboru pliku
				int returnValue = fileChooser.showOpenDialog(null);

				// Sprawdzić, czy plik został wybrany
				if (returnValue == JFileChooser.APPROVE_OPTION) {
					// Pobranie wybranego pliku
					File selectedFile = fileChooser.getSelectedFile();
					
					setFilePath(selectedFile.getAbsolutePath());
					
					Mediator.getInstance().outputConsoleMessage(Strings.fileChoose() + " " + selectedFile.getAbsolutePath());
				}
			};
		};
	}
	
}
