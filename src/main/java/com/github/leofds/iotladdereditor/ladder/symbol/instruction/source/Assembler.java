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
package com.github.leofds.iotladdereditor.ladder.symbol.instruction.source;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.util.ArrayList;
import java.util.List;

import com.github.leofds.iotladdereditor.compiler.domain.GenContext;
import com.github.leofds.iotladdereditor.compiler.domain.Kind;
import com.github.leofds.iotladdereditor.compiler.domain.ProgramFunc;
import com.github.leofds.iotladdereditor.compiler.domain.Quadruple;
import com.github.leofds.iotladdereditor.compiler.domain.Symbol;
import com.github.leofds.iotladdereditor.compiler.domain.SymbolTable;
import com.github.leofds.iotladdereditor.compiler.exception.SemanticErrorException;
import com.github.leofds.iotladdereditor.compiler.exception.SemanticWarnigException;
import com.github.leofds.iotladdereditor.compiler.generator.factory.QuadrupleFactory;
import com.github.leofds.iotladdereditor.device.Device;
import com.github.leofds.iotladdereditor.device.DeviceMemory;
import com.github.leofds.iotladdereditor.ladder.view.DialogScreen;

public class Assembler extends SourceInstruction{

	private static final long serialVersionUID = 1L;
	private static final String sourceType = "asm";
	
	private String fileName;;
	
	public Assembler() {
		super();
		setLabel("ASSEMBLER");
		fileName = "";
	}
	
	// TODO: change this method to return fileName
	public String getFileName() {
		return fileName;
	}

	public void setSeconds(String seconds) {
		this.fileName = seconds;
	}

	@Override
	public void paint(Graphics2D g2d){
		super.paint(g2d);
		
		g2d.setColor(new Color(0, 0, 255));
		g2d.setFont(new Font("Arial", Font.PLAIN, 12));
//		g2d.drawString("File", (blockWidth)/4, blockHeight*getHeight()/1.5f);
		int len = g2d.getFontMetrics().stringWidth("ASSEMBLER");
		g2d.drawString("SOURCE", (blockWidth*getWidth()-len)/2, blockHeight*getHeight()/4);
		len = g2d.getFontMetrics().stringWidth(getLabel());
		g2d.drawString(getLabel(), (blockWidth*getWidth()-len)/2, blockHeight*getHeight()/2.8f);
		
		
		// TODO: show file name here
		g2d.setColor(new Color(0, 0, 0));
		len = g2d.getFontMetrics().stringWidth(""+getFileName());
		g2d.drawString(""+getFileName(), blockWidth*getWidth()-len-15,  blockHeight*getHeight()/1.5f);
	}

	@Override
	public List<Quadruple> generateIRInit(GenContext context) {
		List<Quadruple> quadruples = new ArrayList<Quadruple>();
		return quadruples;
	}
	
	@Override
	public List<Quadruple> generateIR(GenContext context) {
		SymbolTable symbolTable = context.getSymbolTable();
		Symbol symbSourceFilePath = new Symbol(sourceType, Kind.SOURCE, getSourceFilePath());
		Symbol status		= symbolTable.addBoolVar(context.getCurrentStatus(), context.getScope());
		Symbol label 		= symbolTable.addLabel(context.genLabel(), context.getScope());

		List<Quadruple> quadruples = new ArrayList<Quadruple>();
		quadruples.add(QuadrupleFactory.createIfFalse(status, label));
		quadruples.add(QuadrupleFactory.createSource(symbSourceFilePath));
		quadruples.add(QuadrupleFactory.createLabel(label));
		return quadruples;
	}

	@Override
	public void analyze() throws SemanticErrorException, SemanticWarnigException {
	}

	@Override
	public boolean addMemory(DeviceMemory memory, int x, int y) {
		return false;
	}

	@Override
	public void updateDevice(Device device) {
	}

//	@Override
//	public void afterShowScreen(DialogScreen dialog) {
//		// TODO Auto-generated method stub
//		
//	}
}
