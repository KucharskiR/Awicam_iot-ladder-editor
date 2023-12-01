package com.github.leofds.iotladdereditor.view.event;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
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
	
	private static String TEXT_IF_NO_VERSION = "-";
	
	private String installedCore;
	private String lastAvailableCore;
	private Boolean isUpdated;
	private Boolean isInstalled;
	private Boolean hasChanged;

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public EspUpdate() {
		
		installedCore = TEXT_IF_NO_VERSION;
		lastAvailableCore = TEXT_IF_NO_VERSION;
		isUpdated = false;
		isInstalled = false;
		hasChanged = false;
		
		
		
		setTitle(Strings.espUpdateTitle());
		setSize(350, 176);
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setLocationRelativeTo(null);
		
		// Checking current and available version, is core is installed etc.
		check();
		
//		panel_1.add(btnUpdate);
		
		JPanel panel_2 = new JPanel();
		panel_2.setBorder(new EmptyBorder(0, 10, 0, 150));
		getContentPane().add(panel_2, BorderLayout.SOUTH);
		panel_2.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_3 = new JPanel();
		FlowLayout flowLayout_1 = (FlowLayout) panel_3.getLayout();
		flowLayout_1.setAlignment(FlowLayout.LEFT);
		panel_2.add(panel_3, BorderLayout.SOUTH);
		
//		JPanel panel_4 = new JPanel();
//		panel_2.add(panel_4, BorderLayout.NORTH);
		JPanel panel_4 = new JPanel();
		FlowLayout flowLayout = (FlowLayout) panel_4.getLayout();
		flowLayout.setAlignment(FlowLayout.LEFT);
		
		String marks = new String();
		marks = isInstalled && !isUpdated ? " !!!" : "";
		
		// Inastalled core label
		JLabel lblESP32InstalledCore = new JLabel(Strings.installedEspLibrary() +": " + installedCore);
		lblESP32InstalledCore.setHorizontalAlignment(SwingConstants.LEFT);
		lblESP32InstalledCore.setFont(new Font("Tahoma", Font.PLAIN, 11));
		lblESP32InstalledCore.setBorder(new EmptyBorder(0, 10, 0, 15));
		
			if (lastAvailableCore.equals(TEXT_IF_NO_VERSION)) {
				lblESP32InstalledCore.setText(Strings.installedEspLibrary() +": " + "not found");
			} else {
				lblESP32InstalledCore.setText(Strings.installedEspLibrary() +": " + installedCore);
			}
			panel_4.add(lblESP32InstalledCore);
		
		// Last core label
		JLabel lblESP32LastCore = new JLabel(Strings.lastAvailableLibrary() +": " + lastAvailableCore + marks);
		lblESP32LastCore.setHorizontalAlignment(SwingConstants.LEFT);
		lblESP32LastCore.setFont(new Font("Tahoma", Font.PLAIN, 11));
		lblESP32LastCore.setBorder(new EmptyBorder(0, 10, 0, 15));
		
			if (lastAvailableCore.equals(TEXT_IF_NO_VERSION)) {
				lblESP32LastCore.setText(Strings.lastAvailableLibrary() +": " +  "not found");
			} else {
				lblESP32LastCore.setText(Strings.lastAvailableLibrary() +": " +  lastAvailableCore);
			}
		
		panel_3.add(lblESP32LastCore);
		
		panel_2.add(panel_4, BorderLayout.NORTH);
		getContentPane().add(panel_2, BorderLayout.NORTH);
		
		JPanel panel = new JPanel();
		panel.setBorder(new EmptyBorder(0, 10, 0, 10));
		getContentPane().add(panel, BorderLayout.CENTER);
		
		JPanel panel_1 = new JPanel();
		panel.add(panel_1);
		
		JPanel panel_Buttons = new JPanel();
		getContentPane().add(panel_Buttons, BorderLayout.SOUTH);
		
		JButton btnInstall = new JButton("Install");
		btnInstall.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {

				Thread install = new Thread(() -> {
					try {
						/*
						 * 
						 */

						// Command to run
						String command = "cmd /c arduino-cli core install esp32:esp32";

						String currentWorkingDirectory = System.getProperty("user.dir");

						// Working directory
						String workingDirectory = currentWorkingDirectory + "/out/plc"; // Replace with your desired
																						// directory path

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
						while ((line = reader.readLine()) != null) {
							System.out.println(line);
							consoleOutput(line);
						}

						// Wait for the process to complete
						int exitCode = process.waitFor();
						consoleOutput(Strings.processExitedWithCode() + " "  + exitCode);

						if (exitCode == 0)
//							TODO: strings install
							consoleOutput("console install succ");
						else
							consoleOutput("console install erro");

					} catch (IOException | InterruptedException e1) {
						e1.printStackTrace();
						consoleOutput(e1.getMessage());
					}

				});
				
				// Start install thread
				install.start();
			}
		});
		
		btnInstall.setFont(new Font("Tahoma", Font.PLAIN, 11));
		btnInstall.setEnabled(false);
		panel_Buttons.add(btnInstall);
		
		JButton btnUpdate = new JButton("Update");
		btnUpdate.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {

				Thread update = new Thread(() -> {
					try {
						/*
						 * 
						 */

						// Command to run
						String command = "cmd /c arduino-cli core upgrade esp32:esp32";

						String currentWorkingDirectory = System.getProperty("user.dir");

						// Working directory
						String workingDirectory = currentWorkingDirectory + "/out/plc"; // Replace with your desired
																						// directory path

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
						while ((line = reader.readLine()) != null) {
							System.out.println(line);
							consoleOutput(line);
						}

						// Wait for the process to complete
						int exitCode = process.waitFor();
						consoleOutput(Strings.processExitedWithCode() + " "  + " "  + exitCode);

						if (exitCode == 0)
//							TODO: strings upgrade
							consoleOutput(Strings.libraryUpdateSuccess());
						else
							consoleOutput(Strings.libraryUpdateFalse());

					} catch (IOException | InterruptedException e1) {
						e1.printStackTrace();
						consoleOutput(e1.getMessage());
					}

				});
				
				// Start update thread
				update.start();
			}
		});
		btnUpdate.setFont(new Font("Tahoma", Font.PLAIN, 11));
		btnUpdate.setEnabled(false);
		panel_Buttons.add(btnUpdate);
		
		String isUpdatedLbl = new String();
		
		if (isInstalled && isUpdated)
			isUpdatedLbl = Strings.espLibrariesInstalled();
		else if (isInstalled && !isUpdated) {
			isUpdatedLbl = Strings.newLibraryAvailable();
			btnUpdate.setEnabled(true);
		}
		else {
			isUpdatedLbl = Strings.libraryNotInstalled();
			btnInstall.setEnabled(true);
		}
		
		
		JLabel lblIsUpdated = new JLabel(isUpdatedLbl);
		lblIsUpdated.setFont(new Font("Tahoma", Font.PLAIN, 11));
		
		panel_1.add(lblIsUpdated);
		
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
				// Output the message from arduino-cli core list
				consoleOutput(line);
				if (line.contains("esp")) {

					// Split the string using regular expression
					parts = line.split("\\s+");
					installedCore = parts[1];
					lastAvailableCore = parts[2];
				}
			}
			
			// Check if installed and available version is same
			if (installedCore.equalsIgnoreCase(lastAvailableCore) && !installedCore.equals(TEXT_IF_NO_VERSION)) {
				isUpdated = true;
				hasChanged = true;
			}
			
			// Checking whether the core is installed at all
			if(isUpdated && hasChanged)
				isInstalled = true;
			
			// Wait for the process to complete
			int exitCode = process.waitFor();
			consoleOutput(Strings.processExitedWithCode() + " "  + exitCode);
			
			if (exitCode == 0)
				consoleOutput("OK!");
			else 
				consoleOutput(Strings.errorTryAgain());
			
		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
		}
	}

	private static void consoleOutput(String msg) {
		Mediator.getInstance().outputConsoleMessage(msg);
	}
	
}
