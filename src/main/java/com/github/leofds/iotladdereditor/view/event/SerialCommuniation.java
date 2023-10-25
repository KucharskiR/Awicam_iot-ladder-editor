package com.github.leofds.iotladdereditor.view.event;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;

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
	
	private String portName;

	public SerialCommuniation(String portName) {
		this.portName = portName;
	}
	
	public void send() {
		String fileName = System.getProperty("user.dir");

		try {
			CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(portName);
			SerialPort serialPort = (SerialPort) portIdentifier.open("SerialCommuniation", 2000);

			serialPort.setSerialPortParams(9600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

			File file = new File(fileName);
			FileInputStream fileInputStream = new FileInputStream(file);
			OutputStream outputStream = serialPort.getOutputStream();

			byte[] buffer = new byte[64];
			int len;
			while ((len = fileInputStream.read(buffer)) > 0) {
				outputStream.write(buffer, 0, len);
			}

			fileInputStream.close();
			outputStream.close();
			serialPort.close();

			System.out.println("File sent successfully.");

		} catch (NoSuchPortException | PortInUseException | IOException | UnsupportedCommOperationException e) {
			e.printStackTrace();
		}
	}

}
