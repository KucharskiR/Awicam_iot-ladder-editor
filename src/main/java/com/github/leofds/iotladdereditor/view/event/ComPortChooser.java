package com.github.leofds.iotladdereditor.view.event;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Enumeration;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.github.leofds.iotladdereditor.compiler.Compiler;
import com.github.leofds.iotladdereditor.i18n.Strings;

import gnu.io.CommPortIdentifier;


public class ComPortChooser extends JFrame {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JComboBox<String> comPortComboBox;

    public ComPortChooser() {
        setTitle("COM Port Chooser");
        setSize(300, 150);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        comPortComboBox = new JComboBox<>();
        populateComPortComboBox();
        JButton connectButton = new JButton(Strings.upload());

        connectButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String selectedPort = (String) comPortComboBox.getSelectedItem();
                if (selectedPort != null) {
                    // Perform actions to connect using the selected COM port
                    // For example: Open and configure the serial port
                	Compiler uploadCompiler = new Compiler();
                	uploadCompiler.upload(selectedPort);
                    System.out.println("Connecting to " + selectedPort);
                }
            }
        });

        JPanel panel = new JPanel(new FlowLayout());
        panel.add(new JLabel("Select COM Port:"));
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
