package com.github.leofds.iotladdereditor.view;

import java.awt.event.ActionEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.DefaultTableModel;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.util.SerialCommunication;
import com.github.leofds.iotladdereditor.util.zip.ZipContainer;

public class FileListApp extends javax.swing.JFrame {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private ZipContainer container = new ZipContainer();

	public FileListApp() {
		initComponents();
		// Scan container dir
		scanContainerDir();
		// Inicjalizacja stosu do cofania
        stack = new Stack<>();
	}

	private JFrame frame;
	private String pathToContainerDir = System.getProperty("user.dir") + "\\out\\container";
	private List<String> tableListFiles = new ArrayList<String>();
//	private DefaultListModel<String> listModel = new DefaultListModel<String>();
	private HashMap<String, File> filesList = new HashMap<String, File>();
	private Stack<StackEl> stack;
	private static String comPort = "COM3";
	private static SerialCommunication connection = new SerialCommunication();

//    /**   TODO: usunąć main gdy nie potrzebne w przyszłych wersjach
//     * @wbp.parser.entryPoint
//     */
//	public static void main(String[] args) {
//		SwingUtilities.invokeLater(() -> {
//			try {
//				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
//			} catch (Exception e) {
//				e.printStackTrace();
//			}
//			
//			try {
//				connection.start(comPort, 9600);
//			} catch (Exception e) {
//				System.out.println(e.getMessage());
//			}
//			new FileListApp().setVisible(true);
//		});
//	}
	
    // Variables declaration - do not modify                     
    private javax.swing.JButton buttonAddFile;
    private javax.swing.JButton buttonClose;
    private javax.swing.JButton buttonDelete;
    private javax.swing.JButton buttonExport;
    private javax.swing.JButton buttonImport;
    private javax.swing.JButton buttonUndo;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable jTable1;
    // End of variables declaration   

    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">                          
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane();
        jTable1 = new javax.swing.JTable();
        buttonExport = new javax.swing.JButton();
        buttonAddFile = new javax.swing.JButton();
        buttonDelete = new javax.swing.JButton();
        buttonImport = new javax.swing.JButton();
        buttonUndo = new javax.swing.JButton();
        buttonClose = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setLocationRelativeTo(null);

		jTable1.setModel(new javax.swing.table.DefaultTableModel(new Object[][] {

		}, new String[] { "Id", "File name", "Data add" }) {
			/**
			 * 
			 */
			private static final long serialVersionUID = 1L;
			@SuppressWarnings("rawtypes")
			Class[] types = new Class[] { java.lang.String.class, java.lang.String.class, java.lang.Object.class };
			boolean[] canEdit = new boolean[] { false, false, false };

			@SuppressWarnings("rawtypes")
			public Class getColumnClass(int columnIndex) {
				return types[columnIndex];
			}

			public boolean isCellEditable(int rowIndex, int columnIndex) {
				return canEdit[columnIndex];
			}
		});
        
		jScrollPane1.setViewportView(jTable1);
		if (jTable1.getColumnModel().getColumnCount() > 0) {
			jTable1.getColumnModel().getColumn(0).setMinWidth(25);
			jTable1.getColumnModel().getColumn(0).setPreferredWidth(25);
			jTable1.getColumnModel().getColumn(0).setMaxWidth(40);
		}

        buttonExport.setFont(new java.awt.Font("Segoe UI", 1, 14)); // NOI18N
        buttonExport.setText("Export Files to Device");
        buttonExport.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonExportActionPerformed(evt);
            }
        });

        buttonAddFile.setText("Add file");
        buttonAddFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonAddFileActionPerformed(evt);
            }
        });

        buttonDelete.setText("Delete");
        buttonDelete.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonDeleteActionPerformed(evt);
            }
        });

        buttonImport.setFont(new java.awt.Font("Segoe UI", 1, 14)); // NOI18N
        buttonImport.setText("Import Files from Device");
        buttonImport.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
            	buttonImportActionPerformed(evt);
            }
        });
      
        buttonUndo.setText("Undo");
        buttonUndo.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonUndoActionPerformed(evt);
            }
        });

        buttonClose.setText("Close");
        buttonClose.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonCloseActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                .addGap(29, 29, 29)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(buttonAddFile)
                    .addComponent(buttonDelete)
                    .addComponent(buttonUndo))
                .addGap(19, 19, 19))
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGap(48, 48, 48)
                        .addComponent(buttonExport, javax.swing.GroupLayout.PREFERRED_SIZE, 242, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(buttonClose))
                    .addGroup(layout.createSequentialGroup()
                        .addGap(49, 49, 49)
                        .addComponent(buttonImport, javax.swing.GroupLayout.PREFERRED_SIZE, 242, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(109, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addGap(21, 21, 21)
                .addComponent(buttonImport)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 34, Short.MAX_VALUE)
                        .addComponent(buttonAddFile)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(buttonDelete)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(buttonUndo)
                        .addGap(154, 154, 154))
                    .addGroup(layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)))
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.CENTER)
                    .addComponent(buttonExport)
                    .addComponent(buttonClose))
                .addGap(9, 9, 9))
        );
        
        pack();
    }// </editor-fold>           
    
    // Rendering table
    private void renderTable() {
    	// Read table model
    	DefaultTableModel model = (DefaultTableModel)jTable1.getModel();
    	
    	// Clear list
    	model.setRowCount(0);
    	int i = 0;
    	for (String str : tableListFiles) {
    		model.addRow(new Object[] {Integer.valueOf(++i), str, ""});
		}
    }
    
    // Scan files in container dir
    private void scanContainerDir() {
    	String path = pathToContainerDir;
    	
    	File containerDir = new File(path);
    	File[] filesInDir = containerDir.listFiles();
    	
    	// Clear lists
    	tableListFiles.clear();
    	filesList.clear();
    	
    	if (filesInDir != null) {
    		for (File file : filesInDir) {
    			System.out.println(file.getName());
    			tableListFiles.add(file.getName());
    			filesList.put(file.getName(), file);
    		}
    	} else {
    		System.out.println("Empty dir");
    	}
    	renderTable();
    }
    
    // Scan files in container dir
    private void scanAndDeleteFilesInContainerDir() {
    	String path = pathToContainerDir;
    	
    	File containerDir = new File(path);
    	File[] filesInDir = containerDir.listFiles();
    	
    	if (filesInDir != null) {
    		for (File file : filesInDir) {
    			System.out.println(file.getName());
    			file.delete();
    		}
    	} else {
    		System.out.println("Empty dir");
    	}
    	
    	// File from path to Container.zip
    	File deleteContainer = new File(ZipContainer.getZipfilepath());
    	
    	// If Container.zip exists delete it
    	if (deleteContainer.exists())
    		deleteContainer.delete();
    	
    	// Refresh table
    	renderTable();
    }
    
	// Buttons actions
	private void buttonExportActionPerformed(ActionEvent evt) {
		try {
			// Loop over HashMap list with files
			for (Map.Entry<String, File> entry : filesList.entrySet()) {
				container.addFile(entry.getValue());
			}
			// Pack container to .zip with AES
			container.pack();

		} catch (Exception e) {
			e.printStackTrace();
		}
		
		// Send to the device
		if (connection.send(container.getZipFile()) != -1) {
			// If file sent successfully clear container dir from files and delete Container.zip
			scanAndDeleteFilesInContainerDir();
			consoleOutput("Ladder info-> File sent to the device!");
		} else {
			consoleOutput("Ladder info-> Sending file error. Please try again");
		}

	}
	
	private void buttonImportActionPerformed(ActionEvent evt) {
		// Scan and delete files in container dir first
//		scanAndDeleteFilesInContainerDir();
		scanContainerDir();
		
		// Check the Container.zip really exists
		if (!container.getZipFile().exists()) {
			File createdContainer = new File(ZipContainer.getZipfilepath());
			// Receive .zip file from the device
			connection.receive(createdContainer);
		} else {
			// Receive .zip file from the device
			connection.receive(container.getZipFile());
		}
		
		// Try to unpack Container.zip
		try {
			// Unpack
			container.unpack();
			// Scan container dir files
			scanContainerDir();
		} catch (Exception e) {
			System.out.println(e.getMessage());
			e.printStackTrace();
		}
	}
    
	private void buttonAddFileActionPerformed(java.awt.event.ActionEvent evt) {
		JFileChooser fileChooser = new JFileChooser();

		// String path to main dir
		String path = System.getProperty("user.dir");

		// Set choose window open path
		File file = new File(path);
		fileChooser.setCurrentDirectory(file);

		FileNameExtensionFilter filter = new FileNameExtensionFilter("Ladder", "ld");
		fileChooser.setFileFilter(filter);

		int result = fileChooser.showOpenDialog(frame);

		if (result == JFileChooser.APPROVE_OPTION) {
			File selectedFile = fileChooser.getSelectedFile();
			String fileName = selectedFile.getName();

			// Dodaj plik do listy i zapisz na stosie do cofania
//			listModel.addElement(fileName);
			tableListFiles.add(selectedFile.getName());
			filesList.put(fileName, selectedFile);
			stack.push(new StackEl(fileName, 0, selectedFile));
		}
		renderTable();
	}                                            

	private void buttonDeleteActionPerformed(java.awt.event.ActionEvent evt) {
		
		// Validate is any row selected
		if (!jTable1.getSelectionModel().isSelectionEmpty()) {
			// Get filename from selected row and second column (index column =1 )
			String fileName = (String) jTable1.getValueAt(jTable1.getSelectedRow(), 1);
			System.out.println(fileName);
			
			// Check is fileName string object in table list
			if (tableListFiles.contains(fileName))
				// If true delete object
				tableListFiles.remove(fileName);
			
			// Get file from filesList by fileName to add to the stack
			File toStackFile = filesList.get(fileName);
			
			// Add stack element
			stack.add(new StackEl(fileName, 1, toStackFile));
			
			// Remove filneName object from filesList
			filesList.remove(fileName);
			renderTable();
		}
	}

    private void buttonUndoActionPerformed(java.awt.event.ActionEvent evt) {                                           
    	if (!stack.isEmpty()) {
			// Get last stack element
			StackEl lastStack = stack.pop();
			
			// Choose operation according to stack element type
			switch (lastStack.getType()) {
			case 0:
				
				// Remove from fileList
				if (tableListFiles.contains(lastStack.getName()))
					tableListFiles.remove(tableListFiles.indexOf(lastStack.getName()));
				
				filesList.remove(lastStack.getName());
				renderTable();
				break;
			case 1:
				// Dodaj plik do listy
				tableListFiles.add(lastStack.getName());
				filesList.put(lastStack.getName(), null);
				renderTable();
				break;

			default:
				renderTable();
				break;
			}

		}
    }   
    
    private void buttonCloseActionPerformed(java.awt.event.ActionEvent evt) {    
        this.dispose();
    }                                           

    private void consoleOutput(String msg) {
//		lastConsoleOutput = msg;
		System.out.println(msg);
//		TODO: mediator output przywrócić po skończeniu
		Mediator.getInstance().outputConsoleMessage(msg);
	}

	// Stack element class
	class StackEl {
		private String name;
		private int type;
		private File file;

		public StackEl(String name, int type, File file) {
			this.name = name;
			this.type = type;
			this.file = file;
		}

		public String getName() {
			return name;
		}

		public File getFile() {
			return file;
		}

		public int getType() {
			return type;
		}

	}
}
