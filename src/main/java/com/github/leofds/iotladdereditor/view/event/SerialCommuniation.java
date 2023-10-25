package com.github.leofds.iotladdereditor.view.event;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import com.github.leofds.iotladdereditor.application.Mediator;

import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;

public class SerialCommuniation {
	
	private static final int USB_COMMAND_SEND_LD = 0x11;
	private static final int USB_COMMAND_RECEIVE_LD = 0x12;
	private static final int END_OF_DATA = 0xff + 0xfe + 0xff;
	private static final int USB_ESP_OK = 0x16;
	private static final int USB_ESP_ERROR = 0x17;
	
	private static enum Error {ERROR_RECEIVED, ERROR_SEND};
	private static enum Success {SUCCESS_RECEIVED, SUCCESS_SEND};
	
	private String portName;

	public SerialCommuniation(String portName) {
		this.portName = portName;
	}
	
	public void send() {
		String fileName = System.getProperty("user.dir") + "file.ld";
		
		Thread send = new Thread(() -> {
			try {
				CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(portName);
				SerialPort serialPort = (SerialPort) portIdentifier.open("SerialCommuniation", 2000);

				serialPort.setSerialPortParams(9600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1,
						SerialPort.PARITY_NONE);

				File file = new File(fileName);
				FileInputStream fileInputStream = new FileInputStream(file);
				InputStream inputStream = serialPort.getInputStream();
				OutputStream outputStream = serialPort.getOutputStream();
				
				byte[] buffer = new byte[64];
				int len;
				while ((len = fileInputStream.read(buffer)) > 0) {
					outputStream.write(buffer, 0, len);
					try {
						wait(5);
					} catch (InterruptedException e) {
						e.printStackTrace();
						error(Error.ERROR_SEND);
					}
				}
				
				// Send END OF DATA 
				outputStream.write(END_OF_DATA);
				
				try {
					wait(1);
					
				    int availableBytes = inputStream.available();
		            byte[] bufferIn = new byte[availableBytes];
		            int bytesRead = ByteBuffer.wrap(bufferIn).getInt();
		            
		            
				} catch (InterruptedException e) {
					e.printStackTrace();
					error(Error.ERROR_RECEIVED);
				}
				
				// Close connection
				fileInputStream.close();
				outputStream.close();
				serialPort.close();

				success(Success.SUCCESS_SEND);

			} catch (NoSuchPortException | PortInUseException | IOException | UnsupportedCommOperationException e) {
				e.printStackTrace();
			}
		}); // End of thread
		
		// Start thread
		send.start();
	}
	
	private void error(Error error) {
		//TODO: Error procedure to write
		consoleOutput("error");
	}
	
	private void success(Success success) {
		//TODO: Success procedure to write
		consoleOutput("success");
	}
	
	private static void consoleOutput(String msg) {
		Mediator.getInstance().outputConsoleMessage(msg);
	}
}
