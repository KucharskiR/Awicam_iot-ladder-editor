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

public class Compiler{
	
	private int compilationStatus;
	private int uploadingStatus;

	public int getCompilationStatus() {
		return compilationStatus;
	}

	public void setCompilationStatus(int compilationStatus) {
		this.compilationStatus = compilationStatus;
	}

	public Compiler(int compilationStatus) {
		super();
		this.compilationStatus = compilationStatus;
	}

	public Compiler() {
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

	public void compile() {
		// command cmd compile function

		// Info string
		String info = Strings.compilationStartInfo();
		
		this.compilationStatus = 2;

		// Output to the console
		consoleOutput(info+"...");
		
		// Create waiting window
//		createAndShowWaitingWindow();

		try {
			/*
			 * 
			 * Replace "your-command-here" with the actual command you want to 
			 * run in the separate cmd window. The "/c" flag is used to indicate 
			 * that the command should be executed and then the cmd window should be closed.
			 *  The "start" command is used to open a new cmd window, and the "/k" flag 
			 *  is used to keep the cmd window open after the command execution.
			 * 
			 */
			
			// Command to run
//			String command = "cmd /c start cmd /k arduino-cli compile --fqbn esp32:esp32:esp32s2 plc.ino"; // Replace "dir" with your desired command
//			String command = "cmd /c arduino-cli compile --fqbn esp32:esp32:esp32c3 plc.ino"; // Replace "dir" with your desired command
			String command = "cmd /c arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc --build-property build.extra_flags=-DCORE_DEBUG_LEVEL=5 plc.ino"; // Replace "dir" with your desired command

			String currentWorkingDirectory = System.getProperty("user.dir");
//			System.out.println("Current Working Directory: " + currentWorkingDirectory);

			// Working directory
			String workingDirectory = currentWorkingDirectory + "/out/plc"; // Replace with your desired directory path
			/*
			 * //
			 * C:\Users\Dell\Documents\KucharskiR_projects\20230803_Ladder_Editor\Awicam_iot
			 * -ladder-editor\out\plc
			 * 
			 * // String workingDirectory = "C:/Users/Dell/Documents/KucharskiR_projects/"
			 * // + "20230803_Ladder_Editor/Awicam_iot-ladder-editor/out/plc"; // Replace
			 * with your desired directory path
			 * 
			 * // String workingDirectory = "C:/path/to/your/directory"; // Replace with
			 * your desired directory path
			 * 
			 */

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
			this.compilationStatus = exitCode == 0 ? 0 : 1;
			consoleOutput("Process exited with code: " + exitCode);
			
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

		// command cmd compile function

		// Info string
		String info = Strings.uploadingInfo();
		
		this.uploadingStatus = 2;

		// Output to the console
		consoleOutput(info);
		
		// Create waiting window
//		createAndShowWaitingWindow();

		try {
			/*
			 * 
			 * Replace "your-command-here" with the actual command you want to 
			 * run in the separate cmd window. The "/c" flag is used to indicate 
			 * that the command should be executed and then the cmd window should be closed.
			 *  The "start" command is used to open a new cmd window, and the "/k" flag 
			 *  is used to keep the cmd window open after the command execution.
			 * 
			 */
			
			// Command to run
			String command = "cmd /c arduino-cli upload -p " + port + " --fqbn esp32:esp32:esp32c3 plc.ino"; // Replace "dir" with your desired command

			String currentWorkingDirectory = System.getProperty("user.dir");
//			System.out.println("Current Working Directory: " + currentWorkingDirectory);

			// Working directory
			String workingDirectory = currentWorkingDirectory + "/out/plc"; // Replace with your desired directory path
			/*
			 * //
			 * C:\Users\Dell\Documents\KucharskiR_projects\20230803_Ladder_Editor\Awicam_iot
			 * -ladder-editor\out\plc
			 * 
			 * // String workingDirectory = "C:/Users/Dell/Documents/KucharskiR_projects/"
			 * // + "20230803_Ladder_Editor/Awicam_iot-ladder-editor/out/plc"; // Replace
			 * with your desired directory path
			 * 
			 * // String workingDirectory = "C:/path/to/your/directory"; // Replace with
			 * your desired directory path
			 * 
			 */

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
			this.uploadingStatus = exitCode == 0 ? 0 : 1;
			consoleOutput("Process exited with code: " + exitCode);
			
			return exitCode;

		} catch (IOException | InterruptedException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
			return 1;
		}
	
	}
}
