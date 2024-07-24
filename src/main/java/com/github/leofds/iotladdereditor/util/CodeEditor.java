package com.github.leofds.iotladdereditor.util;

import java.io.File;
import java.io.IOException;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.SourceCode;
import com.github.leofds.iotladdereditor.i18n.Strings;

public class CodeEditor {
  
  // private File file;
  private String path;

  public CodeEditor(String path) {
    // this.path = String.format("out/%s", path);
    this.path = path;
    Mediator.getInstance().outputConsoleMessage("Opening file: " + path);

    File file = new File(this.path);
    if(!file.exists()) {
      SourceCode code = new SourceCode();
      code.createNewFile(this.path);
      code.addl(".global asmfunc");
		  code.addl(".text");
		  code.addl("asmfunc:");
		  code.addl("# read inputs from reg a0, a1");
		  code.addl("# code");
		  code.addl("# to reg a0 and a1 write outputs");
		  code.addl("ret");
      try {
				  FileUtils.createFile(this.path, code.getFiles().get(0).getContent());
      } catch (IOException e) {
				Mediator.getInstance().outputConsoleMessage(Strings.failToCreateFile());
			}
    }
  }

  public void openInExternalEditor() {
    try {
      java.awt.Desktop.getDesktop().edit(new File(this.path));
    } catch (IOException e) {
			Mediator.getInstance().outputConsoleMessage(Strings.failToCreateFile());
		}
  }
}