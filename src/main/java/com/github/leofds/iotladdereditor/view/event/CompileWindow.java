package com.github.leofds.iotladdereditor.view.event;

import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Rectangle;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;

public class CompileWindow extends JFrame {
	private static final long serialVersionUID = 1L;

	public CompileWindow() {
		getContentPane().setComponentOrientation(ComponentOrientation.LEFT_TO_RIGHT);
		getContentPane().setFont(new Font("Tahoma", Font.PLAIN, 12));
		
		setBounds(new Rectangle(0, 0, 10, 10));
        setTitle("Buil and Run");
        setSize(363, 169);
        setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        GridBagLayout gridBagLayout = new GridBagLayout();
        gridBagLayout.columnWidths = new int[]{20, 0, 0, 0, 70, 0, 0};
        gridBagLayout.rowHeights = new int[]{0, 0, 0, 0, 0};
        gridBagLayout.columnWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
        gridBagLayout.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
        getContentPane().setLayout(gridBagLayout);
        
        JButton btnCompile = new JButton("Compile");
        btnCompile.setFont(new Font("Tahoma", Font.PLAIN, 14));
        GridBagConstraints gbc_btnCompile = new GridBagConstraints();
        gbc_btnCompile.insets = new Insets(0, 0, 5, 5);
        gbc_btnCompile.gridx = 1;
        gbc_btnCompile.gridy = 1;
        getContentPane().add(btnCompile, gbc_btnCompile);
        
        JComboBox comboBox = new JComboBox();
        comboBox.setBorder(null);
        comboBox.setEnabled(false);
        comboBox.setMinimumSize(new Dimension(35, 19));
        comboBox.setFont(new Font("Tahoma", Font.PLAIN, 14));
        GridBagConstraints gbc_comboBox = new GridBagConstraints();
        gbc_comboBox.insets = new Insets(0, 0, 5, 5);
        gbc_comboBox.gridx = 4;
        gbc_comboBox.gridy = 1;
        getContentPane().add(comboBox, gbc_comboBox);
        
        JButton btnUpload = new JButton("Upload");
        btnUpload.setEnabled(false);
        btnUpload.setFont(new Font("Tahoma", Font.PLAIN, 14));
        GridBagConstraints gbc_btnUpload = new GridBagConstraints();
        gbc_btnUpload.anchor = GridBagConstraints.EAST;
        gbc_btnUpload.insets = new Insets(0, 0, 5, 0);
        gbc_btnUpload.gridx = 5;
        gbc_btnUpload.gridy = 1;
        getContentPane().add(btnUpload, gbc_btnUpload);
        
        JLabel lblOutputlabel = new JLabel("outputLabel");
        lblOutputlabel.setFont(new Font("Tahoma", Font.PLAIN, 15));
        GridBagConstraints gbc_lblOutputlabel = new GridBagConstraints();
        gbc_lblOutputlabel.gridwidth = 5;
        gbc_lblOutputlabel.insets = new Insets(0, 0, 0, 5);
        gbc_lblOutputlabel.gridx = 1;
        gbc_lblOutputlabel.gridy = 3;
        getContentPane().add(lblOutputlabel, gbc_lblOutputlabel);
        
        setVisible(true);
	}

}
