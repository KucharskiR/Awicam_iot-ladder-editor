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
package com.github.leofds.iotladdereditor.view.event;

import java.awt.Desktop;
import java.io.File;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JFrame;
import javax.swing.JOptionPane;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.Compiler;
import com.github.leofds.iotladdereditor.util.bars.CompileWaitingBar;
import com.github.leofds.iotladdereditor.util.bars.UploadingWaitingBar;
import com.github.leofds.iotladdereditor.view.event.Subject.SubMsg;

public class BuildRunEvent implements Observer {

	private Subject subject;
	private Compiler compilation;

	public BuildRunEvent(Subject subject) {
		subject.addObserver(this);
		this.subject = subject;
		this.compilation = new Compiler();
	}

	private void build() {
		Mediator me = Mediator.getInstance();
		me.clearConsole();
		Compiler.build(me.getProject());
		
	}
	
	private void buildRun() {
		Mediator me = Mediator.getInstance();
		build();
		try {
			switch(me.getProject().getLadderProgram().getProperties().getCodeOption()) {
//			case ESP32_ARDUINO_FREERTOS:
//				Desktop.getDesktop().open(new File("out/plc/plc.ino"));
//				compilationConfirm();
//				break;
			case W1VC_ESP32_FREERTOS:
//				Desktop.getDesktop().open(new File("out/plc/plc.ino"));
				compilationConfirm();
				break;
			default:
				break;
			}
		} catch (Exception e) {
			e.printStackTrace();
			me.outputConsoleMessage(e.getMessage());
		}
	}

	private void uploading() {
		// TODO Auto-generated method stub
		ComPortChooser comPortChooser = new ComPortChooser();

		JFrame frame = new JFrame("Uploading...");
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

		int choice = JOptionPane.showConfirmDialog(frame, "Do you want to upload?", "Confirmation",
				JOptionPane.YES_NO_OPTION);

		if (choice == JOptionPane.YES_OPTION) {
			System.out.println("Yes");

			comPortChooser.setVisible(true);


		} else if (choice == JOptionPane.NO_OPTION) {
			System.out.println("No");
		}

		frame.pack();
		frame.setVisible(false);

	}

//	private void comPortChooser() {
//
//		System.out.println("comPortChooser");
//
//		Thread uploadingTerminalThread = new Thread(() -> {
//			Compiler uploadingCompiler = new Compiler();
//			uploadingCompiler.upload(comPortChooser.getPortName());
//		});
//		
//		Thread uploadingWaitingBar = new Thread(() -> {
//			UploadingWaitingBar uploadWaitingBar = new UploadingWaitingBar();
//		});
//		
//		if (comPortChooser.isUploadingStart() && comPortChooser.getPortName() != null) {
//			uploadingTerminalThread.start();
//			uploadingWaitingBar.start();
//		} 
//		comPortChooser.setVisible(true);
//
//	}

	private void compilationConfirm() {
		// confirmation compiling dialog box
		JFrame frame = new JFrame("Compiling...");
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

		// sharing thread data
		SharedResource sharedResource = new SharedResource();
		
		Thread progressThread = new Thread(() -> {
			// progress bar Thread 1
//			JProgressBar progressBar = new JProgressBar(0,100);
//			progressBar.setStringPainted(true); // Display the percentage on the bar
//			progressBar.setPreferredSize(new Dimension(250, 30));
//			frame.setLayout(new FlowLayout(FlowLayout.CENTER));
//			frame.add(progressBar);
//			frame.setLocationRelativeTo(null);
//			frame.pack();
//			frame.setVisible(true);
//			
//			for (int i = 0; i <= 99; i++) {
//				progressBar.setValue(i);
//				try {
//					if (!sharedResource.getData()) {
//						if (i > 80) {
//							Thread.sleep(1500); // Simulating progress updates
//						} else if (i > 50) {
//							Thread.sleep(900); // Simulating progress updates
//						} else {
//							Thread.sleep(500); // Simulating progress updates
//						}
//					} else if (sharedResource.getData()) {
//						Thread.sleep(50); // Simulating progress updates
//					}
//				} catch (InterruptedException e) {
//					e.printStackTrace();
//				}
//			}
//			
//			// while loop if 99% but compiling still working
//			while(!sharedResource.getData()) {
//				progressBar.setValue(99);
//			}
//				progressBar.setValue(100);
//			
//			
//			frame.getContentPane().remove(progressBar);
//			SwingUtilities.invokeLater(() -> {
//				JOptionPane.showMessageDialog(frame, "Operation completed!", "Information",
//						JOptionPane.INFORMATION_MESSAGE);
//			});
//			frame.dispose();
			CompileWaitingBar compilingWaitingBar = new CompileWaitingBar();
			
			while(!sharedResource.getData());
			
			compilingWaitingBar.close();
		});
		
		compilation.setCompilationStatus(5);
		// compile Thread2
		Thread compileThread = new Thread(() -> {
			// Operation 2 code here
			sharedResource.setData(false);

			compilation.compile();
			
			if (compilation.getCompilationStatus() == 0) {
				sharedResource.setData(true);
				uploading(); // uploading method invoke
			}
			sharedResource.setCompilationStatus(compilation.getCompilationStatus());
		});
		
		int choice = JOptionPane.showConfirmDialog(frame, "Do you want to proceed compilation?", "Confirmation",
				JOptionPane.YES_NO_OPTION);

		if (choice == JOptionPane.YES_OPTION) {
//			System.out.println("Yes");
//				frame.dispose(); // Close the window if "No" is chosen

			// start threads
			progressThread.start();
			compileThread.start();
//					try {
//						progressThread.join(); // Wait for compileThread to finish
//					} catch (InterruptedException e) {
//						e.printStackTrace();
//					}

		} else if (choice == JOptionPane.NO_OPTION) {
			System.out.println("No");
		}

		frame.pack();
		frame.setVisible(false);
	}

	private void consoleOutput(String msg) {
		// output console 
		Mediator.getInstance().outputConsoleMessage(msg);
	}

	@Override
	public void update(Observable o, Object arg) {
		if(subject instanceof Subject && arg instanceof SubMsg){
			switch((SubMsg) arg) {
			case BUILD:
				build();
				break;
			case BUILD_RUN:
				buildRun();
				break;
			}
		}
	}
	
	class SharedResource {
	    private boolean isFinished;
	    private int compilationStatus; // 0 - success, 1 - error

	    public int getCompilationStatus() {
			return compilationStatus;
		}

		public void setCompilationStatus(int compilationStatus) {
			this.compilationStatus = compilationStatus;
		}

		synchronized void setData(boolean isFinished) {
	        this.isFinished = isFinished;
	    }

	    synchronized boolean getData() {
	        return isFinished;
	    }
	}

}
