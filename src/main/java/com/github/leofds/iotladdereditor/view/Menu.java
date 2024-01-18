/*******************************************************************************
 * Copyright (C) 2021 Leonardo Fernandes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
package com.github.leofds.iotladdereditor.view;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.util.Locale;
import java.util.ResourceBundle;

import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.KeyStroke;
import javax.swing.border.EmptyBorder;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.application.Preferences;
import com.github.leofds.iotladdereditor.device.DeviceFactory;
import com.github.leofds.iotladdereditor.example.Example;
import com.github.leofds.iotladdereditor.example.ExampleFactory;
import com.github.leofds.iotladdereditor.i18n.Strings;
import com.github.leofds.iotladdereditor.ladder.LadderProgram;
import com.github.leofds.iotladdereditor.util.FileUtils;
import com.github.leofds.iotladdereditor.view.event.EspUpdate;
import com.github.leofds.iotladdereditor.view.event.Subject;
import com.github.leofds.iotladdereditor.view.event.Subject.SubMsg;

public class Menu extends JMenuBar {

	private static final long serialVersionUID = 1L;

	private Mediator me = Mediator.getInstance();
	private JMenu helpLang;

	public Menu() {
		JMenu menuFile = new JMenu(Strings.file());
		JMenuItem fileOpen = new JMenuItem(Strings.open());
		JMenuItem fileNew = new JMenuItem(Strings.neww());
		JMenuItem fileSave = new JMenuItem(Strings.save());
		JMenuItem fileSaveAs = new JMenuItem(Strings.saveas());
		JMenuItem fileExit = new JMenuItem(Strings.exit());
		JMenu menuProject = new JMenu(Strings.project());
		JMenuItem projectBuild = new JMenuItem(Strings.build());
		JMenuItem projectBuildRun = new JMenuItem(Strings.buildRun());
		JMenuItem espUpdate = new JMenuItem(Strings.espUpdateMenuTitle());
		JMenuItem projectChoose = new JMenuItem(Strings.chooseController());
		JMenuItem importExportLadderProgram = new JMenuItem("Import/Export ladder");
		JMenuItem projectProprerties = new JMenuItem(Strings.properties());
		JMenu menuHelp = new JMenu(Strings.help());
		helpLang = new JMenu(Strings.language());
		
		for (ResourceBundle resourceBundle : Strings.getResourceBundles()) {
			JRadioButtonMenuItem item = new JRadioButtonMenuItem( resourceBundle.getString("lang") );
			item.addActionListener(getLanguageAction(resourceBundle.getLocale()));
			if(Strings.getBundle().equals(resourceBundle)) {
				item.setSelected(true);
			}
			helpLang.add( item );
		}

		JMenuItem helpAbout = new JMenuItem(Strings.about());

		JMenu fileExamples = new JMenu(Strings.examples());
		int cnt = 1;
		for(Example ex: Example.values()) {
			JMenuItem item = new JMenuItem(cnt++ + ". " + ex.name);
			item.addActionListener(getExampleAction(ex));
			fileExamples.add(item);
		}
		
		fileOpen.addActionListener(getFileOpenAction());
		fileNew.addActionListener(getFileNewAction());
		fileSave.addActionListener(getFileSaveAction());
		fileSaveAs.addActionListener(getFileSaveAsAction());
		fileExit.addActionListener(getFileExitAction());
		projectBuild.addActionListener(getProjectBuild());
		projectBuildRun.addActionListener(getProjectBuildRun());
		espUpdate.addActionListener(getEspUpdateRun());
		projectChoose.addActionListener(getProjectChoose());
		importExportLadderProgram.addActionListener(getImportExportLdFile());
		projectProprerties.addActionListener(getProjectProperties());
		helpAbout.addActionListener(getHelpAbout());

		fileOpen.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_O, InputEvent.CTRL_DOWN_MASK), "evtOpen");
		fileOpen.getActionMap().put("evtOpen", (AbstractAction) getFileOpenAction());
		fileOpen.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, InputEvent.CTRL_DOWN_MASK));

		fileNew.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_N, InputEvent.CTRL_DOWN_MASK), "evtNew");
		fileNew.getActionMap().put("evtNew", (AbstractAction) getFileNewAction());
		fileNew.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, InputEvent.CTRL_DOWN_MASK));

		fileSave.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.CTRL_DOWN_MASK), "evtSave");
		fileSave.getActionMap().put("evtSave", (AbstractAction) getFileSaveAction());
		fileSave.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.CTRL_DOWN_MASK));

		fileSaveAs.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.ALT_MASK | InputEvent.CTRL_DOWN_MASK), "evtSaveAs");
		fileSaveAs.getActionMap().put("evtSaveAs", (AbstractAction) getFileSaveAsAction());
		fileSaveAs.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.ALT_MASK | InputEvent.CTRL_DOWN_MASK));

		fileExit.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_Q, InputEvent.CTRL_DOWN_MASK), "evtExit");
		fileExit.getActionMap().put("evtExit", (AbstractAction) getFileExitAction());
		fileExit.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, InputEvent.CTRL_DOWN_MASK));

		projectBuild.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_B, InputEvent.CTRL_DOWN_MASK), "evtBuild");
		projectBuild.getActionMap().put("evtBuild", (AbstractAction) getProjectBuild());
		projectBuild.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_B, InputEvent.CTRL_DOWN_MASK));

		projectBuildRun.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_R, InputEvent.CTRL_DOWN_MASK), "evtBuildRun");
		projectBuildRun.getActionMap().put("evtBuildRun", (AbstractAction) getProjectBuildRun());
		projectBuildRun.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, InputEvent.CTRL_DOWN_MASK));
		
		helpAbout.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0, true),"evtAbout");
		helpAbout.getActionMap().put("evtAbout", (AbstractAction) getHelpAbout());
		helpAbout.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0, true));

		menuFile.add(fileOpen);
		menuFile.add(fileNew);
		menuFile.addSeparator();
		menuFile.add(fileExamples);
		menuFile.addSeparator();
		menuFile.add(fileSave);
		menuFile.add(fileSaveAs);
		menuFile.addSeparator();
		menuFile.add(fileExit);
		menuProject.add(projectBuild);
		menuProject.add(projectBuildRun);
		menuProject.add(espUpdate);
		menuProject.add(projectChoose);
		menuProject.add(importExportLadderProgram);
		menuProject.addSeparator();
		menuProject.add(projectProprerties);
		menuHelp.add(helpLang);
		menuHelp.add(helpAbout);

		setBorder(new EmptyBorder(0, 0, 0, 0));
		add(menuFile);
		add(menuProject);
		add(menuHelp);
	}

	private AbstractAction getImportExportLdFile() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;

			@Override
			public void actionPerformed(ActionEvent e) {
				// TODO odczytanie/wysłanie pliku z/do ESP a następnie zapisanie do pliku
//				FileUtils.saveAsLadderProgramFromDevice();
				BuildAndRun importExportObj = new BuildAndRun();
				
			}
		};
	}

	private AbstractAction getFileOpenAction() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			@Override
			public void actionPerformed(ActionEvent e) {
				FileUtils.openLadderProgram();
				me.updateDevice(new DeviceFactory().getDevice());
			}
		};
	}

	private AbstractAction getFileNewAction() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			@Override
			public void actionPerformed(ActionEvent e) {
				FileUtils.newLadderProgram();
			}
		};
	}

	private AbstractAction getFileSaveAction() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			@Override
			public void actionPerformed(ActionEvent e) {
				FileUtils.saveLadderProgram();
			}
		};
	}

	private AbstractAction getFileSaveAsAction() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			@Override
			public void actionPerformed(ActionEvent e) {
				FileUtils.saveAsLadderProgram();
			}
		};
	}

	private AbstractAction getFileExitAction() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			@Override
			public void actionPerformed(ActionEvent e) {
				me.closeAplication();
			}
		};
	}

	private AbstractAction getProjectProperties() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			
			@Override
			public void actionPerformed(ActionEvent e) {
				ProjectProperties dialog = new ProjectProperties();
				dialog.setModal(true);
				dialog.setResizable(false);
				dialog.pack();
				dialog.setLocationRelativeTo(null);
				dialog.setVisible(true);
			}
		};
	}
	private AbstractAction getProjectChoose() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			
			@Override
			public void actionPerformed(ActionEvent e) {
				InitSetup dialog = new InitSetup();
				dialog.setModal(true);
				dialog.setResizable(false);
				dialog.pack();
				dialog.setLocationRelativeTo(null);
				dialog.setVisible(true);
			}
		};
	}
	
	private AbstractAction getProjectBuild() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;

			@Override
			public void actionPerformed(ActionEvent e) {
				Subject.getInstance().notifyChange(SubMsg.BUILD);
			}
		};
	}
	
	private AbstractAction getProjectBuildRun() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;

			@Override
			public void actionPerformed(ActionEvent e) {
				Subject.getInstance().notifyChange(SubMsg.BUILD_RUN);
			}
		};
	}
	
	private AbstractAction getEspUpdateRun() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			
			@Override
			public void actionPerformed(ActionEvent e) {
				
				EspUpdate updateBox = new EspUpdate();
				
				updateBox.setResizable(false);
				updateBox.pack();
				updateBox.setLocationRelativeTo(null);
				updateBox.setVisible(true);			}
		};
	}

	private AbstractAction getHelpAbout() {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			
			@Override
			public void actionPerformed(ActionEvent e) {
				About about = new About();
				about.setModal(true);
				about.setResizable(false);
				about.pack();
				about.setLocationRelativeTo(null);
				about.setVisible(true);
			}
		};
	}

	private AbstractAction getLanguageAction(Locale locale) {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;
			
			@Override
			public void actionPerformed(ActionEvent e) {
				JMenuItem source = (JMenuItem) e.getSource();
				for(int i=0;i<helpLang.getItemCount();i++) {
					JRadioButtonMenuItem item = (JRadioButtonMenuItem) helpLang.getItem(i);
					item.setSelected( item.equals(source) );
				}
				Strings.changeLocale(locale);
				Preferences.put( Preferences.LANG , locale.getLanguage()+"_"+locale.getCountry());
				JOptionPane.showMessageDialog(null, Strings.langChangeMsg());
			}
		};
	}
	
	private AbstractAction getExampleAction(final Example ex) {
		return new AbstractAction() {
			private static final long serialVersionUID = 1L;

			@Override
			public void actionPerformed(ActionEvent e) {
				LadderProgram ladderProgram = ExampleFactory.createExample(ex);
				if(ladderProgram != null) {
					FileUtils.openExample(ex, ladderProgram);
				}
			}
		};
	}
}
