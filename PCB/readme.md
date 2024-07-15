# PCB and Electronics Assembly

The project consists of:

1. The main PCB, with the ESP32 pre-soldered
2. 4 potentiometers, 2 to place on the PCB and two additional off-board potentiometers with small pcbs.
3. 4 Arcade Buttons
4. An LSM6DS3 IMU breakout board
5. An RGB LED and resistors

## Assembly

Make sure get your enclosure drilled for your faceplate before you mount your PCB to the faceplate in step 9!!!

1. Place two potentiometers on the PCB. Make sure they are flat. It is a good idea to place the potentiometers on a faceplate to make sure they are aligned. Solder the potentiometers in place.
	* the potentiometers go on the opposite side of the PCB from the ESP32 (e.g. the ESP32 is on the bottom, and the potentiometers are on the top).
2. The LED uses 3 resistors with values between 120 and 470 ohm. All resistors may be the same value. Place and solder the resistors. 
3. DO NOT solder the LED yet. We will solder the PCB once we are ready to mount the electronics to the faceplate.
4. Solder the small PCBs to the two off-board potentiometers. The potentiometer pins go in the holes next to the word 'POT'
5. Solder headers to the LSM6DS3 breakout on the side that has 6 holes. Leave the five holes on the other side of the PCB empty.
6. Place the LSM6DS3 breakout into the head holes labelled 'IMU' on the PCB. Make sure the V+ and VIN pins on the PCB and breakout are aligned. Double-check - if you are not confident, ask for help! 
	* The breakout should be on the bottom of the board, the same side as the ESP32
7. Cut wires for connecting the arcade buttons and external potentiometers. You will need 3 groups with different colors:
	* 6 wires for grounds
	* 6 wires for signal
	* 2 wires for V+
8. Solder these wires into the PCB. Note that there are six places for external sensors, each with a ground, V+, and signal pad. You will need to solder each ground and signal (A1-A6) pad, but only two of the V+/3v pads. 
9. Once all of these wires are soldered in it is time to mount the PCB in the faceplate and solder the LED. 
10. Put the LED in the LED holes. The longest leg of the LED goes into the round hole next to the square hole. If you look on the bottom of the PCB you will see one trace connected to the LED holes - that hole is where the long leg goes. 
	* It is a good idea to plug in your ESP32 and test the LED works before soldering it down
	* you will also need to adjust the LED so it fits through the hold in the faceplate before soldering it.
11. Once the LED is soldered you can screw the potentiometers on the PCB to the faceplate. Use a 10mm socket to screw this down.
12. Place the arcade buttons and other potentiometers in the faceplate. Make sure the faceplate fits inside the enclosure
13. Solder the wires to the potentiometers. Look on the small PCB and you will see indications for where G, V+, and Sig go.
14. Solder wires to the Arcade buttons. These just have a ground and signal wire soldered to the two pins. It doesn't matter which pin is ground and which is signal.
15. You are done soldering! Time to program the ESP32 and test your MIDI controller. You might want to hold off on screwing the faceplate down until you have tested all of the sensors. . .