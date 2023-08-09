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
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.io.File;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.SwingUtilities;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.Compiler;
import com.github.leofds.iotladdereditor.view.event.Subject.SubMsg;

public class BuildRunEvent implements Observer {

	private Subject subject;

	public BuildRunEvent(Subject subject) {
		subject.addObserver(this);
		this.subject = subject;
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
			case ESP32_ARDUINO_FREERTOS:
				Desktop.getDesktop().open(new File("out/plc/plc.ino"));
				yesNoDialog();
				break;
			case W1VC_ESP32_FREERTOS:
				Desktop.getDesktop().open(new File("out/plc/plc.ino"));
				yesNoDialog();
				break;
			default:
				break;
			}
		} catch (Exception e) {
			e.printStackTrace();
			me.outputConsoleMessage(e.getMessage());
		}
	}

	private void yesNoDialog() {
		// TODO confirm dialog box
		JFrame frame = new JFrame("Compiling...");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		// sharing thread data
		SharedResource sharedResource = new SharedResource();
		
		Thread progressThread = new Thread(() -> {
			// progress bar Thread 1
			JProgressBar progressBar = new JProgressBar(0,100);
			progressBar.setStringPainted(true); // Display the percentage on the bar
			progressBar.setPreferredSize(new Dimension(250, 30));
			frame.setLayout(new FlowLayout());
			frame.add(progressBar);
			frame.pack();
			frame.setVisible(true);
			
			for (int i = 0; i <= 100; i++) {
				progressBar.setValue(i);
				try {
					if (!sharedResource.getData()) {
						if (i > 80) {
							Thread.sleep(800); // Simulating progress updates
						} else {
							Thread.sleep(400); // Simulating progress updates
						}
					} else if(sharedResource.getData()) {
						Thread.sleep(50); // Simulating progress updates
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			frame.getContentPane().remove(progressBar);
			SwingUtilities.invokeLater(() -> {
				JOptionPane.showMessageDialog(frame, "Operation completed!", "Information",
						JOptionPane.INFORMATION_MESSAGE);
			});
			
			frame.dispose();
		});
		
		// compile Thread2
	    Thread compileThread = new Thread(() -> {
            // Operation 2 code here
	    			sharedResource.setData(false);
                    Compiler.compile();
                    sharedResource.setData(true);
        });
		
		int choice = JOptionPane.showConfirmDialog(frame, "Do you want to proceed compilation?", "Confirmation",
				JOptionPane.YES_NO_OPTION);

		if (choice == JOptionPane.YES_OPTION) {
//			System.out.println("Yes");
			try {
				Thread.sleep(10); // Wait for 2 seconds
				consoleOutput("Compilation running...");
				frame.dispose(); // Close the window if "No" is chosen
				
				
				// start threads
				progressThread.start();
				compileThread.start();
				
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			
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

	    synchronized void setData(boolean isFinished) {
	        this.isFinished = isFinished;
	    }

	    synchronized boolean getData() {
	        return isFinished;
	    }
	}

}
