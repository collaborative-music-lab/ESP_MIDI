/*
  File Name: ESP32_MIDI_Jan2024.ino
  Version: 1.0
  Creator: Ian Hattwick
  Date: Dec 17, 2024

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
#include "potentiometer.h"
//#include "Capsense.h"

const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;
/************** DEFINE INPUTS ******************/

const byte numPots = 4;
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[numPots] = {Potentiometer(2), Potentiometer(9, 1), Potentiometer(8, 1), Potentiometer(6)};
uint8_t potCCs[] = {0,1,2,3};

// // Pins for the sensors
// const int touchPins[10] = {8, 9, 10, 5, 4, 3, 2, 1, 7, 6};

// // Upper and lower thresholds
// const int UPPER_THRESHOLD = 300;
// const int LOWER_THRESHOLD = 200;

// // Array of CapSense instances
// CapSense cap[] = {
//     CapSense(touchPins[0], 0, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[1], 1, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[2], 2, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[3], 3, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[4], 4, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[5], 5, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[6], 6, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[7], 7, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[8], 8, UPPER_THRESHOLD, LOWER_THRESHOLD),
//     CapSense(touchPins[9], 9, UPPER_THRESHOLD, LOWER_THRESHOLD)
// };

// arguments are the pin numbers
// LED led(11,12,13);

/************** SETUP ******************/

void setup() {
  Serial.begin(115200);
  if( ENABLE_USB_MIDI) usbMidiSetup();
  
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  //touchSetCycles(10,1000); //(uint16_t measure, uint16_t sleep);
  //for(int i=0;i<10;i++) touchSetThreshold(i+1, 1000);
  //for(int i=0;i<10;i++) pinMode(i+1, INPUT_PULLUP);
  //imuSetup();
}//setup

/************** LOOP ******************/

void loop() {
  return;

  // static uint32_t timer = 0;
  // int interval = 100;
  // static int count = 0;
  // if(millis()-timer > interval){
  //   timer= millis();
  //   uint32_t data[10];
  //   // int touchPins = []
  //   for(int i=0;i<10;i++){
  //     cap[i].update();
      
  //     if( SERIAL_DEBUG ){
  //       if (cap[i].getState()  ) {
  //         Serial.print(cap[i].getValue());
  //         Serial.print('\t');
  //       }
  //       else{
  //         Serial.print(cap[i].getValue()/100);
  //         Serial.print('\t');
  //       }
  //     } 
  //     // if(ENABLE_USB_MIDI){
  //     //   sendMidiNoteOn( buttonNoteNums[i], 127 );
  //     // }
  //   }
  //   // Serial.print("pot:\t");
  //   // Serial.println(analogRead(11));
  //     Serial.println();
  // }
  // // Handle MIDI input
  // if(millis() > noteTriggerTime + 10) imuLoop(); //handle IMU
  // led.loop(); //handle LEDS

  // //potentiometers
  // for(byte i=0;i<numPots;i++){
  //   int val = pots[i].read();
  //   if( val >= 0 ) {
  //     if(SERIAL_DEBUG) {
  //       Serial.println("Pot " + String(i) + ": " + String(val));
  //     }
  //     else {
  //       sendMidiCC( potCCs[i], val);
  //     }

  //     //use potentiometer values to set LED color
  //     if(i==0) led.setRed(val*2);
  //     if(i==1) led.setGreen(val*2);
  //     if(i==2) led.setBlue(val*2);
  //   }
  // }

  // //buttons
  // for(byte i=0;i<numButtons;i++){
  //   buttons[i].loop();
  //   if(SERIAL_DEBUG){
  //     if( buttons[i].isPressed()) Serial.println("Button " + String(i) + " pressed ");
  //     else if( buttons[i].isReleased()) Serial.println("Button " + String(i) + " released ");
  //   } else{
  //     if( buttons[i].isPressed()) {
  //       sendMidiNoteOn( buttonNoteNums[i], 127 );
  //       noteTriggerTime = millis();
  //     }
  //     else if( buttons[i].isReleased()) sendMidiNoteOff( buttonNoteNums[i] );
  //   }
  // }
}

