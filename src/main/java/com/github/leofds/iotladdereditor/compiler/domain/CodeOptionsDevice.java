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
package com.github.leofds.iotladdereditor.compiler.domain;

public enum CodeOptionsDevice{

	W1VC_64R("W1VC 64R"),
	W1VC_128R("W1VC 128R");
	
	String value;
	
	private CodeOptionsDevice(String value) {
		this.value = value;
	}
	
	public static CodeOptionsDevice getByName(String name) {
		for(CodeOptionsDevice code: CodeOptionsDevice.values()) {
			if(code.name().equals(name)) {
				return code;
			}
		}
		return null;
	}
	
	@Override
	public String toString() {
		return value;
	}
}
