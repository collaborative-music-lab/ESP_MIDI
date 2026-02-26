/*
  File Name: ESP32_MIDI.ino
  Version: v6
  Creator: Ian Hattwick
  Date: Jan 19, 2025

  Note: this is an exploatory version trying to switch to USBMIDI.h
  - tested as working!

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
  - two external LED outputs, on pin 43
  - onboard LED on pin 21
  - MIDI out via USB

  Version notes:
  - this version adds cc control for pots and bar

  Additional Information:
  - Supports serial debugging. set SERIAL_DEBUG to 1 to enable debugging
  - ENABLE_USB_MIDI should be set to 1 to enable USB MIDI functionality.
*/
const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

//sensor global variables
byte ccSendRate = 1;

#include "USB_MIDI.h"
#include <Wire.h>
// #include "potentiometer.h"
#include "controller.h"
#include "buttons.h"
//#include "./src/LSM6.h"
// #include "./src/BMI160Gen.h"
// #include <esp_task_wdt.h>
// #include "enc.h"

// #include "parameters.h"
// #include <Preferences.h>
// Preferences preferences;
// ParameterObject parameters;

#include <FastLED.h>
CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 

/************** DEFINE INPUTS ******************/
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2))
CController cc[] = {
  CController(0, 50, 0, 3000, 10, .1), //pot1
  CController(1, 2, 1900, 2800, 3, .5), //watchdog timer
  CController(2, 20, 0, 2000, 10, .1), //
  CController(3, 20, 0, 500, 10, .1), //touchbar
  CController(4, 20, 0, 500, 10, .1),  //
  //afterTouch for pads 0-7
  CController(10, 20, -127, 127, 1, .5), //7 accelX
  CController(11, 20, -127, 127, 1, .5), //8 accelkY
  CController(12, 20, -127, 127, 1, .5), //9 accelZ
  CController(20, 50, 0, 180, 1, .5), //10 tiltX
  CController(21, 50, 0, 180, 1, .5), //11 tilyY
  CController(22, 50, 0, 180, 1, .5), //12 tiltZ
  CController(30, 20, 0, 127, 1, .5), //13 total magnitude
  CController(17, 20, 0, 127, 1, .8),
};

int velocityDepth = 200;

Button key[] = {
  Button(1),
  Button(2),
  Button(3),
  Button(4)
};

/************** SETUP ******************/

void setup() {

  FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  FastLED.setBrightness(25);
  built_in[0] = CRGB(0, 0, 20);
  FastLED.show();

  if( ENABLE_USB_MIDI) usbMidiSetup();
  else Serial.begin(115200);
  if( SERIAL_DEBUG) {
    while (!Serial && millis() < 5000) {
    delay(10);
    }
    Serial.println("Serial enabled");
    while(0) {
      Serial.println("Serial enabled");
      delay(100);
    }
  }


  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  if( SERIAL_DEBUG) Serial.println("Serial enable");

}//setup



void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {  
  
}

void handleNoteOff(uint8_t channel, uint8_t note) {

}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  statusLed(1);
  
  if( number == 127) {
    ESP.restart(); // Clean restart  
  } 

}


/************** LOOP ******************/

void loop() {
  //imuLoop();
  statusLed(0);
  processCCBuffer();

  // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 10; 
  static byte prevValue[] = {0,0,0,0,0};
  static byte num = 0;



    for(byte i=0;i<4;i++) {
      key[i].loop();
       if( key[i].isPressed() ){
        sendMidiNoteOn( i, 127 );
        statusLed(2);
        FastLED.show();
      } else if( key[i].isReleased() ){
        sendMidiNoteOff( i );
      }

    }

    if(millis()-timer > interval){
    timer= millis();
   

    processIncomingMidi();
  }//timer
        
}//loop

void statusLed(int num){
  byte color[][3] = {
    {100, 0, 0},   // Red
    {0, 100, 0},   // Green
    {0, 0, 100},   // Blue
    {100, 0, 100}, // Magenta
    {100, 100, 100} // White
  };
  static int state = 0;
  static uint32_t timer = 0;
  int interval = 63;
  if( num==0){
    if(state>0){
      if(millis()-timer > interval){
        state = 0;
        built_in[0] = CRGB(0, 0, 0);
        FastLED.show();
      }
      return;
    } else return;
  } 

  if(num>5) return;

  //flash led
  timer = millis();
  state=1;
  built_in[0] = CRGB(color[num-1][0], color[num-1][1], color[num-1][2]);
  FastLED.show();
}