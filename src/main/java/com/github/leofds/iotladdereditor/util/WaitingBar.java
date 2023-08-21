package com.github.leofds.iotladdereditor.util;

import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

public class WaitingBar  {
    private JLabel statusLabel;
    private Timer timer;
    private JFrame frame;
    private int dotCount = 1;

    public WaitingBar() {
        this.frame = new JFrame("Progress/Waiting Bar");
        frame.setTitle("Process");;
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setSize(254, 113);
        frame.setLayout(new FlowLayout(FlowLayout.CENTER));
        frame.setLocationRelativeTo(null);
        frame.getContentPane().setLayout(null);

        statusLabel = new JLabel("Loading");
        statusLabel.setFont(new Font("Tahoma", Font.PLAIN, 14));
        statusLabel.setBounds(79, 21, 79, 25);
        statusLabel.setHorizontalAlignment(SwingConstants.LEFT);
        frame.getContentPane().add(statusLabel);

        timer = new Timer(500, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                updateStatusLabel();
            }
        });
        timer.start();

        frame.setVisible(true);
    }

    private void updateStatusLabel() {
        dotCount = (dotCount + 1) % 4; // Cycling through 0, 1, 2, 3
        String dots = ".".repeat(dotCount);

        statusLabel.setText("Loading" + dots);
        statusLabel.setFont(new Font("Tahoma", Font.PLAIN, 13));
    }

	public void close() {
		frame.getContentPane().removeAll();
		
		SwingUtilities.invokeLater(() -> {
			 JOptionPane.showMessageDialog(frame, "Compilation completed!", "Information",
					JOptionPane.INFORMATION_MESSAGE);
				frame.dispose();
		});
	}
}
