package com.github.leofds.iotladdereditor.view.event;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.Thread.UncaughtExceptionHandler;
import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import org.apache.commons.codec.binary.Hex;

import com.fazecast.jSerialComm.SerialPort;
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

	private SerialPort comPort;
	private InputStream inputStream;
	private OutputStream outputStream;

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
		try {
			this.comPort = SerialPort.getCommPort(portName);
			comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
			comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);
			this.inputStream = comPort.getInputStream();
			this.outputStream = comPort.getOutputStream();
			consoleOutput(Strings.portConnected());

		} catch (Exception e) {
			consoleOutput(Strings.portConnectingError() + " " + e.getMessage());

		}
	}
	
	public SerialCommunication() {
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

	public static void main(String[] args) {
		// Create a Scanner object to read user input

		SerialCommunication serialConnection = new SerialCommunication("COM4", 115200);

        Scanner scanner = new Scanner(System.in);

        System.out.print("Choose:\n1-Sending \n"
        		+ "2-Receiving \n"
        		+ "3-Sending(testing)");
        
        // File path
        String zipFilePath = System.getProperty("user.dir") + "/out/Container.zip";
        // File
        File file = new File(zipFilePath);

        int input = scanner.nextInt();

        switch (input) {
		case 1:
			serialConnection.send(file);
//			byte[] data = new byte[] { 0x01, 0x41, (byte) 0xa0, 0x10, 0x41, (byte) 0xa0, 0x10, 0x41,
//					(byte) 0xa0, 0x10 };
//			long crc = CRC.calculateCRC(CRC.Parameters.CRC8, data);
////			crc = longTo1Byte(crc);
//
//			System.out.printf("CRC: 0x%X\n", crc);
//
//			SerialCommunication serial = new SerialCommunication();
//			// byte[] data = Files.readAllBytes(file.toPath());
////			byte[] packet = serial.packetGen((byte) USB_COMMAND_INIT_WRITE_LD, data);
//			byte[] packet = serial.packetGen((byte) 0x01, data);
//			System.out.println("Packet: " + Hex.encodeHexString(packet));

			break;
		case 2:
//			serialConnection.receive();
			break;
		case 3:
//			serialConnection.send(file);
			byte[] data = new byte[] { 0x01, 0x41, (byte) 0xa0, 0x10, 0x41, (byte) 0xa0, 0x10, 0x41,
					(byte) 0xa0, 0x10 };
			long crc = CRC.calculateCRC(CRC.Parameters.CRC8, data);
//			crc = longTo1Byte(crc);

			System.out.printf("CRC: 0x%X\n", crc);

			SerialCommunication serial = new SerialCommunication();
			// byte[] data = Files.readAllBytes(file.toPath());
//			byte[] packet = serial.packetGen((byte) USB_COMMAND_INIT_WRITE_LD, data);
			byte[] packet = serial.packetGen((byte) 0x01, data);
			System.out.println("Packet: " + Hex.encodeHexString(packet));

			break;
		default:
			break;
		}
        scanner.close();
	}

	public SerialPort getComPort() {
		return comPort;
	}

	public void receive(File file) {

		Thread serialReceiving = new Thread(() -> {
			try {
				consoleOutput("Connecting...");

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

	public void send(File fileIn) {

		Thread send = new Thread(() -> {
			try {
				consoleOutput("Connecting...");

				if (comPort.openPort()) {
					consoleOutput("Connected!");
					
					FileInputStream fileInputStream = new FileInputStream(fileIn);
					//TODO: usunać streamy --> przeniesione jako zmienne klasy
//					InputStream inputStream = comPort.getInputStream();
//					OutputStream outputStream = comPort.getOutputStream();

					consoleOutput("File to send: " + fileIn.toString());
// TODO: odebrać wszystko i wyczyścić bufor
//					inputStream.read();
					// Convert file size (in bytes) from long to 4 bytes array
					byte[] fileSize = new byte[4];
					fileSize = longTo4Bytes(fileIn.length());
					// Init packet
					byte[] initComm = packetGen((byte)USB_COMMAND_INIT_WRITE_LD, fileSize);
					// Sending init command to ESP
					outputStream.write(initComm);
					
					// If response from ESP is OK than go into sending file block
					if (isEspResponseOk(responseFromESP(inputStream))) {
						success(Success.SUCCESS_RECEIVED_OK);
						consoleOutput("ESP response OK");
						
						// Sending file procedure
						// 61 bytes buffer declaration
						byte[] buffer = new byte[61];
						@SuppressWarnings("unused")
						int len;
						while ((len = fileInputStream.read(buffer)) > 0) {
							try {
								// Packet generate
								byte[] packet = packetGen((byte)USB_COMMAND_WRITE_LD, buffer);
								
								// Print
								consoleOutput("Packet length: " + packet.length + " bytes");
								consoleOutput("Packet: " + Hex.encodeHexString(packet));
								// Send
								outputStream.write(packet);
								
								// Check response from ESP
								if(!isEspResponseOk(responseFromESP(inputStream))) {
									error(Error.ERROR_RECEIVING);
									throw new Exception("Error receive exception");
								}
								
							} catch (Exception e) {
								error(Error.ERROR_SEND);
							}
							// Wait after each packet 
							TimeUnit.MILLISECONDS.sleep(1);
						}
						
						// Close file input stream
						fileInputStream.close();

						// Send END OF DATA
						byte[] packet = packetGen((byte)USB_COMMAND_END_WRITE_LD, null);
						outputStream.write(packet);
						
						try {

							byte[] resArr = responseFromESP(inputStream);

							if (isEspResponseOk(resArr)) {
								success(Success.SUCCESS_SEND);
								// Close connection
								closeCOM();
							}
							else if (!isEspResponseOk(resArr)) {
								error(Error.ERROR_FROM_ESP);
								closeCOM();
							}
							else {
								error(Error.ERROR_ESP_NOT_SEND_OK);
								closeCOM();
							}

						} catch (Exception e) {
							e.printStackTrace();
							error(Error.ERROR_RECEIVING_OK);
							closeCOM();
						}
					} else {
						error(Error.ERROR_ESP_NOT_SEND_OK);
						closeCOM();
					}

					// Close connection
					closeCOM();

				} else {
					error(Error.ERROR_OPEN_SERIAL);
					throw new IOException();
				}
			} catch (Exception e) {
				e.printStackTrace();
				error(Error.ERROR_SEND );
				// Print to console error from thread
				consoleOutput(e.getMessage());
			}
		}); // End of thread

		// Start thread
		send.start();
	}
	
	/* Name: closeCOM
	 * 
	 * Description:
	 * 	Closing input and output streams and finally close COM port
	 */
	private synchronized void closeCOM() throws IOException {
		this.inputStream.close();
		this.outputStream.close();
		this.comPort.clearBreak();
		this.comPort.removeDataListener();
//		this.comPort.setComPortTimeouts(SerialPort.TIMEOUT_NONBLOCKING, 0, 0);
		this.comPort.clearRTS();
		this.comPort.clearDTR();
		this.comPort.closePort();
		System.out.println("Last error code: " + comPort.getLastErrorCode() + ", " + comPort.getLastErrorLocation());
		this.comPort = null;
	}
	/*
	 *
	 * Packet structrue: <command><length><data><crc>
	 * 
	 */
	
	private byte[] packetGen(byte command, byte[] data) {
		/*  Build command
		 *  
		  Bits: 	   1         1    0<61   1
		  Bytes:	<command><length><data><crc>
		 */
		
		// Whole packet byte[] declaration
		int len = (data == null) ? 3 : data.length + 3;
		byte[] sequence = new byte[len-1];
		
		// Add <command> and <length>
		sequence[0] = command;
		sequence[1] = (byte) len;
		
		// Data adding loop
		if (data != null) {
			int c = 0;
			for (c = 0; c < data.length; c++) {
				sequence[c + 2] = data[c];
			}
		}
		
		// Calculate CRC
//		TODO: poniższy komentarz usunąć 
//		System.out.println("Before crc: " + Hex.encodeHexString(sequence));
		byte[] sequenceCrc = new byte[sequence.length+1];
		// Copy arrays
		System.arraycopy(sequence, 0, sequenceCrc, 0, sequence.length);
		// Generate CRC
		long crc8 = CRC.calculateCRC(CRC.Parameters.CRC8, sequence);
		// Add CRC on the end
		sequenceCrc[sequenceCrc.length-1] = (byte)crc8;
		
		return sequenceCrc;
	}
	/*
	 * Name: dataPacket()
	 * 
	 * Description:
	 * 	Retrieve <data> from packet received from the device
	 * 
	 * Return:
	 * 	bytes array (byte[])
	 */
	
	private byte[] dataPacket(byte[] data) {
		int len = data.length - 3;
		byte[] out = new byte[len];
		
		for (int i=0; i<len; i++) 
			out[i] = data[i+2];
		
		return out;
	}
	/* Name:	longTo4Bytes
	 * 
	 * Description:
	 *	 Generate byte[] array (4 bytes size) from Long 
	 *	 Long need to be less than 4294967295 
	 *
	 * return:
	 * 	byte[]
	 */
	private static byte[] longTo4Bytes(Long value) {
		byte[] bytes = new byte[4];
		
		bytes[0] = (byte) (value & 0xFF);
		bytes[1] = (byte) ((value >> 8) & 0xFF);
		bytes[2] = (byte) ((value >> 16) & 0xFF);
		bytes[3] = (byte) ((value >> 24) & 0xFF);
		
		return bytes;
	}
	
	@SuppressWarnings("unused")
	private static byte longTo1Byte(Long value) {
		byte bytes;
		bytes = (byte) (value & 0xFF);
		return bytes;
	}
	
	@SuppressWarnings("unused")
//	TODO: Prawdopodobnie już niepotrzebne ---> do usunięcia
	private static String toBinaryString(int bits, int input) {
		
		String val = Integer.toBinaryString(input);
		String paramBits = "%" + Integer.toString(bits) + "s";
		return String.format(paramBits, val).replace(' ', '0');
	}
	/* Name:	isEspResponseOk
	 * 
	 * Description:
	 *	 Check most significiant bit.
	 *	 0 --> packet OK
	 *	 1 --> error 
	 *
	 * return:
	 * 	boolean true(MSB=0) or false(MSB=1)
	 */
	private static boolean isEspResponseOk(byte[] rensponse) {
		return (rensponse[0] & 0xFF) >> 7 == 0 ? true : false;
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
			consoleOutput("Available bytes: " + availableBytes);

			byte[] bufferIn = new byte[availableBytes];
			inputStream.read(bufferIn);
			// TODO: printowanie do usunięcia gdy wszystko będzie działać
			consoleOutput("Response: " + Hex.encodeHexString(bufferIn));
//			consoleOutput("MSB: " + isEspResponseOk(bufferIn));
			
			// Error handling from ESP device
			if (!isEspResponseOk(bufferIn)) {
				byte[] data = dataPacket(bufferIn);
				int err = data[data.length - 1];

				switch (err) {
				case USB_ERROR_BAD_CRC:
					consoleOutput("Device error-> Bad CRC");
					break;
				case USB_ERROR_BAD_LENGTH:
					consoleOutput("Device error-> Bad packet length (bytes)");
					break;
				case USB_ERROR_NO_COMMAND:
					consoleOutput("Device error-> No command in the packet");
					break;
				case USB_ERROR_COMMAND_ERROR:
					consoleOutput("Device error-> Internal command error");
					break;

				default:
					break;
				}
			}
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
			consoleOutput("Ladder info-> Sending thread error");
			break;
		case ERROR_RECEIVING:
			consoleOutput("Ladder info-> Receiving error");
			break;
		case ERROR_RECEIVING_OK:
			consoleOutput("Ladder info-> Device not sent OK response");
			break;
		case ERROR_SEND_TIME:
			consoleOutput("Ladder info-> Wait time after sending packet error");
			break;
		case ERROR_ESP_NOT_SEND_OK:
			consoleOutput("Ladder info-> File sended but not received OK from the device");
			break;
		case ERROR_FROM_ESP:
			consoleOutput("Ladder info-> ESP sent an error");
			break;
		case ERROR_OPEN_SERIAL:
			consoleOutput("Ladder info-> Can not open serial port");
			break;
		case ERROR_WAITING_ESP:
			consoleOutput("Ladder info-> Error during waiting for ESP response");
			break;
		case ERROR_RESPONSE_TIMEOUT:
			consoleOutput("Ladder info-> Response from the device timeout");
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
//		TODO: mediator output przywrócić po skończeniu
//		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
