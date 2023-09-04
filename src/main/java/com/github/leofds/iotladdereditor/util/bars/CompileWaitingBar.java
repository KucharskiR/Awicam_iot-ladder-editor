package com.github.leofds.iotladdereditor.util.bars;

import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.util.WaitingBar;

public class CompileWaitingBar extends WaitingBar {

	public CompileWaitingBar() {
		super();
		
	}
	@Override
	public String loadingText() {
		return Strings.pleaseWait();
	}

	@Override
	public String title() {
		return Strings.compilation();
	}
	
	@Override
	public void close() {
		super.getFrame().getContentPane().removeAll();
		
		SwingUtilities.invokeLater(() -> {
			 JOptionPane.showMessageDialog(super.getFrame(), Strings.compilationCompleted(), Strings.information(),
					JOptionPane.INFORMATION_MESSAGE);
				super.getFrame().dispose();
		});
	}

}
