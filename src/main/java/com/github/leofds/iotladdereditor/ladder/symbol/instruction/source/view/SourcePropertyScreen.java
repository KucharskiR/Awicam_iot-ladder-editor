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
	private JLabel pathLbl = new JLabel("", SwingConstants.LEFT);
	
	
	public JLabel getPathLbl() {
		return pathLbl;
	}

	public void setPathLbl(JLabel pathLbl) {
		this.pathLbl = pathLbl;
	}

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
		panel.setPreferredSize(new Dimension(250, 100));
		panel.setBorder(new EmptyBorder(5, 5, 5, 5));
		
		JLabel openFileLbl = new JLabel("Open file:", SwingConstants.RIGHT);
		openFileLbl.setBounds(35, 10, 56, 14);
		panel.add(openFileLbl);
		
		JButton btnOpen = new JButton(Strings.open());
		btnOpen.setBounds(111, 10, 100, 20);
		btnOpen.addActionListener(openFile());
		panel.add(btnOpen);
		
		pathLbl.setBounds(10, 30, 300, 14);
		panel.add(pathLbl);
//		filePath = FieldFactory.createLongField(1, 999);
//		filePath.setHorizontalAlignment(SwingConstants.RIGHT);
//		filePath.setBounds(76, 10, 86, 20);
//		panel.add(filePath);
		
		JSeparator separator = new JSeparator();
		separator.setBounds(5, 50, 240, 2);
		panel.add(separator);

		JButton btnSaveButton = new JButton(Strings.save());
		btnSaveButton.setBounds(35, 60, 89, 23);
		btnSaveButton.addActionListener(getSaveAction());
		panel.add(btnSaveButton);

		JButton btnCancelButton = new JButton(Strings.cancel());
		btnCancelButton.setBounds(135, 60, 89, 23);
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
				// Create a JFileChooser object
				JFileChooser fileChooser = new JFileChooser();

				// Specify the default directory
				// (optional)
				// fileChooser.setCurrentDirectory(new File("/ścieżka/do/domyślnego/katalogu"));

				// Ustawić tytuł okna dialogowego
				fileChooser.setDialogTitle(Strings.choose());

				// Determine the types of files to display
				// (optional)
				// String[] extensions = new String[] { "txt", "pdf" };
				// fileChooser.setFileFilter(new FileNameExtensionFilter("Pliki tekstowe i PDF",
				// extensions));

				// Display the file selection dialog box
				int returnValue = fileChooser.showOpenDialog(null);

				// Verify that the file has been selected
				if (returnValue == JFileChooser.APPROVE_OPTION) {
					// Downloading the selected file
					File selectedFile = fileChooser.getSelectedFile();
					
					setFilePath(selectedFile.getAbsolutePath());
					pathLbl.setText(pathGenerate(filePath));
					
					Mediator.getInstance().outputConsoleMessage(Strings.fileChoose() + " " + selectedFile.getAbsolutePath());
				}
			};
		};
	}
	
	/**
	 * Makes string (like path) shorter
	 * 
	 * @param text
	 * @return String
	 */
	private static String pathGenerate(String text) {
		if (text.length() <= 37) {
			return text;
		} else {
			int start = 0;
			int end = text.length() - 1;

			StringBuilder sb = new StringBuilder();
			sb.append(text.substring(start, start + 5));
			sb.append("...");
			sb.append(text.substring(end - 29, end + 1));
			
			return sb.toString();
		}
	}
	
}
