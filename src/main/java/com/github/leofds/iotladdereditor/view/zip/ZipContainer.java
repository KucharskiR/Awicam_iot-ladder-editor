package com.github.leofds.iotladdereditor.view.zip;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;

import com.github.leofds.iotladdereditor.application.Mediator;

public class ZipContainer {
	private List<File> fileList = new ArrayList<File>();

//	private static final int USB_COMMAND_SEND_ZIP = 0x13;      TO DELETE
//	private static final int USB_COMMAND_RECEIVE_ZIP = 0x14;  TO DELETE
	private static final String password = "testawicam1234";
	private static final String passwordIv = "testawicam1234";
	private static final String salt = "test";
//	private static final String secretKey = "fNXDLNHs3/C~NI)b%udh^V-vChFt]XD?";	     TO DELETE
	private static final String zipFilePath = System.getProperty("user.dir") + "/out/tempContainer.zip";
	private static final String sourceFolderPath = System.getProperty("user.dir") + "/out/container";
	private File zipFile;
	
	private static final IvParameterSpec iv = generateIv();


	public ZipContainer() {
		// TODO Auto-generated constructor stub
		fileList.clear();
		
		File folder = new File(sourceFolderPath);
		if(!folder.exists())
			folder.mkdirs();
	}
	
	public static void main(String[] args){
		File file = new File(System.getProperty("user.dir") + "/ladder.pref");
		ZipContainer container = new ZipContainer();
		
		try {
			container.addFile(file);
			container.pack();
			System.out.println("Waiting");
			Thread.sleep(5000);
			System.out.println("End waiting");
			container.unpack();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
	    System.out.print("Hello World");
	  }
	

	public List<File> getFileList() {
		// Clear list
		fileList.clear();
		
		try {
			File folder = new File(sourceFolderPath);
			
			if(!folder.exists())
				folder.mkdir();
			
			for (File file : folder.listFiles()) {
				fileList.add(file);
			}
			
		} catch (Exception e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());
		}
		return this.fileList;
	}
	
	public void addFile(File file) throws Exception {
		 Path sourcePath = Paths.get(file.getAbsolutePath());
		 Path destinationPath = Paths.get(sourceFolderPath + "\\" + file.getName());
		 Files.copy(sourcePath, destinationPath, StandardCopyOption.REPLACE_EXISTING);
	}

	public void pack() throws Exception {

//		// Generate SecretKeySpec from the secret key
//		SecretKeySpec secretKeySpec = new SecretKeySpec(secretKey.getBytes(), "AES");   TO DELETE

		// Create Cipher instance
		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");

		// Initialize Cipher for encryption
		cipher.init(Cipher.ENCRYPT_MODE, getKeyFromPassword(password, salt), iv);

		// Create output stream for zip file
		 try (ZipOutputStream zipOutputStream = new ZipOutputStream(new CipherOutputStream(new FileOutputStream(zipFilePath), cipher))) {

			// Setting compression level (0-9) 9-best
			zipOutputStream.setLevel(9);

			// Add files to zip
			addFilesToZip(sourceFolderPath, "", zipOutputStream);
			
			File zipFile = new File(zipFilePath);
			
			// Check zip file is created
			if (zipFile.exists()) {
				this.zipFile = zipFile;
			}

		} catch (IOException e) {
			e.printStackTrace();
			consoleOutput(e.getMessage());

		}
	}
	
	public void deleteZipFile() {
		this.zipFile.delete();
	}
	
	private void unpack() throws Exception {
		// Create Cipher instance
		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");

		// Initialize Cipher for decryption
		cipher.init(Cipher.DECRYPT_MODE, getKeyFromPassword(password, salt), iv);

		try (ZipInputStream zipInputStream = new ZipInputStream(
				new CipherInputStream(new FileInputStream(zipFilePath), cipher))) {

			// Create output directory if it doesn't exist
			File outputDir = new File(sourceFolderPath);
			if (!outputDir.exists()) {
				outputDir.mkdirs();
			}

			// Extract files from zip
			extractFilesFromZip(zipInputStream, outputDir);
			

		} catch (IOException e) {
			e.printStackTrace();
		}

	}
	
	public File getZipFile() {
		File file = new File(zipFilePath);    
		return file;
	}

	public static void extractFilesFromZip(ZipInputStream zipInputStream, File outputDirectory) throws IOException {
		byte[] buffer = new byte[1024];
		ZipEntry entry = zipInputStream.getNextEntry();

		while (entry != null) {
			String fileName = entry.getName();
			File outputFile = new File(outputDirectory, fileName);
			System.out.println(outputFile.toString());
			
			System.out.println(fileName);

			// Ensure the parent directories exist
			File parentDir = outputFile.getParentFile();
			if (!parentDir.exists()) {
				parentDir.mkdirs();
			}

			try (FileOutputStream fos = new FileOutputStream(outputFile)) {
				int len;
				while ((len = zipInputStream.read(buffer)) > 0) {
					fos.write(buffer, 0, len);
				}
			}

			zipInputStream.closeEntry();
			entry = zipInputStream.getNextEntry();
		}
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
	
	public static SecretKey getKeyFromPassword(String password, String salt)
		    throws NoSuchAlgorithmException, InvalidKeySpecException {
		    
		    SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
		    KeySpec spec = new PBEKeySpec(password.toCharArray(), salt.getBytes(), 65536, 256);
		    SecretKey secret = new SecretKeySpec(factory.generateSecret(spec).getEncoded(), "AES");
		    return secret;
		}
	
		public static IvParameterSpec generateIv() {
			   // Convert the string to bytes
	        byte[] byteArray = passwordIv.getBytes();
	        
	        // Ensure the byte array has a length of 16
	        byte[] iv = new byte[16];
	        System.arraycopy(byteArray, 0, iv, 0, Math.min(byteArray.length, 16));
	        
			return new IvParameterSpec(iv);
		}

	private void consoleOutput(String msg) {
//		lastConsoleOutput = msg;
		Mediator.getInstance().outputConsoleMessage(msg);
	}

}
