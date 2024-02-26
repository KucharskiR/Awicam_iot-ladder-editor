package com.github.leofds.iotladdereditor.view.event;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.concurrent.TimeUnit;

import org.apache.commons.codec.binary.Hex;

import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.view.event.SerialCommunication.Error;
import com.github.leofds.iotladdereditor.view.event.SerialCommunication.Success;

public class send {

	public send() {
		// TODO Auto-generated constructor stub
	}
	public void send(File fileIn) {
		Thread send = new Thread(() -> {
			try {

				if (comPort.openPort()) {
					
					FileInputStream fileInputStream = new FileInputStream(fileIn);
					//TODO: usunać streamy --> przeniesione jako zmienne klasy
//					InputStream inputStream = comPort.getInputStream();
//					OutputStream outputStream = comPort.getOutputStream();

					consoleOutput(Strings.fileToSend() + " " + fileIn.toString());
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
}
