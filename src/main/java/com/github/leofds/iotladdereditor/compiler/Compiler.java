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
package com.github.leofds.iotladdereditor.compiler;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.application.ProjectContainer;
import com.github.leofds.iotladdereditor.compiler.analizer.SemanticAnalyzer;
import com.github.leofds.iotladdereditor.compiler.domain.IR;
import com.github.leofds.iotladdereditor.compiler.generator.CodeGenerator;
import com.github.leofds.iotladdereditor.compiler.generator.IRGenerator;
import com.github.leofds.iotladdereditor.compiler.generator.factory.CodeGeneratorFactory;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.util.FileUtils;

import com.github.cliftonlabs.json_simple.JsonException;
import com.github.cliftonlabs.json_simple.JsonObject;
import com.github.cliftonlabs.json_simple.Jsoner;
import java.util.Set;

public class Compiler {
	
	private int compilationStatus;
	private int uploadingStatus;
	private String platformioPath;
	private String pythonPath;
	private String platformioFilePath;
	private String workingDirectory;
	private String srcDirectory;

	public int getCompilationStatus() {
		return compilationStatus;
	}

	public int getUploadingStatus() {
		return uploadingStatus;
	}

	public void setUploadingStatus(int uploadingStatus) {
		this.uploadingStatus = uploadingStatus;
	}

	public void setCompilationStatus(int compilationStatus) {
		this.compilationStatus = compilationStatus;
	}

	public Compiler(int compilationStatus) {
		super();
		this.compilationStatus = compilationStatus;
		this.setWorkingDir();
	}

	public Compiler() {
		this.setWorkingDir();
	}

	public static boolean build(ProjectContainer project){

		printDate();
		Mediator.getInstance().outputConsoleMessage(Strings.compiling()+"...");

		if(SemanticAnalyzer.analyze(project.getLadderProgram())){
			IR ir = IRGenerator.generate(project.getLadderProgram());
			project.setIr(ir);

			try {
				FileUtils.createFile("out/ladder.ir", ir.getTextQuadruple());
				CodeGenerator codeGenerator = CodeGeneratorFactory.create( project.getLadderProgram().getProperties().getCodeOption() );
				SourceCode sCode = codeGenerator.generate(project);

				for(SourceFile src:sCode.getFiles()){
					FileUtils.createFile(String.format("out/%s", src.getFname()),src.getContent());
				}

				Mediator.getInstance().outputConsoleMessage(Strings.successfullyCompleted());
				project.setCompiled(true);
				return true;
			} catch (IOException e) {
				Mediator.getInstance().outputConsoleMessage(Strings.failToCreateFile());
			}
		}else{
			project.setCompiled(false);
			Mediator.getInstance().outputConsoleMessage(Strings.terminatedWithError());	
		}
		return false;
	}

	private static void printDate() {
		SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		String date = simpleDateFormat.format(new Date());
		consoleOutput(date);
	}

	private void setWorkingDir() {
		String currentWorkingDirectory = System.getProperty("user.dir");

		this.workingDirectory = currentWorkingDirectory + "/out";

		this.srcDirectory = workingDirectory + "/plc-controller";
		
		this.pythonPath = workingDirectory + "/Python311-32/python.exe";

		this.platformioFilePath = this.workingDirectory + "tmp.js";
	}

	private boolean downloadPio() {

		try {
			String command = this.pythonPath + " get-pio.py"; 
			// Create the process builder
			ProcessBuilder processBuilder = new ProcessBuilder(command.split(" "));

			// Set the working directory
			processBuilder.directory(new File(this.workingDirectory));

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
			
			if (exitCode != 0)
			{
				consoleOutput("ERROR \r\nPlatformIO not install! ");
				return false;
			}
			consoleOutput("\nPlatformIO installed successfully!\r\n");
			
		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
			return false;
		}

		return true;
	}

	private boolean loadPio() {
		// Info string
		// String info = Strings.compilationStartInfo();	

		// Output to the console
		consoleOutput("Looking for PlatformIO ...");
		
		// Create waiting window
    // createAndShowWaitingWindow();

		try {
			// TODO add read python path
 
			// String pythonPath = "py";
			String command = this.pythonPath + " get-pio.py check core --dump-state " + this.platformioFilePath; 

			// Create the process builder
			ProcessBuilder processBuilder = new ProcessBuilder(command.split(" "));

			// Set the working directory
			processBuilder.directory(new File(this.workingDirectory));

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
			
			if (exitCode != 0)
			{
				consoleOutput("PlatformIO not found! Installing...");
				return false;
			}
			consoleOutput("\nPlatformIO found!\r\n");

			try (FileReader jsonFile = new FileReader(this.platformioFilePath)) {
				JsonObject jsonObject = (JsonObject) Jsoner.deserialize(jsonFile);

				// Print json file
				Set<String> keys = jsonObject.keySet();
				for (String key : keys) {
					consoleOutput(key + ": " + jsonObject.get(key).toString());
				}

				this.platformioPath = jsonObject.get("platformio_exe").toString();
				this.pythonPath = jsonObject.get("python_exe").toString();

			} catch (IOException | JsonException e) {
				// throw new RuntimeException(e);
			}

			File file = new File(this.platformioFilePath);
			file.delete();
				
		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
			return false;
		}
		return true;
	}

	public void compile() {
		// command cmd compile function

		// Info string
		String info = Strings.compilationStartInfo();
		
		this.compilationStatus = 2;

		// Output to the console
		consoleOutput(info + "...");
		
		// Create waiting window
//		createAndShowWaitingWindow();

		try {
			// Load PlatformIO path
			if (loadPio() == false) {
				downloadPio();
				loadPio();
			}
			
			String controllerName = Mediator.getInstance().getProject().getLadderProgram().getDevice().getName();
			String command = this.platformioPath 
			// + " --silent"
			+ " run --environment " + controllerName;
			
			// Create the process builder
			ProcessBuilder processBuilder = new ProcessBuilder(command.split(" "));

			// Set the working directory
			processBuilder.directory(new File(this.srcDirectory));

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
			this.compilationStatus = exitCode == 0 ? 0 : 1;
			consoleOutput("Process exited with code: " + exitCode);
			
			if (exitCode == 0)
				consoleOutput("\n************************* SUCCESSFULLY COMPILED!****************************\r\n"
						+ "Select the COM port and press the Upload button to send program to the device ");
			else 
				consoleOutput(Strings.compilationError());
			
		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
			this.compilationStatus = 1;
		}
	}

	private static void consoleOutput(String msg) {
		Mediator.getInstance().outputConsoleMessage(msg);
	}

	public int upload(String port) {
		// TODO Auto-generated method stub

		// Info string
		String info = Strings.uploadingInfo();
		
		this.uploadingStatus = 2;

		// Output to the console
		consoleOutput(info);
		
		// Create waiting window
//		createAndShowWaitingWindow();

		try {
			// Load PlatformIO path
			if (loadPio() == false) {
				downloadPio();
				loadPio();
			}

			// Command to run
			String controllerName = Mediator.getInstance().getProject().getLadderProgram().getDevice().getName();

			String uploadPort = "";
			if (port.length() > 0)
				uploadPort = port;
			else
				uploadPort = Mediator.getInstance().getConnection().getComPort().toString();

			String command = this.platformioPath 
			+ " run --environment " + controllerName 
			// + " --silent"
			+ " --target nobuild" 
			+ " --target upload"
			+ " --upload-port " + uploadPort; 


			// Create the process builder
			ProcessBuilder processBuilder = new ProcessBuilder(command.split(" "));

			// Set the working directory
			processBuilder.directory(new File(this.srcDirectory));

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
			this.uploadingStatus = exitCode == 0 ? 0 : 1;
			consoleOutput("Process exited with code: " + exitCode);
			
			if (exitCode == 0)
				consoleOutput(Strings.successfullyUploadConsole());
			else 
				consoleOutput(Strings.uploadingError());
			
			return exitCode;

		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
			return 1;
		}
	}
}
