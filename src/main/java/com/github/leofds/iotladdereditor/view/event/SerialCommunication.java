package com.github.leofds.iotladdereditor.view.event;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Scanner;
import java.util.concurrent.TimeUnit;

import org.apache.commons.codec.binary.Hex;

import com.fazecast.jSerialComm.SerialPort;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.util.crc.CRC;

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

	public SerialCommunication() {
		
	}
	
	public void start(String portName, int baudRate) {
		try {
			consoleOutput(Strings.connecting() + "...");
			this.comPort = SerialPort.getCommPort(portName);
			comPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
			comPort.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);
			this.inputStream = comPort.getInputStream();
			this.outputStream = comPort.getOutputStream();
			// Print Connected on port 
			consoleOutput(Strings.portConnected() + portName);

		} catch (Exception e) {
			// Print connection error to the console
			consoleOutput(Strings.portConnectingError() + " " + e.getMessage());

		}
	}
	/* Name: closeCOM
	 * 
	 * Description:
	 * 	Closing input and output streams and finally close COM port
	 */
	private void closeCOM() throws IOException {
		synchronized (this) {
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

		SerialCommunication serial = new SerialCommunication();
		serial.start("COM4", 115200);

        Scanner scanner = new Scanner(System.in);

        System.out.print("Choose:\n1-Sending \n"
        		+ "2-Receiving \n"
        		+ "3-Sending(testing)");
        
        // File path send
        String zipFilePath = System.getProperty("user.dir") + "/out/Container.zip";
        // File path to receivee
        String fileRecPath = System.getProperty("user.dir") + "/out/ContainerRec.zip";
        // File send
        File fileSend = new File(zipFilePath);
        // File receive
        File fileReceive = new File(fileRecPath);

        int input = scanner.nextInt();

        switch (input) {
		case 1:
			serial.send(fileSend);
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
				serial.receive(fileReceive);
			break;
		case 3:
//			serialConnection.send(file);
			byte[] data = new byte[] { 0x01, 0x41, (byte) 0xa0, 0x10, 0x41, (byte) 0xa0, 0x10, 0x41,
					(byte) 0xa0, 0x10 };
			long crc = CRC.calculateCRC(CRC.Parameters.CRC8, data);
//			crc = longTo1Byte(crc);

			System.out.printf("CRC: 0x%X\n", crc);

			// byte[] data = Files.readAllBytes(file.toPath());
//			byte[] packet = serial.packetGen((byte) USB_COMMAND_INIT_WRITE_LD, data);
			byte[] packet = serial.packetGen((byte) 0x01, data);
			System.out.println("Packet: " + Hex.encodeHexString(packet));

			break;
		default:
			break;
		}
        	try {
        		Thread.sleep(80);
        		serial.closeCOM();
        	} catch (Exception e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	}
        scanner.close();
	}

	public SerialPort getComPort() {
		return comPort;
	}

	public int receive(File fileOut) {
		try {
			if (comPort.openPort()) {

				// Create a FileOutputStream to save the received file
				FileOutputStream fileOutputStream = new FileOutputStream(fileOut.getAbsolutePath());

				// Sending start command to ESP
				byte[] initComm = packetGen((byte) USB_COMMAND_INIT_READ_LD, null);
				outputStream.write(initComm);

				// Get response from ESP
				byte[] resp = responseFromESP(inputStream);

				// Get length of file (that will be send) in bytes from response
				long fileLen = convertToLong(littleEndian(dataPacket(resp)));
				System.out.println("File len: " + fileLen);

				if (isEspResponseOk(resp)) {
					success(Success.SUCCESS_RECEIVED_OK);
					consoleOutput("ESP response OK");

					long lenCnt = 0;
					// Read and write the file data
					do {
						// Send command for next chunk
						byte[] next = packetGen((byte) USB_COMMAND_READ_LD, null);
						outputStream.write(next);

						byte[] read = responseFromESP(inputStream);

						if (isEspResponseOk(read) && isCRCOk(read)) {
							// Get <data> from read bytes and write to output stream
							byte[] write = dataPacket(read);

							if (write.equals(null))
								break;

							fileOutputStream.write(write, 0, write.length);

							// Sum data bytes to be compared to lenght
							lenCnt += write.length;
						} else {
							// Not correct CRC info
							if (!isCRCOk(read)) {
								consoleOutput("Ladder info-> CRC not correct");
								return -1;
							} else {
								error(Error.ERROR_RECEIVING_OK);
								return -1;
							}
						}

						// Check that the number of bytes received equals the number of bytes expected
						// (var length)
						if (lenCnt >= fileLen) {
							if (lenCnt == fileLen) {
								// If sum sent bytes = length send end command
								byte[] end = packetGen((byte) USB_COMMAND_END_READ_LD, null);
								outputStream.write(end);

								if (!isEspResponseOk(responseFromESP(inputStream))) {
									error(Error.ERROR_RECEIVING_OK);
									return -1;
								} else {
									consoleOutput("File received!");
									break;
								}
							} else {
								// Size not correct
								consoleOutput("Ladder info-> Size of received file not correct");
								return -1;
							}
						}

					} while (lenCnt <= fileLen);

					// Close file output stream
					fileOutputStream.close();

				} else {
					error(Error.ERROR_ESP_NOT_SEND_OK);
					return -1;
				}

				// TODO: Prawdopodobnie do usunięcia, zamknięcię streamów przesunięte do
				// closeCOM()
				// Close the streams and serial port
//					fileOutputStream.close();
//					inputStream.close();
//					comPort.closePort();
			} else {
				error(Error.ERROR_OPEN_SERIAL);
				throw new IOException();
			}
			// TODO: trzeba gdzieś to przesunąć w inne miejsce
//				success(Success.SUCCESS_RECEIVED);
		} catch (Exception e) {
			e.printStackTrace();
			error(Error.ERROR_RECEIVING);
			return -1;
		}
		return 0;
	}
	
	public int send(File fileIn) {
		try {

			if (comPort.openPort()) {

				FileInputStream fileInputStream = new FileInputStream(fileIn);

				consoleOutput(Strings.fileToSend() + " " + fileIn.toString());

				// Clear buffor
				consoleOutput("Ladder info-> Clearing device buffor...");
				@SuppressWarnings("unused")
				byte[] clear = responseFromESP(inputStream);
				clear = null;

				// Convert file size (in bytes) from long to 4 bytes array
				byte[] fileSize = new byte[4];
				fileSize = longTo4Bytes(fileIn.length());

				// Init packet
				byte[] initComm = packetGen((byte) USB_COMMAND_INIT_WRITE_LD, fileSize);

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
							byte[] packet = packetGen((byte) USB_COMMAND_WRITE_LD, buffer);

							// Print
							consoleOutput("Packet length: " + packet.length + " bytes");
							consoleOutput("Packet: " + Hex.encodeHexString(packet));
							// Send
							outputStream.write(packet);

							// Check response from ESP
							if (!isEspResponseOk(responseFromESP(inputStream))) {
								error(Error.ERROR_RECEIVING);
								throw new Exception("Error receive exception");
							}

						} catch (Exception e) {
							error(Error.ERROR_SEND);
							fileInputStream.close();
							return -1;
						}
						// Wait after each packet
						TimeUnit.MILLISECONDS.sleep(1);
					}

					// Close file input stream
					fileInputStream.close();

					// Send END OF DATA
					byte[] packet = packetGen((byte) USB_COMMAND_END_WRITE_LD, null);
					outputStream.write(packet);

					try {

						byte[] resArr = responseFromESP(inputStream);

						if (isEspResponseOk(resArr)) {
							success(Success.SUCCESS_SEND);
							// Close connection
							closeCOM();
						} else if (!isEspResponseOk(resArr)) {
							error(Error.ERROR_FROM_ESP);
							closeCOM();
						} else {
							error(Error.ERROR_ESP_NOT_SEND_OK);
							closeCOM();
						}

					} catch (Exception e) {
						e.printStackTrace();
						error(Error.ERROR_RECEIVING_OK);
						return -1;
					}
				} else {
					error(Error.ERROR_ESP_NOT_SEND_OK);
					fileInputStream.close();
					return -1;
				}

				// Close connection

			} else {
				error(Error.ERROR_OPEN_SERIAL);
				throw new IOException();
			}
		} catch (Exception e) {
			e.printStackTrace();
			error(Error.ERROR_SEND);
			// Print to console error from thread
			consoleOutput(e.getMessage());
			return -1;
		}
		return 0;
	}
	
	/*
	 *
	 * Packet structrue: <command><length><data><crc>
	 * 
	 */
	
	private byte[] packetGen(byte command, byte[] data) {
		/*  Build command
		 *  
		  Bytes: 	   1         1    0<61   1
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
	
	/*
	 * Name: littleEndian()
	 * 
	 * Description:
	 * 	Swap byte arrays (convert endians)
	 * 
	 * Return:
	 * 	bytes array (byte[])
	 */
	private static byte[] littleEndian(byte[] data) {
		int c = data.length - 1;
		byte[] out = new byte[data.length];

		for (byte b : data) {
			out[c] = b;
			c--;
		}
		return out;
	}
	
	/* Name:	convertToLong
	 * 
	 * Description:
	 *	 A utility function that return long value from a byte array 
	 *
	 * return:
	 * 	long
	 */
	static long convertToLong(byte[] bytes) {
		long value = 0l;

		// Iterating through for loop
		for (byte b : bytes) {
			// Shifting previous value 8 bits to right and
			// add it with next value
			value = (value << 8) + (b & 255);
		}

		return value;
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
	
	/* Name:	isCRCOk
	 * 
	 * Description:
	 *	 Check is CRC ok
	 *
	 * return:
	 * 	boolean 
	 */
	private static boolean isCRCOk(byte[] data) {
		return (CRC.calculateCRC(CRC.Parameters.CRC8, data) == 0) ? true : false ;
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
			consoleOutput("Ladder info-> Receiving function error");
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
