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
package com.github.leofds.iotladdereditor.device;

import java.util.List;

import com.github.leofds.iotladdereditor.application.Mediator;
import com.github.leofds.iotladdereditor.compiler.domain.CodeOptionsDevice;
import com.github.leofds.iotladdereditor.compiler.domain.CodeOptionsDevice2;
import com.github.leofds.iotladdereditor.ladder.LadderProgram;

public class DeviceFactory {

	private LadderProgram ladderProgram;
	private Device device;

	public DeviceFactory() {
		try {
			ladderProgram = Mediator.getInstance().getProject().getLadderProgram();

			String mainDevice = ladderProgram.getProperties().getCodeOptionDevice();
			List<String> peripheralDevices = ladderProgram.getProperties().getDevices();

			this.device = createW1VC(mainDevice);
//			Peripheral output = new Peripheral("Q", "Q");
//			Peripheral input = new Peripheral("I", "I");
				
			
//			device = createEsp32();
			
			/*
			 *   Inputs/Outputs generating loops 
			 * 
			 */
			int id = 0;
			for (String propertiesDevice : peripheralDevices) {
				id++;
				List<Peripheral> listPeripheral = null;
				switch (propertiesDevice) {
				case "BB 20":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB20.name()).getPeripherals();
					break;
				case "BB 28":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB28.name()).getPeripherals();
					break;
				case "BB 32":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB32.name()).getPeripherals();
					break;
				case "BB 84":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB84.name()).getPeripherals();
					break;
				case "BB 128":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB128.name()).getPeripherals();
					break;
				case "BB 1616":
					listPeripheral = createBB(id, CodeOptionsDevice2.BB1616.name()).getPeripherals();
					break;

				default:
					break;
				}

				List<Peripheral> listPeripheralDevice = device.getPeripherals();

				for (Peripheral peripheralDevice : listPeripheralDevice) {
					for (Peripheral peripheral : listPeripheral) {
						List<PeripheralIO> list = peripheral.getPeripheralItems();
						for (PeripheralIO peripheralIO : list) {
							if (peripheralDevice.equals(peripheral)) {
								peripheralDevice.addPeripheralItem(peripheralIO);
								System.out.println(peripheralIO.toString());
							}
						}
					}
				}
			}

		} catch (Exception e) {
			device = createDefaultDevice();
			System.out.println("default Device Factory");
		}
	}

	public Device getDevice() {
		return device;
	}

	public static Device createDefaultDevice() {
		Device device = new Device("ESP32");
		Peripheral output = new Peripheral("Output", "Q");
		Peripheral input = new Peripheral("Input", "I");

		PeripheralIO pc1out = new PeripheralIO("Q0.1", Boolean.class, "2", "PIN_Q0.1", IO.OUTPUT);
		PeripheralIO pc2out = new PeripheralIO("Q0.2", Boolean.class, "4", "PIN_Q0.2", IO.OUTPUT);
		PeripheralIO pc3out = new PeripheralIO("Q0.3", Boolean.class, "12", "PIN_Q0.3", IO.OUTPUT);
		PeripheralIO pc4out = new PeripheralIO("Q0.4", Boolean.class, "13", "PIN_Q0.4", IO.OUTPUT);
		PeripheralIO pc5out = new PeripheralIO("Q0.5", Boolean.class, "33", "PIN_Q0.5", IO.OUTPUT);
		PeripheralIO pc6out = new PeripheralIO("Q0.6", Boolean.class, "25", "PIN_Q0.6", IO.OUTPUT);
		PeripheralIO pc7out = new PeripheralIO("Q0.7", Boolean.class, "26", "PIN_Q0.7", IO.OUTPUT);
		PeripheralIO pc8out = new PeripheralIO("Q0.8", Boolean.class, "27", "PIN_Q0.8", IO.OUTPUT);

		PeripheralIO pc1in = new PeripheralIO("I0.1", Boolean.class, "14", "PIN_I0.1", IO.INPUT);
		PeripheralIO pc2in = new PeripheralIO("I0.2", Boolean.class, "16", "PIN_I0.2", IO.INPUT);
		PeripheralIO pc3in = new PeripheralIO("I0.3", Boolean.class, "17", "PIN_I0.3", IO.INPUT);
		PeripheralIO pc4in = new PeripheralIO("I0.4", Boolean.class, "18", "PIN_I0.4", IO.INPUT);
		PeripheralIO pc5in = new PeripheralIO("I0.5", Boolean.class, "19", "PIN_I0.5", IO.INPUT);
		PeripheralIO pc6in = new PeripheralIO("I0.6", Boolean.class, "21", "PIN_I0.6", IO.INPUT);
		PeripheralIO pc7in = new PeripheralIO("I0.7", Boolean.class, "22", "PIN_I0.7", IO.INPUT);
		PeripheralIO pc8in = new PeripheralIO("I0.8", Boolean.class, "23", "PIN_I0.8", IO.INPUT);

		output.addPeripheralItem(pc1out);
		output.addPeripheralItem(pc2out);
		output.addPeripheralItem(pc3out);
		output.addPeripheralItem(pc4out);
		output.addPeripheralItem(pc5out);
		output.addPeripheralItem(pc6out);
		output.addPeripheralItem(pc7out);
		output.addPeripheralItem(pc8out);
		input.addPeripheralItem(pc1in);
		input.addPeripheralItem(pc2in);
		input.addPeripheralItem(pc3in);
		input.addPeripheralItem(pc4in);
		input.addPeripheralItem(pc5in);
		input.addPeripheralItem(pc6in);
		input.addPeripheralItem(pc7in);
		input.addPeripheralItem(pc8in);

		device.addPeripheral(output);
		device.addPeripheral(input);

		return device;
	}
/*
 * 
 * 	W1VC static configuration
 * 
 * 
 */
	public static Device createW1VC(String mainDevice) {

		Device device = new Device("");

		Peripheral output = new Peripheral("Output", "Q");
		Peripheral input = new Peripheral("Input", "I");

		if (mainDevice.equals(CodeOptionsDevice.W1VC_64R.name())) {

			device.setName(mainDevice);

			PeripheralIO pc1out = new PeripheralIO("Q0.1", Boolean.class, "2", "PIN_Q0.1", IO.OUTPUT);
			PeripheralIO pc2out = new PeripheralIO("Q0.2", Boolean.class, "4", "PIN_Q0.2", IO.OUTPUT);
			PeripheralIO pc3out = new PeripheralIO("Q0.3", Boolean.class, "12", "PIN_Q0.3", IO.OUTPUT);
			PeripheralIO pc4out = new PeripheralIO("Q0.4", Boolean.class, "13", "PIN_Q0.4", IO.OUTPUT);

			PeripheralIO pc1in = new PeripheralIO("I0.1", Boolean.class, "14", "PIN_I0.1", IO.INPUT);
			PeripheralIO pc2in = new PeripheralIO("I0.2", Boolean.class, "16", "PIN_I0.2", IO.INPUT);
			PeripheralIO pc3in = new PeripheralIO("I0.3", Boolean.class, "17", "PIN_I0.3", IO.INPUT);
			PeripheralIO pc4in = new PeripheralIO("I0.4", Boolean.class, "18", "PIN_I0.4", IO.INPUT);
			PeripheralIO pc5in = new PeripheralIO("I0.5", Boolean.class, "19", "PIN_I0.5", IO.INPUT);
			PeripheralIO pc6in = new PeripheralIO("I0.6", Boolean.class, "21", "PIN_I0.6", IO.INPUT);

			output.addPeripheralItem(pc1out);
			output.addPeripheralItem(pc2out);
			output.addPeripheralItem(pc3out);
			output.addPeripheralItem(pc4out);
			input.addPeripheralItem(pc1in);
			input.addPeripheralItem(pc2in);
			input.addPeripheralItem(pc3in);
			input.addPeripheralItem(pc4in);
			input.addPeripheralItem(pc5in);
			input.addPeripheralItem(pc6in);
		} else {
			if (mainDevice.equals(CodeOptionsDevice.W1VC_128R.name())) {

				device.setName(mainDevice);

				PeripheralIO pc1out = new PeripheralIO("Q0.1", Boolean.class, "2", "PIN_Q0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q0.2", Boolean.class, "4", "PIN_Q0.2", IO.OUTPUT);
				PeripheralIO pc3out = new PeripheralIO("Q0.3", Boolean.class, "12", "PIN_Q0.3", IO.OUTPUT);
				PeripheralIO pc4out = new PeripheralIO("Q0.4", Boolean.class, "13", "PIN_Q0.4", IO.OUTPUT);
				PeripheralIO pc5out = new PeripheralIO("Q0.5", Boolean.class, "33", "PIN_Q0.5", IO.OUTPUT);
				PeripheralIO pc6out = new PeripheralIO("Q0.6", Boolean.class, "25", "PIN_Q0.6", IO.OUTPUT);
				PeripheralIO pc7out = new PeripheralIO("Q0.7", Boolean.class, "26", "PIN_Q0.7", IO.OUTPUT);
				PeripheralIO pc8out = new PeripheralIO("Q0.8", Boolean.class, "27", "PIN_Q0.8", IO.OUTPUT);

				PeripheralIO pc1in = new PeripheralIO("I0.1", Boolean.class, "14", "PIN_I0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I0.2", Boolean.class, "16", "PIN_I0.2", IO.INPUT);
				PeripheralIO pc3in = new PeripheralIO("I0.3", Boolean.class, "17", "PIN_I0.3", IO.INPUT);
				PeripheralIO pc4in = new PeripheralIO("I0.4", Boolean.class, "18", "PIN_I0.4", IO.INPUT);
				PeripheralIO pc5in = new PeripheralIO("I0.5", Boolean.class, "19", "PIN_I0.5", IO.INPUT);
				PeripheralIO pc6in = new PeripheralIO("I0.6", Boolean.class, "21", "PIN_I0.6", IO.INPUT);
				PeripheralIO pc7in = new PeripheralIO("I0.7", Boolean.class, "22", "PIN_I0.7", IO.INPUT);
				PeripheralIO pc8in = new PeripheralIO("I0.8", Boolean.class, "23", "PIN_I0.8", IO.INPUT);
				PeripheralIO pc9in = new PeripheralIO("I0.9", Boolean.class, "25", "PIN_I0.9", IO.INPUT);
				PeripheralIO pc10in = new PeripheralIO("I0.10", Boolean.class, "26", "PIN_I0.10", IO.INPUT);
				PeripheralIO pc11in = new PeripheralIO("I0.11", Boolean.class, "27", "PIN_I0.11", IO.INPUT);
				PeripheralIO pc12in = new PeripheralIO("I0.12", Boolean.class, "28", "PIN_I0.12", IO.INPUT);

				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				output.addPeripheralItem(pc3out);
				output.addPeripheralItem(pc4out);
				output.addPeripheralItem(pc5out);
				output.addPeripheralItem(pc6out);
				output.addPeripheralItem(pc7out);
				output.addPeripheralItem(pc8out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
				input.addPeripheralItem(pc3in);
				input.addPeripheralItem(pc4in);
				input.addPeripheralItem(pc5in);
				input.addPeripheralItem(pc6in);
				input.addPeripheralItem(pc7in);
				input.addPeripheralItem(pc8in);
				input.addPeripheralItem(pc9in);
				input.addPeripheralItem(pc10in);
				input.addPeripheralItem(pc11in);
				input.addPeripheralItem(pc12in);
			}
		}
		device.addPeripheral(output);
		device.addPeripheral(input);

		return device;
	}
	
	public static Device createBB(int id, String deviceFromList) {
		Device device = new Device("");

		Peripheral output = new Peripheral("Output", "Q");
		Peripheral input = new Peripheral("Input", "I");

		if (deviceFromList.equals(CodeOptionsDevice2.BB20.name())) {

			PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+".1", IO.INPUT);
			PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+".2", IO.INPUT);

			input.addPeripheralItem(pc1in);
			input.addPeripheralItem(pc2in);
		} 
			if (deviceFromList.equals(CodeOptionsDevice2.BB28.name())) {

				PeripheralIO pc1out = new PeripheralIO("Q"+id+".1", Boolean.class, "2", "PIN_Q"+id+"0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q"+id+".2", Boolean.class, "4", "PIN_Q"+id+"0.2", IO.OUTPUT);
				PeripheralIO pc3out = new PeripheralIO("Q"+id+".3", Boolean.class, "12", "PIN_Q"+id+"0.3", IO.OUTPUT);
				PeripheralIO pc4out = new PeripheralIO("Q"+id+".4", Boolean.class, "13", "PIN_Q"+id+"0.4", IO.OUTPUT);
				PeripheralIO pc5out = new PeripheralIO("Q"+id+".5", Boolean.class, "33", "PIN_Q"+id+"0.5", IO.OUTPUT);
				PeripheralIO pc6out = new PeripheralIO("Q"+id+".6", Boolean.class, "25", "PIN_Q"+id+"0.6", IO.OUTPUT);
				PeripheralIO pc7out = new PeripheralIO("Q"+id+".7", Boolean.class, "26", "PIN_Q"+id+"0.7", IO.OUTPUT);
				PeripheralIO pc8out = new PeripheralIO("Q"+id+".8", Boolean.class, "27", "PIN_Q"+id+"0.8", IO.OUTPUT);

				PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+"0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+"0.2", IO.INPUT);

				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				output.addPeripheralItem(pc3out);
				output.addPeripheralItem(pc4out);
				output.addPeripheralItem(pc5out);
				output.addPeripheralItem(pc6out);
				output.addPeripheralItem(pc7out);
				output.addPeripheralItem(pc8out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
			}
			if (deviceFromList.equals(CodeOptionsDevice2.BB32.name())) {
				
				PeripheralIO pc1out = new PeripheralIO("Q"+id+".1", Boolean.class, "2", "PIN_Q"+id+"0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q"+id+".2", Boolean.class, "4", "PIN_Q"+id+"0.2", IO.OUTPUT);
				
				PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+"0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+"0.2", IO.INPUT);
				PeripheralIO pc3in = new PeripheralIO("I"+id+".3", Boolean.class, "17", "PIN_I"+id+"0.3", IO.INPUT);
				
				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
				input.addPeripheralItem(pc3in);
			}
			if (deviceFromList.equals(CodeOptionsDevice2.BB84.name())) {
				
				PeripheralIO pc1out = new PeripheralIO("Q"+id+".1", Boolean.class, "2", "PIN_Q"+id+"0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q"+id+".2", Boolean.class, "4", "PIN_Q"+id+"0.2", IO.OUTPUT);
				PeripheralIO pc3out = new PeripheralIO("Q"+id+".3", Boolean.class, "12", "PIN_Q"+id+"0.3", IO.OUTPUT);
				PeripheralIO pc4out = new PeripheralIO("Q"+id+".4", Boolean.class, "13", "PIN_Q"+id+"0.4", IO.OUTPUT);
				
				PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+"0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+"0.2", IO.INPUT);
				PeripheralIO pc3in = new PeripheralIO("I"+id+".3", Boolean.class, "17", "PIN_I"+id+"0.3", IO.INPUT);
				PeripheralIO pc4in = new PeripheralIO("I"+id+".4", Boolean.class, "18", "PIN_I"+id+"0.4", IO.INPUT);
				PeripheralIO pc5in = new PeripheralIO("I"+id+".5", Boolean.class, "19", "PIN_I"+id+"0.5", IO.INPUT);
				PeripheralIO pc6in = new PeripheralIO("I"+id+".6", Boolean.class, "21", "PIN_I"+id+"0.6", IO.INPUT);
				PeripheralIO pc7in = new PeripheralIO("I"+id+".7", Boolean.class, "22", "PIN_I"+id+"0.7", IO.INPUT);
				PeripheralIO pc8in = new PeripheralIO("I"+id+".8", Boolean.class, "23", "PIN_I"+id+"0.8", IO.INPUT);
				
				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				output.addPeripheralItem(pc3out);
				output.addPeripheralItem(pc4out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
				input.addPeripheralItem(pc3in);
				input.addPeripheralItem(pc4in);
				input.addPeripheralItem(pc5in);
				input.addPeripheralItem(pc6in);
				input.addPeripheralItem(pc7in);
				input.addPeripheralItem(pc8in);
			}
			if (deviceFromList.equals(CodeOptionsDevice2.BB128.name())) {
				
				PeripheralIO pc1out = new PeripheralIO("Q"+id+".1", Boolean.class, "2", "PIN_Q"+id+"0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q"+id+".2", Boolean.class, "4", "PIN_Q"+id+"0.2", IO.OUTPUT);
				PeripheralIO pc3out = new PeripheralIO("Q"+id+".3", Boolean.class, "12", "PIN_Q"+id+"0.3", IO.OUTPUT);
				PeripheralIO pc4out = new PeripheralIO("Q"+id+".4", Boolean.class, "13", "PIN_Q"+id+"0.4", IO.OUTPUT);
				PeripheralIO pc5out = new PeripheralIO("Q"+id+".5", Boolean.class, "33", "PIN_Q"+id+"0.5", IO.OUTPUT);
				PeripheralIO pc6out = new PeripheralIO("Q"+id+".6", Boolean.class, "25", "PIN_Q"+id+"0.6", IO.OUTPUT);
				PeripheralIO pc7out = new PeripheralIO("Q"+id+".7", Boolean.class, "26", "PIN_Q"+id+"0.7", IO.OUTPUT);
				PeripheralIO pc8out = new PeripheralIO("Q"+id+".8", Boolean.class, "27", "PIN_Q"+id+"0.8", IO.OUTPUT);
				
				PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+"0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+"0.2", IO.INPUT);
				PeripheralIO pc3in = new PeripheralIO("I"+id+".3", Boolean.class, "17", "PIN_I"+id+"0.3", IO.INPUT);
				PeripheralIO pc4in = new PeripheralIO("I"+id+".4", Boolean.class, "18", "PIN_I"+id+"0.4", IO.INPUT);
				PeripheralIO pc5in = new PeripheralIO("I"+id+".5", Boolean.class, "19", "PIN_I"+id+"0.5", IO.INPUT);
				PeripheralIO pc6in = new PeripheralIO("I"+id+".6", Boolean.class, "21", "PIN_I"+id+"0.6", IO.INPUT);
				PeripheralIO pc7in = new PeripheralIO("I"+id+".7", Boolean.class, "22", "PIN_I"+id+"0.7", IO.INPUT);
				PeripheralIO pc8in = new PeripheralIO("I"+id+".8", Boolean.class, "23", "PIN_I"+id+"0.8", IO.INPUT);
				PeripheralIO pc9in = new PeripheralIO("I"+id+".9", Boolean.class, "25", "PIN_I"+id+"0.9", IO.INPUT);
				PeripheralIO pc10in = new PeripheralIO("I"+id+".10", Boolean.class, "26", "PIN_I"+id+"0.10", IO.INPUT);
				PeripheralIO pc11in = new PeripheralIO("I"+id+".11", Boolean.class, "27", "PIN_I"+id+"0.11", IO.INPUT);
				PeripheralIO pc12in = new PeripheralIO("I"+id+".12", Boolean.class, "28", "PIN_I"+id+"0.12", IO.INPUT);
				
				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				output.addPeripheralItem(pc3out);
				output.addPeripheralItem(pc4out);
				output.addPeripheralItem(pc5out);
				output.addPeripheralItem(pc6out);
				output.addPeripheralItem(pc7out);
				output.addPeripheralItem(pc8out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
				input.addPeripheralItem(pc3in);
				input.addPeripheralItem(pc4in);
				input.addPeripheralItem(pc5in);
				input.addPeripheralItem(pc6in);
				input.addPeripheralItem(pc7in);
				input.addPeripheralItem(pc8in);
				input.addPeripheralItem(pc9in);
				input.addPeripheralItem(pc10in);
				input.addPeripheralItem(pc11in);
				input.addPeripheralItem(pc12in);
			}
			if (deviceFromList.equals(CodeOptionsDevice2.BB1616.name())) {
				
				PeripheralIO pc1out = new PeripheralIO("Q"+id+".1", Boolean.class, "2", "PIN_Q"+id+"0.1", IO.OUTPUT);
				PeripheralIO pc2out = new PeripheralIO("Q"+id+".2", Boolean.class, "4", "PIN_Q"+id+"0.2", IO.OUTPUT);
				PeripheralIO pc3out = new PeripheralIO("Q"+id+".3", Boolean.class, "12", "PIN_Q"+id+"0.3", IO.OUTPUT);
				PeripheralIO pc4out = new PeripheralIO("Q"+id+".4", Boolean.class, "13", "PIN_Q"+id+"0.4", IO.OUTPUT);
				PeripheralIO pc5out = new PeripheralIO("Q"+id+".5", Boolean.class, "33", "PIN_Q"+id+"0.5", IO.OUTPUT);
				PeripheralIO pc6out = new PeripheralIO("Q"+id+".6", Boolean.class, "25", "PIN_Q"+id+"0.6", IO.OUTPUT);
				PeripheralIO pc7out = new PeripheralIO("Q"+id+".7", Boolean.class, "26", "PIN_Q"+id+"0.7", IO.OUTPUT);
				PeripheralIO pc8out = new PeripheralIO("Q"+id+".8", Boolean.class, "27", "PIN_Q"+id+"0.8", IO.OUTPUT);
				PeripheralIO pc9out = new PeripheralIO("Q"+id+".9", Boolean.class, "2", "PIN_Q"+id+"0.9", IO.OUTPUT);
				PeripheralIO pc10out = new PeripheralIO("Q"+id+".10", Boolean.class, "4", "PIN_Q"+id+"0.10", IO.OUTPUT);
				PeripheralIO pc11out = new PeripheralIO("Q"+id+".11", Boolean.class, "12", "PIN_Q"+id+"0.11", IO.OUTPUT);
				PeripheralIO pc12out = new PeripheralIO("Q"+id+".12", Boolean.class, "13", "PIN_Q"+id+"0.12", IO.OUTPUT);
				PeripheralIO pc13out = new PeripheralIO("Q"+id+".13", Boolean.class, "33", "PIN_Q"+id+"0.13", IO.OUTPUT);
				PeripheralIO pc14out = new PeripheralIO("Q"+id+".14", Boolean.class, "25", "PIN_Q"+id+"0.14", IO.OUTPUT);
				PeripheralIO pc15out = new PeripheralIO("Q"+id+".15", Boolean.class, "26", "PIN_Q"+id+"0.15", IO.OUTPUT);
				PeripheralIO pc16out = new PeripheralIO("Q"+id+".16", Boolean.class, "27", "PIN_Q"+id+"0.16", IO.OUTPUT);
				
				PeripheralIO pc1in = new PeripheralIO("I"+id+".1", Boolean.class, "14", "PIN_I"+id+"0.1", IO.INPUT);
				PeripheralIO pc2in = new PeripheralIO("I"+id+".2", Boolean.class, "16", "PIN_I"+id+"0.2", IO.INPUT);
				PeripheralIO pc3in = new PeripheralIO("I"+id+".3", Boolean.class, "17", "PIN_I"+id+"0.3", IO.INPUT);
				PeripheralIO pc4in = new PeripheralIO("I"+id+".4", Boolean.class, "18", "PIN_I"+id+"0.4", IO.INPUT);
				PeripheralIO pc5in = new PeripheralIO("I"+id+".5", Boolean.class, "19", "PIN_I"+id+"0.5", IO.INPUT);
				PeripheralIO pc6in = new PeripheralIO("I"+id+".6", Boolean.class, "21", "PIN_I"+id+"0.6", IO.INPUT);
				PeripheralIO pc7in = new PeripheralIO("I"+id+".7", Boolean.class, "22", "PIN_I"+id+"0.7", IO.INPUT);
				PeripheralIO pc8in = new PeripheralIO("I"+id+".8", Boolean.class, "23", "PIN_I"+id+"0.8", IO.INPUT);
				PeripheralIO pc9in = new PeripheralIO("I"+id+".9", Boolean.class, "25", "PIN_I"+id+"0.9", IO.INPUT);
				PeripheralIO pc10in = new PeripheralIO("I"+id+".10", Boolean.class, "26", "PIN_I"+id+"0.10", IO.INPUT);
				PeripheralIO pc11in = new PeripheralIO("I"+id+".11", Boolean.class, "27", "PIN_I"+id+"0.11", IO.INPUT);
				PeripheralIO pc12in = new PeripheralIO("I"+id+".12", Boolean.class, "28", "PIN_I"+id+"0.12", IO.INPUT);
				PeripheralIO pc13in = new PeripheralIO("I"+id+".13", Boolean.class, "28", "PIN_I"+id+"0.13", IO.INPUT);
				PeripheralIO pc14in = new PeripheralIO("I"+id+".14", Boolean.class, "28", "PIN_I"+id+"0.14", IO.INPUT);
				PeripheralIO pc15in = new PeripheralIO("I"+id+".15", Boolean.class, "28", "PIN_I"+id+"0.15", IO.INPUT);
				PeripheralIO pc16in = new PeripheralIO("I"+id+".16", Boolean.class, "28", "PIN_I"+id+"0.16", IO.INPUT);
				
				output.addPeripheralItem(pc1out);
				output.addPeripheralItem(pc2out);
				output.addPeripheralItem(pc3out);
				output.addPeripheralItem(pc4out);
				output.addPeripheralItem(pc5out);
				output.addPeripheralItem(pc6out);
				output.addPeripheralItem(pc7out);
				output.addPeripheralItem(pc8out);
				output.addPeripheralItem(pc9out);
				output.addPeripheralItem(pc10out);
				output.addPeripheralItem(pc11out);
				output.addPeripheralItem(pc12out);
				output.addPeripheralItem(pc13out);
				output.addPeripheralItem(pc14out);
				output.addPeripheralItem(pc15out);
				output.addPeripheralItem(pc16out);
				input.addPeripheralItem(pc1in);
				input.addPeripheralItem(pc2in);
				input.addPeripheralItem(pc3in);
				input.addPeripheralItem(pc4in);
				input.addPeripheralItem(pc5in);
				input.addPeripheralItem(pc6in);
				input.addPeripheralItem(pc7in);
				input.addPeripheralItem(pc8in);
				input.addPeripheralItem(pc9in);
				input.addPeripheralItem(pc10in);
				input.addPeripheralItem(pc11in);
				input.addPeripheralItem(pc12in);
				input.addPeripheralItem(pc13in);
				input.addPeripheralItem(pc14in);
				input.addPeripheralItem(pc15in);
				input.addPeripheralItem(pc16in);
			}
		
		device.addPeripheral(output);
		device.addPeripheral(input);

		return device;
	}


}
