package com.github.leofds.iotladdereditor.view.zip;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import javax.crypto.Cipher;
import javax.crypto.CipherOutputStream;
import javax.crypto.spec.SecretKeySpec;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.view.event.SerialCommunication;

public class ZipContainer {
	private List<File> fileList = new ArrayList<File>();

	private static final int USB_COMMAND_SEND_ZIP = 0x13;
	private static final int USB_COMMAND_RECEIVE_ZIP = 0x14;
	private static final String secretKey = "fvXGl2OZsZgQfPaFRI1MTpKxNhJED/QZOFW8D3v9bfy8fjgHV43yshJiH51yTH5x";
	private static final String zipFilePath = System.getProperty("user.dir") + "/out/output.zip";
	private static final String sourceFolderPath = System.getProperty("user.dir") + "/out/container";
	private SerialCommunication serialCommunication;

	public List<File> getFileList() {
		// Clear list
		fileList.clear();

		try {
			File folder = new File(sourceFolderPath);
			for (File file : folder.listFiles()) {
				fileList.add(file);
			}

		} catch (Exception e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
		}
		return this.fileList;
	}

	public ZipContainer() {
		// TODO Auto-generated constructor stub
		fileList.clear();
	}

	public void addFile() {
		// TODO Auto-generated constructor stub
	}

	public void send() throws Exception {

		// Generate SecretKeySpec from the secret key
		SecretKeySpec secretKeySpec = new SecretKeySpec(secretKey.getBytes(), "AES");

		// Create Cipher instance
		Cipher cipher = Cipher.getInstance("AES");

		// Initialize Cipher for encryption
		cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);

		// Create output stream for zip file
		try (ZipOutputStream zipOutputStream = new ZipOutputStream(new FileOutputStream(zipFilePath));
				CipherOutputStream cipherOutputStream = new CipherOutputStream(zipOutputStream, cipher)) {

			// Setting compression level (0-9) 9-best
			zipOutputStream.setLevel(9);

			// Add files to zip
			addFilesToZip(sourceFolderPath, "", zipOutputStream);

			try {
				// Send zip file via serial communication
				File file = new File(zipFilePath);
				serialCommunication.send(file);

				// Delete zip file after send
				file.delete();

			} catch (Exception e) {
				e.printStackTrace();
				consoleOutput(e.getMessage());
			}

		} catch (IOException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());

		}
	}

	public void receive() {
		// TODO Auto-generated constructor stub
	}

	public void removeFile() {
		// TODO Auto-generated constructor stub
	}

	public static void addFilesToZip(String folderPath, String currentPath, ZipOutputStream zipOutputStream)
			throws IOException {
		File folder = new File(folderPath);
		for (File file : folder.listFiles()) {
			if (file.isDirectory()) {
				addFilesToZip(file.getAbsolutePath(), currentPath + file.getName() + "/", zipOutputStream);
			} else {
				try (FileInputStream fis = new FileInputStream(file)) {
					ZipEntry entry = new ZipEntry(currentPath + file.getName());
					zipOutputStream.putNextEntry(entry);

					byte[] buffer = new byte[1024];
					int len;
					while ((len = fis.read(buffer)) > 0) {
						zipOutputStream.write(buffer, 0, len);
					}

					zipOutputStream.closeEntry();
				}
			}
		}
	}

	private void consoleOutput(String msg) {
//		lastConsoleOutput = msg;
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
