package com.github.leofds.iotladdereditor.view.event;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.view.crc.CRC;

/*
 * 
 * ESP documenation
 * From: https://bitbucket.org/twojtczak/esp32-controller/src/master/
 *
 *
 *
 *
PC  ---Send `USB_COMMAND_INIT_WRITE_LD`--> ESP \ Send once
PC  <---------------Response-------------- ESP / to init

PC  ------Send `USB_COMMAND_WRITE_LD`----> ESP \ Send data
PC  <---------------Response-------------- ESP / chunk
                       .
                       . Send other chunks
                       .
PC  ----Send `USB_COMMAND_END_WRITE_LD`--> ESP \ Send once
PC  <---------------Response-------------- ESP / to finalize


Command sequence to read ladder diagram program:

PC  ----Send `USB_COMMAND_INIT_READ_LD`--> ESP \ Send once
PC  <---------------Response-------------- ESP / to init

PC  ------Send `USB_COMMAND_READ_LD`----> ESP \ Send to get next
PC  <---------Response with data--------- ESP / data chunk 
                        .
                        . Send other chunks
                        .
PC  ----Send `USB_COMMAND_END_READ_LD`--> ESP \ Send once
PC  <---------------Response-------------- ESP / to finalize
 *
 */

public class SerialCommunication {

	// Sending commands PC ---> ESP
	private static final int USB_COMMAND_INIT_WRITE_LD = 0x33;
	private static final int USB_COMMAND_WRITE_LD = 0x34;
	private static final int USB_COMMAND_END_WRITE_LD = 0x35;

	// Receiving commands PC <--- ESP
	private static final int USB_COMMAND_INIT_READ_LD = 0x30;
	private static final int USB_COMMAND_READ_LD = 0x31;
	private static final int USB_COMMAND_END_READ_LD = 0x32;

	// ESP packet error responses
	private static final int USB_ERROR_BAD_CRC = 0x01;
	private static final int USB_ERROR_BAD_LENGTH = 0x02;
	private static final int USB_ERROR_NO_COMMAND = 0x03;
	private static final int USB_ERROR_COMMAND_ERROR = 0x04;
	
//	private static final byte[] USB_ESP_OK = new byte[]{(byte) 0xff, (byte) 0x16, (byte) 0xff};  	<--- deprecated 05.02.2024  
//	private static final byte[] USB_ESP_ERROR = new byte[]{(byte) 0xff, (byte) 0x17, (byte) 0xff}; 	<--- deprecated 05.02.2024

	private static final int TIMEOUT = 10000; // Timeout (milliseconds) response from device

	private static enum Error {
		ERROR_RECEIVING, ERROR_SEND, ERROR_RECEIVING_OK, ERROR_SEND_TIME, ERROR_ESP_NOT_SEND_OK, ERROR_FROM_ESP,
		ERROR_OPEN_SERIAL, ERROR_WAITING_ESP, ERROR_RESPONSE_TIMEOUT
	}

	private static enum Success {
		SUCCESS_RECEIVED, SUCCESS_SEND, SUCCESS_RECEIVED_OK
	}

//	private static final byte[] END_OF_DATA = new byte[]{(byte) 0xff, (byte) 0xfe, (byte) 0xff};	<--- depracated 05.02.2024

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

//	public enum EspCommands {
//		INITIALIZING (0x11), IN_PROGRESS (0x11), COMPLETE (0x11);
//
//		private int value;
//
//		private EspCommands(int value) {
//			this.value = value;
//		}
//
//	};

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

	public void receive(File file) {

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
					FileOutputStream fileOutputStream = new FileOutputStream(file.getAbsolutePath());

					// Sending start command to ESP
					outputStream.write(USB_COMMAND_INIT_READ_LD);
					consoleOutput("Start command sent to ESP");

					if (isEspResponseOk(responseFromESP(inputStream))) {

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
						// TODO: obsługa błędów + odczyt ostatnich trzech bajtów (ESP OK)
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
		 * byte[] result = resultFuture.get(); // This will block until the result is
		 * available Now you can use the 'result' variable
		 * 
		 * byte[] bytes = receive(int inputCommand).get();
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

						if (isEspResponseOk(responseFromESP(inputStream))) {
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
					outputStream.write(USB_COMMAND_INIT_WRITE_LD);

					// USB_ESP_OK
					try {

						// If response from ESP is OK than go into sending file block
						if (isEspResponseOk(responseFromESP(inputStream))) {
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
							outputStream.write(USB_COMMAND_END_WRITE_LD);

							try {

								byte[] resArr = responseFromESP(inputStream);

								if (isEspResponseOk(resArr))
									success(Success.SUCCESS_SEND);
								else if (!isEspResponseOk(resArr))
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

	public void send(File fileIn) {

		Thread send = new Thread(() -> {
			try {
				consoleOutput("Connecting...");

				SerialPort comPort = SerialPort.getCommPort(portName);
				comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
				comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);

				if (comPort.openPort()) {
					consoleOutput("Connected!");
					
					FileInputStream fileInputStream = new FileInputStream(fileIn);
					InputStream inputStream = comPort.getInputStream();
					OutputStream outputStream = comPort.getOutputStream();

					consoleOutput("File to send: " + fileIn.toString());

					// Convert file size (in bytes) from long to 4 bytes array
					byte[] fileSize = new byte[4];
					fileSize = longTo4BytesInt(fileIn.length());
					byte[] initComm = packetGen((byte)USB_COMMAND_INIT_WRITE_LD, fileSize);
					// Sending init command to ESP
					outputStream.write(initComm);
					
					try {

						// If response from ESP is OK than go into sending file block
						if (isEspResponseOk(responseFromESP(inputStream))) {
							success(Success.SUCCESS_RECEIVED_OK);
							consoleOutput("ESP response OK");
							
//							// CRC start
//							CRC tableDriven = new CRC(CRC.Parameters.CRC8);

							// Sending file procedure
							byte[] buffer = new byte[61];
							@SuppressWarnings("unused")
							int len;
							while ((len = fileInputStream.read(buffer)) > 0) {
								try {
//									TODO: 13.02.2024 --> usunąć poniższy komentarz w przyszłości.
//									Procedura przeniesiona do funkcji packetGen()
//									Usunąć też CRC start powyżej 
//									
//									// data length (len) +1 byte for command and +1 byte for lenght info
//									int length = len +2;
//									byte command = (byte) USB_COMMAND_WRITE_LD;
//
//									/*  Build command
//									 *  
//									  Bits: 	   1         1    0<61   1
//									  Bytes:	<command><length><data><crc>
//									 */
//									
//									byte[] sequence = new byte[length];
//									
//									// Add <command> and <length>
//									sequence[0] = command;
//									sequence[1] = (byte) length;
//									
//									// Data adding loop
//									int c = 0;
//									for (c = 0; c <= buffer.length-1; c++) {
//										sequence[c + 2] = buffer[c];
//									}
//									
//									// Calculate CRC
//									long crc8 = tableDriven.calculateCRC(sequence);
//									// Copy sequence to Crc sequence and +1 byte for CRC
//									byte[] sequenceCrc = new byte[length+1];
//									System.arraycopy(sequence, 0, sequenceCrc, 0, sequence.length);
//									// Add CRC on the end
//									sequenceCrc[sequenceCrc.length-1] = (byte) crc8;
									
									// Packet generate
									byte[] packet = packetGen((byte)USB_COMMAND_WRITE_LD, buffer);
									
									// Print
									consoleOutput("Sending " + packet.length + " bytes");
									outputStream.write(packet);
									
									// Check response from ESP
									if(!isEspResponseOk(responseFromESP(inputStream))) {
										error(Error.ERROR_SEND);
										throw new Exception("Error send exception");
									}
									
								} catch (Exception e) {
									error(Error.ERROR_SEND);
								}

								TimeUnit.MILLISECONDS.sleep(5);
							}

							// Send END OF DATA
							byte[] packet = packetGen((byte)USB_COMMAND_END_WRITE_LD, null);
							outputStream.write(packet);
							
							try {

								byte[] resArr = responseFromESP(inputStream);

								if (isEspResponseOk(resArr))
									success(Success.SUCCESS_SEND);
								else if (!isEspResponseOk(resArr))
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
	/*
	 *
	 * Packet structrue: <command><length><data><crc>
	 * 
	 */
	
	private byte[] packetGen(byte command, byte[] data) {
		// CRC start
		CRC tableDriven = new CRC(CRC.Parameters.CRC8);
		/*  Build command
		 *  
		  Bits: 	   1         1    0<61   1
		  Bytes:	<command><length><data><crc>
		 */
		
		// Whole packet byte[] declaration
		int len = data.length + 2;
		byte[] sequence = new byte[len];
		
		// Add <command> and <length>
		sequence[0] = command;
		sequence[1] = (byte) len;
		
		// Data adding loop
		if (data != null) {
			int c = 0;
			for (c = 0; c <= data.length - 1; c++) {
				sequence[c + 2] = data[c];
			}
		}
		
		// Calculate CRC
		long crc8 = tableDriven.calculateCRC(sequence);
		// Copy sequence to Crc sequence and +1 byte for CRC
		byte[] sequenceCrc = new byte[len+1];
		System.arraycopy(sequence, 0, sequenceCrc, 0, sequence.length);
		// Add CRC on the end
		sequenceCrc[sequenceCrc.length-1] = (byte) crc8;
		
		return sequenceCrc;
	}
	/* Name:	longTo4BytesInt
	 * 
	 * Description:
	 *	 Generate byte[] array (4 bytes size) from Long 
	 *	 Long need to be less than 4294967295 
	 *
	 * return:
	 * 	byte[]
	 */
	private static byte[] longTo4BytesInt(Long value) {
		byte[] bytes = new byte[4];
		
		bytes[0] = (byte) (value & 0xFF);
		bytes[1] = (byte) ((value >> 8) & 0xFF);
		bytes[2] = (byte) ((value >> 16) & 0xFF);
		bytes[3] = (byte) ((value >> 24) & 0xFF);
		
		return bytes;
	}
	
	private static String toBinaryString(int bits, int input) {
		
		String val = Integer.toBinaryString(input);
		String paramBits = "%" + Integer.toString(bits) + "s";
		return String.format(paramBits, val).replace(' ', '0');
	}
	
	private static boolean isEspResponseOk(byte[] rensponse) {
		int c = (int) rensponse[0];
		char[] arr = toBinaryString(8, c).toCharArray();
		return Integer.parseInt(String.valueOf(arr[0])) == 0 ? true : false;
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
		System.out.println(msg);
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
