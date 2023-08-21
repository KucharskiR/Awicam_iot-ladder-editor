package com.github.leofds.iotladdereditor.util.bars;

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

}
