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

public enum CodeOptions{

	ESP32_ARDUINO_FREERTOS("ESP32 Arduino [.ino]");
	
	String value;
	
	private CodeOptions(String value) {
		this.value = value;
	}
	
	public static CodeOptions getByName(String name) {
		for(CodeOptions code: CodeOptions.values()) {
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
