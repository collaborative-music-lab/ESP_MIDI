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
#include "potentiometer.h"
#include "led.h"
#include "capsense.h"

const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

/************** DEFINE INPUTS ******************/

const byte numPots = 1;
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[numPots] =  {Potentiometer(11)};
uint8_t potCCs[] = {2,1,2,3};

// Pins for the sensors
const int touchPins[10] = {8, 9, 10, 5, 4, 3, 2, 1, 7, 6};

// Upper and lower thresholds
const int UPPER_THRESHOLD = 300;
const int LOWER_THRESHOLD = 100;

// Array of CapSense instances
CapSense cap[] = {
    CapSense(touchPins[0], 0, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[1], 1, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[2], 2, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[3], 3, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[4], 4, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[5], 5, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[6], 6, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[7], 7, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[8], 8, UPPER_THRESHOLD, LOWER_THRESHOLD),
    CapSense(touchPins[9], 9, UPPER_THRESHOLD, LOWER_THRESHOLD)
};

// Global FastLED array
// Create LED objects for each LED
WS2812LED led[] = { WS2812LED(0), WS2812LED(1) };

/************** SETUP ******************/

void setup() {
  Serial.begin(115200);
  if( ENABLE_USB_MIDI) usbMidiSetup();
  
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  touchSetCycles(10,50); //(uint16_t measure, uint16_t sleep);

  WS2812LED::begin();

  //neopixelWrite(21 ,200,0,0); // Red
}//setup

/************** LOOP ******************/

byte currentSensor = 0;
int measureCycles = 100;   // Default measure cycles
int sleepCycles = 20;      // Default sleep cycles

void loop() {
  if (Serial.available() > 0) {
        parseSerialCommand();
    }

  // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 10; 

  if(millis()-timer > interval){
    timer= millis();
    for(int i=0;i<10;i++){
      cap[i].update();
      
      if( SERIAL_DEBUG ){
        if( currentSensor == i )  printVals(cap[i].getValue(), cap[i].getState());
        // if (cap[i].getState()  ) {
        //   Serial.print(cap[i].getValue());
        //   Serial.print('\t');
        // }
        // else{
        //   Serial.print(cap[i].getValue());
        //   Serial.print('\t');
        // }
      } 
      if(ENABLE_USB_MIDI){
        if( cap[i].state == true && cap[i].getChange() == true){
          sendMidiNoteOn( i, 127 );
          led[0].color(100,100,100);
          led[1].color(100,100,100);
        } else if( cap[i].state == false && cap[i].getChange() == true){
          sendMidiNoteOff( i );
          led[0].color(200,100,255);
          led[1].color(100,100,100);
        }
        WS2812LED::show();
      }
    }
     if( SERIAL_DEBUG ) { Serial.println();

     }
  }

  //potentiometers
  for(byte i=0;i<numPots;i++){
    int val = pots[i].read();
    if( val >= 0 ) {
      if(SERIAL_DEBUG) {
       // Serial.println("Pot " + String(i) + ": " + String(val));
      }
      else {
        sendMidiCC( potCCs[i], val);
      }

    }
  }

  //expressin strips
  if(SERIAL_DEBUG) {
       // Serial.println("Pot " + String(i) + ": " + String(val));
      }
      else {
        sendMidiCC( 0, constrain( map( cap[8].getValue(), 0,5000,0,127),0,127 ));
        sendMidiCC( 1, constrain( map( cap[9].getValue(), 0,5000,0,127),0,127 ));
      }
}

void printVals(int raw, bool touch) {
    int scaledTouch = touch ? 1000 : 0;
    Serial.print(1000);
    Serial.print(" ");
    Serial.print(raw);
    Serial.print(" ");      // Space separates multiple values (if added later)
    Serial.println(scaledTouch);
}

// Function to parse serial input commands
void parseSerialCommand() {
    String command = Serial.readStringUntil('\n'); // Read input until newline

    if (command.length() < 2) {
        Serial.println("Invalid command");
        return;
    }

    char type = command[0];          // First character is the command type
    int value = command.substring(1).toInt(); // Extract the number after the command

    switch (type) {
        case 'p': // Change active sensor
            if (value >= 0 && value < 10) {
                currentSensor = value;
                Serial.print("Active sensor set to: ");
                Serial.println(currentSensor);
            } else {
                Serial.println("Invalid sensor number");
            }
            break;

        case 'c': // Change measure cycles
            if (value > 0) {
                measureCycles = value;
                touchSetCycles(measureCycles, sleepCycles);
                Serial.print("Measure cycles set to: ");
                Serial.println(measureCycles);
            } else {
                Serial.println("Invalid measure cycles value");
            }
            break;

        case 's': // Change sleep cycles
            if (value > 0) {
                sleepCycles = value;
                touchSetCycles(measureCycles, sleepCycles);
                Serial.print("Sleep cycles set to: ");
                Serial.println(sleepCycles);
            } else {
                Serial.println("Invalid sleep cycles value");
            }
            break;

        default:
            Serial.println("Unknown command");
            break;
    }
}