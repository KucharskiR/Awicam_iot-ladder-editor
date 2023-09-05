package com.github.leofds.iotladdereditor.util;

import java.awt.BorderLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

import org.apache.commons.lang3.StringUtils;

import com.github.leofds.iotladdereditor.i18n.Strings;

public class WaitingBar  {
    private JLabel statusLabel;
    private Timer timer;
    private JFrame frame;
    private int dotCount = 1;

	public WaitingBar() {
        this.frame = new JFrame("Progress/Waiting Bar");
        frame.setTitle(title());;
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setSize(300, 113);
        frame.setLocationRelativeTo(null);
        frame.getContentPane().setLayout(new BorderLayout(0, 0));

        statusLabel = new JLabel(loadingText());
        statusLabel.setFont(new Font("Tahoma", Font.PLAIN, 14));
        statusLabel.setHorizontalAlignment(SwingConstants.CENTER);
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
	

	public JFrame getFrame() {
		return frame;
	}

    public String title() {
		return "Process";
	}

	public String loadingText() {
		return "Loading";
	}

	private void updateStatusLabel() {
        dotCount = (dotCount + 1) % 4; // Cycling through 0, 1, 2, 3
//        String dots = ".".repeat(dotCount);
        String dots = StringUtils.repeat(".", dotCount);

        statusLabel.setText(loadingText() + dots);
        statusLabel.setFont(new Font("Tahoma", Font.PLAIN, 13));
    }

	public void close() {
		frame.getContentPane().removeAll();
		
		SwingUtilities.invokeLater(() -> {
			 JOptionPane.showMessageDialog(frame, "Compilation completed!", Strings.information(),
					JOptionPane.INFORMATION_MESSAGE);
				frame.dispose();
		});
	}

}
