package com.github.leofds.iotladdereditor.view;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Stack;

import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListCellRenderer;
import javax.swing.ListSelectionModel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.filechooser.FileNameExtensionFilter;

public class FileListApp2 {

	public FileListApp2() {
		this.createAndShowGUI();
	}

	private JFrame frame;
	private JList<String> fileList;
	private DefaultListModel<String> listModel;
	private HashMap<String, File> filesList;
	private Stack<StackEl> undoStack;

//    /**
//     * @wbp.parser.entryPoint
//     */
	public static void main(String[] args) {
		SwingUtilities.invokeLater(() -> {
			try {
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			} catch (Exception e) {
				e.printStackTrace();
			}
			new FileListApp2();
		});
	}

	   private void createAndShowGUI() {
	        frame = new JFrame("File container in device memory");
	        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	        frame.setSize(400, 300);

	        JPanel mainPanel = new JPanel(new BorderLayout());

	        // Przyciski u góry
	        JPanel topButtonPanel = new JPanel();
	        JButton addButton = new JButton("Add File");
	        JButton undoButton = new JButton("Undo");

	        topButtonPanel.add(addButton);
	        topButtonPanel.add(undoButton);
	        mainPanel.add(topButtonPanel, BorderLayout.NORTH);

	        // Przyciski na dole
	        JPanel bottomButtonPanel = new JPanel();
	        JButton readButton = new JButton("Read");
	        JButton saveButton = new JButton("Save");
	        JButton cancelButton = new JButton("Cancel");

	        bottomButtonPanel.add(readButton);
	        bottomButtonPanel.add(saveButton);
	        bottomButtonPanel.add(cancelButton);
	        mainPanel.add(bottomButtonPanel, BorderLayout.SOUTH);

	        // Utwórz model danych dla JList
	        listModel = new DefaultListModel<>();
	        fileList = new JList<>(listModel);
	        fileList.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);

	        // Dodaj obsługę przycisku 'Remove'
	        fileList.setCellRenderer(new FileListRenderer());
	        fileList.addMouseListener(new FileListMouseListener());

	        JScrollPane scrollPane = new JScrollPane(fileList);
	        mainPanel.add(scrollPane, BorderLayout.CENTER);

	        // Dodaj przykładowe pliki
	        addSampleFiles();

	        // Inicjalizacja stosu do cofania
	        undoStack = new Stack<>();

	        frame.getContentPane().add(mainPanel);
	        frame.setLocationRelativeTo(null);
	        frame.setVisible(true);

	        addButton.addActionListener(new ActionListener() {
	            @Override
	            public void actionPerformed(ActionEvent e) {
	                addFile();
	            }
	        });

	        undoButton.addActionListener(new ActionListener() {
	            @Override
	            public void actionPerformed(ActionEvent e) {
	                undo();
	            }
	        });

	        readButton.addActionListener(new ActionListener() {
	            @Override
	            public void actionPerformed(ActionEvent e) {
	                updateFileList();
	            }
	        });
	    }

	    private void addSampleFiles() {
	        // Dodaj przykładowe pliki do listy
			String path = System.getProperty("user.dir");
			File file = new File(path, "ladder.pref");
			
			listModel.addElement(file.getName());
			filesList.put(file.getName(), file);
	    }

	    private void addFile() {
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
	            listModel.addElement(fileName);
	            filesList.put(fileName, selectedFile);
	            undoStack.push(new StackEl(fileName, 0, selectedFile));
	        }
	    }

		private void undo() {
			if (!undoStack.isEmpty()) {
				// Get last stack element
				StackEl lastStack = undoStack.pop();
				
				// Choose operation according to stack element type
				switch (lastStack.getType()) {
				case 0:
					// Usuń plik z listy
					listModel.removeElement(lastStack.getName());
					filesList.remove(lastStack.getName());
					break;
				case 1:
					// Dodaj plik do listy
					listModel.addElement(lastStack.getName());
					filesList.put(lastStack.getName(), null);
					break;

				default:
					break;
				}

			}
		}

	    private void updateFileList() {
	        StringBuilder selectedFiles = new StringBuilder("Selected Files:\n");

	        int[] selectedIndices = fileList.getSelectedIndices();
	        for (int index : selectedIndices) {
	            selectedFiles.append(listModel.getElementAt(index)).append("\n");
	        }

	        JOptionPane.showMessageDialog(frame, selectedFiles.toString());
	    }

	    // Klasa do dostosowania renderowania komórek listy (dodanie przycisku 'Remove')
	    class FileListRenderer extends JButton implements ListCellRenderer<String> {
	        /**
			 * 
			 */
			private static final long serialVersionUID = 1L;

			@Override
	        public Component getListCellRendererComponent(JList<? extends String> list, String value, int index, boolean isSelected, boolean cellHasFocus) {
	            setText(value);
	            return this;
	        }
	    }

	    // Klasa do obsługi kliknięć myszy w liście (dodanie akcji dla przycisku 'Remove')
	    class FileListMouseListener extends MouseAdapter {
	        @Override
	        public void mouseClicked(MouseEvent e) {
	        	JFrame remFrame = new JFrame();
	            int index = fileList.locationToIndex(e.getPoint());
	            if (index != -1) {
	                String selectedFile = listModel.getElementAt(index);

	                JButton removeButton = new JButton("Remove");
	                removeButton.addActionListener(new ActionListener() {
	                    @Override
	                    public void actionPerformed(ActionEvent e) {
	                        // Usuń plik po naciśnięciu przycisku 'Remove'
	                        listModel.removeElement(selectedFile);
	                        undoStack.push(new StackEl(selectedFile, 1, filesList.get(selectedFile)));
	                        filesList.remove(selectedFile);
	                        remFrame.dispose();
	                    }
	                });

	                // Wyświetl okno dialogowe z przyciskiem 'Remove'
	                JOptionPane.showMessageDialog(remFrame, removeButton, "Remove File", JOptionPane.PLAIN_MESSAGE);
	            }
	        }
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
