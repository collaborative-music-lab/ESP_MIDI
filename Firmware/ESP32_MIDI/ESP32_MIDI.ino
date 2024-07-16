/*
  File Name: ESP32_MIDI.ino
  Version: 1.0
  Creator: Ian Hattwick
  Date: July 1, 2024

  Description:
  This program allows an ESP32_S3 to act as a class compliant midi device.
  NOTE: the ESP32 USB Mode MUST be set to "USB OTG (TinyUSB)".
  - once this is done, the board will no longer be recongized by Arduino
  - in order to reprogram the board, you will need to hold down the buttons on the board
  - hold down the B (boot) button and tap the R (reset) button
  - Arduino should recognize the ESP32s3
  
  Input Signals:
  - IMU (Inertial Measurement Unit): 
    - Uses the LSM6 library to read acceleration data, mapped to MIDI CC messages (10, 11, 12, 13).

  - Potentiometers:
    - Configured for four potentiometers connected to pins 2, 9, 8, and 6.
    - Includes options for pin number and for potentiometer ranges to be reversed.
    - Potentiometer values are mapped to MIDI CC messages (0, 1, 2, 3).
    - Potentiometer values also control the RGB LED colors.

  - Buttons:
    - Configured for four buttons connected to pins 3, 4, 5, and 7.
    - Button presses send MIDI note on messages, and releases send MIDI note off messages.
    - Button note numbers are mapped to MIDI notes {36,38,42,46}
    - (kick,snare,closed hat, open hat)

  Additional Information:
  - Supports serial debugging. set SERIAL_DEBUG to 1 to enable debugging
  - ENABLE_USB_MIDI should be set to 1 to enable USB MIDI functionality.
*/

#include "USB_MIDI.h"
#include "LSM6.h"
#include "potentiometer.h"
#include "buttons.h"
#include "led.h"

const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

/************** DEFINE INPUTS ******************/

LSM6 imu;
uint8_t accelCCs[] = {10,11,12,13};

const byte numPots = 4;
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[numPots] = {Potentiometer(2), Potentiometer(9, 1), Potentiometer(8, 1), Potentiometer(6)};
uint8_t potCCs[] = {0,1,2,3};

const byte numButtons = 4;
//for buttons the argument is the pin number
Button buttons[numButtons] = { Button(3), Button(4), Button(5), Button(7) };
uint8_t buttonNoteNums[] = {36,38,42,46};
//keep track of when notes are triggered to temporarily mute IMU
uint32_t noteTriggerTime = 0;

// arguments are the pin numbers
LED led(11,12,13);

/************** SETUP ******************/

void setup() {
  Serial.begin(115200);
  if( ENABLE_USB_MIDI) usbMidiSetup();
  
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  imuSetup();
}//setup

/************** LOOP ******************/

void loop() {
  // Handle MIDI input
  if(millis() > noteTriggerTime + 10) imuLoop(); //handle IMU
  led.loop(); //handle LEDS

  //potentiometers
  for(byte i=0;i<numPots;i++){
    int val = pots[i].read();
    if( val >= 0 ) {
      if(SERIAL_DEBUG) {
        Serial.println("Pot " + String(i) + ": " + String(val));
      }
      else {
        sendMidiCC( potCCs[i], val);
      }

      //use potentiometer values to set LED color
      if(i==0) led.setRed(val*2);
      if(i==1) led.setGreen(val*2);
      if(i==2) led.setBlue(val*2);
    }
  }

  //buttons
  for(byte i=0;i<numButtons;i++){
    buttons[i].loop();
    if(SERIAL_DEBUG){
      if( buttons[i].isPressed()) Serial.println("Button " + String(i) + " pressed ");
      else if( buttons[i].isReleased()) Serial.println("Button " + String(i) + " released ");
    } else{
      if( buttons[i].isPressed()) {
        sendMidiNoteOn( buttonNoteNums[i], 127 );
        noteTriggerTime = millis();
      }
      else if( buttons[i].isReleased()) sendMidiNoteOff( buttonNoteNums[i] );
    }
  }
}

