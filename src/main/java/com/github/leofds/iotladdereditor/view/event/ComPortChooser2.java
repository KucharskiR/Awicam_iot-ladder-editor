package com.github.leofds.iotladdereditor.view.event;

import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Enumeration;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.github.leofds.iotladdereditor.i18n.Strings;

import gnu.io.CommPortIdentifier;


public class ComPortChooser2 extends JFrame {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JComboBox<String> comPortComboBox;
	private boolean uploadingStart;
	private String portName;

    public boolean isUploadingStart() {
		return uploadingStart;
	}

	public String getPortName() {
		return portName;
	}

	public ComPortChooser2() {
    	this.uploadingStart = false;
    	this.portName = null;
    	
        setTitle("ESP Upload");
        setSize(300, 100);
        setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        setLocationRelativeTo(null);

        comPortComboBox = new JComboBox<>();
        comPortComboBox.setBounds(128, 11, 55, 21);
        populateComPortComboBox();
        JButton connectButton = new JButton(Strings.upload());
        connectButton.setBounds(193, 10, 67, 23);
        connectButton.setFont(new Font("Tahoma", Font.PLAIN, 12));

        connectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String selectedPort = (String) comPortComboBox.getSelectedItem();
                if (selectedPort != null) {
                    // Perform actions to connect using the selected COM port
                    // For example: Open and configure the serial port
//                	Compiler uploadCompiler = new Compiler();
//                	uploadCompiler.upload(selectedPort);
                	uploadingStart = true;
                    portName = selectedPort;
                    System.out.println("Connecting to " + selectedPort);
                }
            }
        });

        JPanel panel = new JPanel();
        panel.setLayout(null);
        JLabel label = new JLabel("Select COM Port:");
        label.setBounds(29, 13, 89, 15);
        label.setFont(new Font("Tahoma", Font.PLAIN, 12));
        panel.add(label);
        panel.add(comPortComboBox);
        panel.add(connectButton);

        getContentPane().add(panel);
    }

    private void populateComPortComboBox() {
        @SuppressWarnings("unchecked")
		Enumeration<CommPortIdentifier> portEnum = CommPortIdentifier.getPortIdentifiers();
        while (portEnum.hasMoreElements()) {
            CommPortIdentifier portIdentifier = portEnum.nextElement();
            if (portIdentifier.getPortType() == CommPortIdentifier.PORT_SERIAL) {
                comPortComboBox.addItem(portIdentifier.getName());
            }
        }
    }

}
