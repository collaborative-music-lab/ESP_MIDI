/*
  File Name: ESP32_MIDI.ino
  Version: v3
  Creator: Ian Hattwick
  Date: Dec 28, 2024

  Description:
  This program allows an ESP32_S3 to act as a class compliant midi device.
  NOTE: the ESP32 USB Mode MUST be set to "USB OTG (TinyUSB)".
  - once this is done, the board will no longer be recongized by Arduino
  - in order to reprogram the board, you will need to hold down the buttons on the board
  - hold down the B (boot) button and tap the R (reset) button
  - Arduino should recognize the ESP32s3
  - USB CDC on boot may need to be set to 'disabled'
  
  Input Signals:
  - capacitive touch pads. 8 for fingers, two alt pads, 
  - two potentiometers
  
  Output Signals:
  - two external LED outputs, TBD on pin 43 or 44
  - onboard LED on pin 21
  - MIDI out via USB

  Version notes:
  - this version 

  Additional Information:
  - Supports serial debugging. set SERIAL_DEBUG to 1 to enable debugging
  - ENABLE_USB_MIDI should be set to 1 to enable USB MIDI functionality.
*/

#include "USB_MIDI.h"
#include "potentiometer.h"
#include "controller.h"

#include "parameters.h"
#include <Preferences.h>
Preferences preferences;
ParameterObject parameters;

#include <FastLED.h>
CRGB leds[2]; // Array for LEDs on pin 13
CRGB built_in[1]; // Array for LEDs on pin 21

#include "capsense.h"

const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

/************** DEFINE INPUTS ******************/


const byte numPots = 1;
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[numPots] =  {Potentiometer(11)};
uint8_t potCCs[] = {2,1,2,3};


enum InstrumentMode { MONOPHONIC, POLYPHONIC };
InstrumentMode currentMode = MONOPHONIC;

void setMode(InstrumentMode mode) {
    currentMode = mode;
    //Serial.printf("Switched to %s mode\n", mode == MONOPHONIC ? "Monophonic" : "Polyphonic");
}


/************** SETUP ******************/

void setup() {
  //Serial.begin(115200);
  if( ENABLE_USB_MIDI) usbMidiSetup();

  // Initialize Preferences
    // preferences.begin("params", false); // Open namespace "params" for writing
    // // Load parameters
    // parameters.loadFromPreferences(preferences);
    // // Close Preferences
    // preferences.end();
  
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  touchSetCycles(10,5); //(uint16_t measure, uint16_t sleep);

  FastLED.addLeds<WS2811, 13, RGB>(leds, 2);
  FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  FastLED.setBrightness(25);

  pinMode(43, INPUT_PULLUP);
  pinMode(44, INPUT_PULLUP);

  //setupFingeringToMidiNote();

}//setup

void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  //Serial.printf("Note On: Channel %d, Note %d, Velocity %d\n", channel + 1, note, velocity);
  // Add your custom behavior for Note On here
}

void handleNoteOff(uint8_t channel, uint8_t note) {
  //Serial.printf("Note Off: Channel %d, Note %d\n", channel + 1, note);
  // Add your custom behavior for Note Off here
}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  built_in[0] = CRGB(0, 0, 255);
  FastLED.show();
  if (number == 120 && value == 120) { // CC 120 triggers the parameter query
        sendStoredParameters();
        return;
    } else updateParameterValue(channel, number, value);
}


/************** LOOP ******************/

byte currentSensor = 0;
int measureCycles = 100;   // Default measure cycles
int sleepCycles = 20;      // Default sleep cycles

void loop() {
  // if (Serial.available() > 0) {
  //       parseSerialCommand();
  //   }
  processIncomingMidi();

  // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 10; 

  if(millis()-timer > interval){
    timer= millis();
    for(int i=0;i<10;i++){
      cap[i].update();
      
      if( SERIAL_DEBUG ){
        if( currentSensor == i )  printVals(cap[i].getValue(), cap[i].getState());
      } 
      if(ENABLE_USB_MIDI){
        if( cap[i].state == true && cap[i].getChange() == true){
          sendMidiNoteOn( i, touchToMidi(cap[i].getValue()) );
          leds[0] = CRGB(255, 0, 0);
          leds[1] = CRGB(255, 0, 0);
        } else if( cap[i].state == false && cap[i].getChange() == true){
          sendMidiNoteOff( i );
          leds[0] = CRGB(0, 255, 0);
          leds[1] = CRGB(0, 255, 0);
          built_in[0] = CRGB(0, 0, 0);
        }
        FastLED.show();
      }
    }
  }//timer
  int val = map(touchRead(12),0,4095,0,127);
  val = val>127 ? 127 : val;
  sendMidiCC( 12, val);

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
}//loop

void readTouchpads() {
  for (int i = 0; i < 11; i++) {
    cap[i].update();
    if( cap[i].state == true && cap[i].getChange() == true){
          if (currentMode == MONOPHONIC) {
                handleMonophonicTouchpad(i, 1);
            } else {
                handlePolyphonicTouchpad(i, 1);
            }
        } else if( cap[i].state == false && cap[i].getChange() == true){
          if (currentMode == MONOPHONIC) {
                handleMonophonicTouchpad(i, 0);
            } else {
                handlePolyphonicTouchpad(i, 0);
            }
        }
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

uint8_t touchToMidi(int val){
  val = map(val, 0,4095, 0,127);
  return uint8_t(val>127 ? 127 : val);
}

