package com.github.leofds.iotladdereditor.util;

import java.awt.BorderLayout;
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

public class ProgressWaitingBar  {
    private JLabel statusLabel;
    private Timer timer;
    private JFrame frame;
    private int dotCount = 1;

    public ProgressWaitingBar() {
        this.frame = new JFrame("Progress/Waiting Bar Example");;
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setSize(300, 100);
        frame.setLayout(new FlowLayout(FlowLayout.CENTER));

        statusLabel = new JLabel("Loading");
        statusLabel.setHorizontalAlignment(SwingConstants.CENTER);
        frame.add(statusLabel, BorderLayout.CENTER);

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
