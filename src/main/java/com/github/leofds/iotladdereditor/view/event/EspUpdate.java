package com.github.leofds.iotladdereditor.view.event;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Font;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.i18n.Strings;

public class EspUpdate extends JFrame {
	
	private String installedCore;
	private String lastAvailableCore;

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public EspUpdate() {
		
		installedCore = "-";
		lastAvailableCore = "-";
		
		setTitle("ESP core updater");
		setSize(350, 176);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);
		
		check();
		
		JPanel panel = new JPanel();
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		
		JPanel panel_1 = new JPanel();
		panel_1.setLayout(new BorderLayout(0, 0));
		
		
		JButton btnUpdate = new JButton(Strings.espUpdateBtn());
		btnUpdate.setBorder(new EmptyBorder(10, 70, 10, 70));
		btnUpdate.setFont(new Font("Tahoma", Font.PLAIN, 14));
//		panel_1.add(btnUpdate);
		
		JPanel panel_2 = new JPanel();
		getContentPane().add(panel_2, BorderLayout.SOUTH);
		panel_2.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_3 = new JPanel();
		panel_2.add(panel_3, BorderLayout.SOUTH);
		
//		JPanel panel_4 = new JPanel();
//		panel_2.add(panel_4, BorderLayout.NORTH);
		
		JLabel lblESP32Core = new JLabel("ESP32 core: ");
		lblESP32Core.setHorizontalAlignment(SwingConstants.LEFT);
		lblESP32Core.setFont(new Font("Tahoma", Font.PLAIN, 14));
		panel_3.add(lblESP32Core);
		
		JLabel lblESP32InstalledCore = new JLabel("Installed\r\n: " + installedCore);
		lblESP32InstalledCore.setHorizontalAlignment(SwingConstants.LEFT);
		lblESP32InstalledCore.setFont(new Font("Tahoma", Font.ITALIC, 13));
		lblESP32InstalledCore.setBorder(new EmptyBorder(0, 10, 0, 15));
		panel_3.add(lblESP32InstalledCore);
		
		JLabel lblESP32LastCore = new JLabel("Last available\r\n: " + lastAvailableCore);
		lblESP32LastCore.setHorizontalAlignment(SwingConstants.LEFT);
		lblESP32LastCore.setFont(new Font("Tahoma", Font.ITALIC, 13));
		lblESP32LastCore.setBorder(new EmptyBorder(0, 10, 0, 15));
		panel_3.add(lblESP32LastCore);
		
		JPanel panel_4 = new JPanel();
		panel_2.add(panel_4, BorderLayout.NORTH);
		
//		JLabel lblStatus = new JLabel(Strings.compilationStatus());
//		panel_4.add(lblStatus);
//		lblStatus.setHorizontalAlignment(SwingConstants.LEFT);
//		lblStatus.setFont(new Font("Tahoma", Font.PLAIN, 14));

		getContentPane().add(panel);
		getContentPane().add(panel_1, BorderLayout.NORTH);
	}
	
	public void check() {

		// Info string
		String info = Strings.checkingESP32Updates();
		
		// Output to the console
		consoleOutput(info+"...");
		
		// Create waiting window
//		createAndShowWaitingWindow();

		try {
			// Command to run
			String command = "cmd /c arduino-cli core list"; 

			String currentWorkingDirectory = System.getProperty("user.dir");
//			System.out.println("Current Working Directory: " + currentWorkingDirectory);

			// Working directory
			String workingDirectory = currentWorkingDirectory + "/out/plc"; // Replace with your desired directory path

			// Create the process builder
			ProcessBuilder processBuilder = new ProcessBuilder(command.split(" "));

			// Set the working directory
			processBuilder.directory(new File(workingDirectory));

			// Redirect error stream to output stream
			processBuilder.redirectErrorStream(true);

			// Start the process
			Process process = processBuilder.start();

			// Get the process output
			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
			String line;
			String[] parts;
			while ((line = reader.readLine()) != null) {
				if (line.contains("esp")) {
					// Split the string using regular expression
					parts = line.split("\\s+");
					installedCore = parts[1];
					lastAvailableCore = parts[2];
				}
			}

			// Wait for the process to complete
			int exitCode = process.waitFor();
			consoleOutput("Process exited with code: " + exitCode);
			
			if (exitCode == 0)
//				consoleOutput("\n************************* SUCCESSFULLY COMPILED!****************************\r\n"
//						+ "Select the COM port and press the Upload button to send program to the device ");
				consoleOutput("\n************************* ESP32 CORE INSTALLED AND UPDATED! ****************************");
			else 
				consoleOutput("\nCOMPILATION ERROR!");
			
		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
		}
	}

	private static void consoleOutput(String msg) {
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
