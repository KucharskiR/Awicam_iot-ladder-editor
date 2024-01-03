package com.github.leofds.iotladdereditor.view.event;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.i18n.Strings;



public class SerialCommunication {
	
	private static final int USB_COMMAND_SEND_LD = 0x11;
	private static final int USB_COMMAND_RECEIVE_LD = 0x12;
	private static final byte[] USB_ESP_OK = new byte[]{(byte) 0xff, (byte) 0x16, (byte) 0xff};
	private static final byte[] USB_ESP_ERROR = new byte[]{(byte) 0xff, (byte) 0x17, (byte) 0xff};;
	
	private static final int TIMEOUT = 10000; // Timeout (milliseconds) response from device 
	
	private static enum Error {ERROR_RECEIVING, ERROR_SEND, ERROR_RECEIVING_OK, ERROR_SEND_TIME, ERROR_ESP_NOT_SEND_OK, ERROR_FROM_ESP, ERROR_OPEN_SERIAL, ERROR_WAITING_ESP, ERROR_RESPONSE_TIMEOUT};
	private static enum Success {SUCCESS_RECEIVED, SUCCESS_SEND, SUCCESS_RECEIVED_OK};
	
	private static final byte[] END_OF_DATA = new byte[]{(byte) 0xff, (byte) 0xfe, (byte) 0xff};
	
	private String portName;
	private int baudRate;
	private SerialPort comPort;
	
//	private String lastConsoleOutput = "";
//	private OutputStream outputStream;
//
//	public String getLastConsoleOutput() {
//		return lastConsoleOutput;
//	}
//
//	public void setLastConsoleOutput(String lastConsoleOutput) {
//		this.lastConsoleOutput = lastConsoleOutput;
//	}
//
//	public OutputStream getOutputStream() {
//		return outputStream;
//	}
//
//	public void setOutputStream(OutputStream outputStream) {
//		this.outputStream = outputStream;
//	}

	public SerialCommunication(String portName, int baudRate) {
		this.portName = portName; // Com port name
		this.baudRate = baudRate;
		
		try {
			this.comPort = SerialPort.getCommPort(portName);
			comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
			comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);
			consoleOutput(Strings.portConnected());
			
		} catch (Exception e) {
			consoleOutput(Strings.portConnectingError() + " " + e.getMessage());
			
		}
	}

//	public static void main(String[] args) {
//		// Create a Scanner object to read user input
//		
//		SerialCommunication serialConnection = new SerialCommunication("COM4");
//		
//        Scanner scanner = new Scanner(System.in);
//        
//        System.out.print("Choose:\n1-Sending \n2-Receiving \n");
//        
//        int input = scanner.nextInt();
//        
//        switch (input) {
//		case 1:
//			serialConnection.send();
//			break;
//		case 2:
//			serialConnection.receive();
//			break;
//
//		default:
//			break;
//		}
//
//	}
	
	public void receive() {
		
		Thread serialReceiving = new Thread(() -> {
			try {
				consoleOutput("Connecting...");

				SerialPort comPort = SerialPort.getCommPort(portName);
				comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
				comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);

				if (comPort.openPort()) {

					InputStream inputStream = comPort.getInputStream();
					OutputStream outputStream = comPort.getOutputStream();

					// Create a FileOutputStream to save the received file
					FileOutputStream fileOutputStream = new FileOutputStream("received_file.ld");

					// Sending start command to ESP
					outputStream.write(USB_COMMAND_RECEIVE_LD);
					consoleOutput("Start command sent to ESP");
					
					if (Arrays.equals(responseFromESP(inputStream), USB_ESP_OK)) {
						
						// Define a buffer for receiving data
						byte[] buffer = new byte[64];

						int bytesRead;
						
//						if (responseFromESP(inputStream) != null) {

							// Read and write the file data
							while ((bytesRead = inputStream.read(buffer)) != -1) {
								consoleOutput("Reading " + bytesRead + " bytes");
								consoleOutput(Arrays.toString(buffer));
								fileOutputStream.write(buffer, 0, bytesRead);
							}
							//TODO: obsługa błędów + odczyt ostatnich trzech bajtów (ESP OK)
//						}
					
				} else 
					error(Error.ERROR_ESP_NOT_SEND_OK);
					
					// Close the streams and serial port
					fileOutputStream.close();
					inputStream.close();
					comPort.closePort();
				} else {
					error(Error.ERROR_OPEN_SERIAL);
					throw new IOException();
				}
				success(Success.SUCCESS_RECEIVED);
			} catch (Exception e) {
				e.printStackTrace();
				error(Error.ERROR_RECEIVING);
			}
		}); // End thread
		
		serialReceiving.start();
	}
	
	public CompletableFuture<byte[]> receive(int inputCommand) {
		/*
		 * 
		 * byte[] result = resultFuture.get(); // This will block until the result is available
 				Now you can use the 'result' variable
 				
 				byte[] bytes = receive(int inputCommand).get();
		 */
		
		CompletableFuture<byte[]> resultFuture = new CompletableFuture<>();

		Thread serialReceiving = new Thread(() -> {

			try {
				consoleOutput("Connecting...");

				SerialPort comPort = SerialPort.getCommPort(portName);
				comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
				comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);

				if (comPort.openPort()) {
					try (InputStream inputStream = comPort.getInputStream();
							ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
							OutputStream outputStream = comPort.getOutputStream()) {

						// Sending begin command to ESP
						outputStream.write(inputCommand);
						consoleOutput("Command sent to ESP");

						if (Arrays.equals(responseFromESP(inputStream), USB_ESP_OK)) {
							// Define a buffer for receiving data
							byte[] buffer = new byte[64];
							int bytesRead;

							// Read and write the file data
							while ((bytesRead = inputStream.read(buffer)) != -1) {
								consoleOutput("Reading " + bytesRead + " bytes");
								consoleOutput(Arrays.toString(buffer));
								byteArrayOutputStream.write(buffer, 0, bytesRead);
							}

							// Retrieve the result bytes
							byte[] bytes = byteArrayOutputStream.toByteArray();
							resultFuture.complete(bytes);
						} else {
							error(Error.ERROR_ESP_NOT_SEND_OK);
						}
					} catch (IOException e) {
						error(Error.ERROR_RECEIVING);
						e.printStackTrace();
					} finally {
						// Close the streams and serial port
						comPort.closePort();
					}
				} else {
					error(Error.ERROR_ESP_NOT_SEND_OK);
				}

				success(Success.SUCCESS_RECEIVED);
			} catch (Exception e) {
				e.printStackTrace();
				error(Error.ERROR_OPEN_SERIAL);
			}
		}); // End thread

		serialReceiving.start();
		return resultFuture;
	}

	public void send() {
		String fileName = System.getProperty("user.dir") + "/blink.ld";
		
		Thread send = new Thread(() -> {
			try {
				consoleOutput("Connecting...");

				SerialPort comPort = SerialPort.getCommPort(portName);
				comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
				comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);

				if (comPort.openPort()) {

					File file = new File(fileName);
					FileInputStream fileInputStream = new FileInputStream(file);
					InputStream inputStream = comPort.getInputStream();
					OutputStream outputStream = comPort.getOutputStream();

					consoleOutput("File to send: " + file.toString());

					// Sending start command to ESP
					outputStream.write(USB_COMMAND_SEND_LD);

					// USB_ESP_OK
					try {
						
						// If response from ESP is OK than go into sending file block
						if (responseFromESP(inputStream) == USB_ESP_OK) {
							success(Success.SUCCESS_RECEIVED_OK);

							// Sending file procedure
							byte[] buffer = new byte[64];
							int len;
							while ((len = fileInputStream.read(buffer)) > 0) {
								try {
									consoleOutput("Sending " + len + " bytes");
									outputStream.write(buffer, 0, len);
								} catch (Exception e) {
									error(Error.ERROR_SEND);
								}

								TimeUnit.MILLISECONDS.sleep(5);
							}

							// Send END OF DATA
							outputStream.write(END_OF_DATA);
							
							try {

								byte[] resArr = responseFromESP(inputStream);
								
								if (Arrays.equals(resArr, USB_ESP_OK))
									success(Success.SUCCESS_SEND);
								else if (Arrays.equals(resArr, USB_ESP_ERROR))
									error(Error.ERROR_FROM_ESP);
								else
									error(Error.ERROR_ESP_NOT_SEND_OK);
								
							} catch (Exception e) {
								e.printStackTrace();
								error(Error.ERROR_RECEIVING_OK);
							}
						} else
							error(Error.ERROR_ESP_NOT_SEND_OK);
					} catch (InterruptedException e) {
						e.printStackTrace();
						error(Error.ERROR_RECEIVING_OK);
					}

					// Close connection
					fileInputStream.close();
					outputStream.close();
					comPort.closePort();

				} else {
					error(Error.ERROR_OPEN_SERIAL);
					throw new IOException();
				}
			} catch (Exception e) {
				e.printStackTrace();
				error(Error.ERROR_SEND);
			}
		}); // End of thread
		
		// Start thread
		send.start();
	}
	
	public void send(File file) {
		// TODO Auto-generated method stub
		
	}
	

	private byte[] responseFromESP(InputStream inputStream) {
		long startTime = System.currentTimeMillis();
		int counter = 0;

		try {
			// Waiting loop for data from ESP
			while (inputStream.available() == 0) {
//				consoleOutput(inputStream.available());
				counter++;
				if (counter > 15) {
					if (System.currentTimeMillis() >= (startTime + TIMEOUT)) {
						error(Error.ERROR_RESPONSE_TIMEOUT);
						return null;
					}
					counter = 0;
				}
			}

			int availableBytes = inputStream.available();
			consoleOutput("available bytes: " + availableBytes);

			byte[] bufferIn = new byte[availableBytes];
			inputStream.read(bufferIn);
			consoleOutput(Arrays.toString(bufferIn));
			
			return bufferIn;
			
		} catch (IOException e) {
			error(Error.ERROR_WAITING_ESP);
			e.printStackTrace();
			return null;
		}
	}

	private void error(Error error) {
		switch (error) {
		case ERROR_SEND:
			consoleOutput("Sending error");
			break;
		case ERROR_RECEIVING:
			consoleOutput("Receiving error");
			break;
		case ERROR_RECEIVING_OK:
			consoleOutput("Receiving OK from the device error");
			break;
		case ERROR_SEND_TIME:
			consoleOutput("Wait time after sending packet error");
			break;
		case ERROR_ESP_NOT_SEND_OK:
			consoleOutput("File sended but not received OK from the device");
			break;
		case ERROR_FROM_ESP:
			consoleOutput("ESP send an error");
			break;
		case ERROR_OPEN_SERIAL:
			consoleOutput("Can not open serial port");
			break;
		case ERROR_WAITING_ESP:
			consoleOutput("Error during waiting for ESP response");
			break;
		case ERROR_RESPONSE_TIMEOUT:
			consoleOutput("Response from the device timeout");
			break;
			
		default:
			break;
		}
	}
	
	private void success(Success success) {
		switch (success) {
		case SUCCESS_SEND:
			consoleOutput("Succesfully sended");
			break;
		case SUCCESS_RECEIVED:
			consoleOutput("Succesfully received");
			break;
		case SUCCESS_RECEIVED_OK:
			consoleOutput("Received OK from ESP!");
			break;

		default:
			break;
		}
	}
	
	private void consoleOutput(String msg) {
//		lastConsoleOutput = msg;
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
