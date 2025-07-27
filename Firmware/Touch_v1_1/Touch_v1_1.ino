/*
  File Name: ESP32_MIDI.ino
  Version: v6
  Creator: Ian Hattwick
  Date: Jan 19, 2025

  Note: this version is meant to be working with hall efect sensors

  Description:
  This program allows an ESP32_S3 to act as a class compliant midi device.
  NOTE: the ESP32 USB Mode MUST be set to "USB OTG (TinyUSB)".
  - once this is done, the board will no longer be recongized by Arduino
  - in order to reprogram the board, you will need to hold down the buttons on the board
  - hold down the B (boot) button and tap the R (reset) button
  - Arduino should recognize the ESP32s3
  - USB CDC on boot may need to be set to 'disabled'
  

  Additional Information:
  - Supports serial debugging. set SERIAL_DEBUG to 1 to enable debugging
  - ENABLE_USB_MIDI should be set to 1 to enable USB MIDI functionality.
*/
const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

#include "USB_MIDI.h"
#include "controller.h"
#include "hallEffect.h"
#include "parameters.h"
#include <Preferences.h>
#include "src/m370_MPR121.h"
#include "clock.h"
Preferences preferences;
ParameterObject parameters;

#include <FastLED.h>
CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 



//#include "capsense.h"
bool mpr121_present[4] = {false, false, false, false};  // for 0x5A to 0x5D

m370_MPR121 mpr121[] = {
  m370_MPR121(43,44),
    m370_MPR121(43,44),
      m370_MPR121(43,44),
        m370_MPR121(43,44)
        }; //SDA,SCL

HallEffectSensor hall[] = { HallEffectSensor(), HallEffectSensor() };

void setupESPNow();
void setupClockTimer();


int monitorInput = -1;
/************** DEFINE INPUTS ******************/
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float minCutoff, float beta, float dCutoff ))
//deltaThreshold: how much the value must change before a new value is sent
//float minCutoff: Minimum cutoff frequency of the filter, in Hz. Lower values = more smoothing always (sluggish output).Higher values = more responsive overall (but possibly jittery).
//float beta = Responsiveness scaling factor. Higher beta means more reactivity to fast movements (less smoothing).
//float dCutoff: Derivative filter cutoff frequency. Low values = more stable velocity estimate, but might lag.
int hallEffectRate = 5;
int potEffectRate = 50;
int capEffectRate = 50;
float ccRateScalar = 1.0;

CController cc[] = {
  CController(0, hallEffectRate, 10, 4095, 5,   .001, 0.01, 4), //hall1
  CController(1, hallEffectRate, 10, 4095, 5,    .001, 0.01, 4), //hall2
  CController(2, potEffectRate, 50, 4000, 16, 1, 0.02, 2), //pot1
  CController(3, potEffectRate, 50, 4000, 16, 1, 0.02, 2), //pot2
  CController(4, capEffectRate, 20, 400, 32, 1, 0.02, 2),  //Capsense1
  CController(5, capEffectRate, 20, 400, 32, 1, 0.02, 2), //Capsense2
};

/************** SETUP ******************/

void setup() {

  if( ENABLE_USB_MIDI) usbMidiSetup();
  else Serial.begin(115200);

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
  esp_log_level_set("*", ESP_LOG_NONE);  // Suppress all component logs

  touchSetCycles(5,10); //(uint16_t measure, uint16_t sleep);

  const byte numCapSensors = m370_cap_begin();
  if(SERIAL_DEBUG){
  Serial.print(numCapSensors);
  Serial.println(" MPR121 devices found");
  }

  setupESPNow();
  setupClockTimer();

  FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  FastLED.clear();
  built_in[0] = CRGB(0, 20, 0);
  FastLED.show();
  FastLED.setBrightness(50);  // Max brightness

}//setup

/************** LOOP ******************/

void loop() {
  processMidiSendQueue();
  

  // // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 2; 

  if(millis()-timer > interval){
    processIncomingMidi();
    statusLed(0);

    timer= millis();
    int hallVal[2];

    hallVal[0] = hall[0].update( analogRead(2) );
    hallVal[1] = hall[1].update( analogRead(9) );

    cc[0].send(hallVal[0]); //12    2
    cc[1].send(hallVal[1]); // 5     9
    cc[2].send(analogRead(10)); //11    3
    cc[3].send(analogRead(8)); //10    4
    cc[4].send( touchRead(11) / 10); //4       10
    cc[5].send( touchRead(7) / 10); //7       7

    readCap();
  }
}//loop

void readCap() {
  static uint64_t lastTouchStatus = 0;  // Previous 48-bit touch state
  uint64_t touchStatus = 0;             // Current 48-bit touch state

  for (uint8_t d = 0; d < 4; d++) {
   if( SERIAL_DEBUG) Serial.println(mpr121_present[d]);
    if (mpr121_present[d]) {
      uint16_t deviceTouch = mpr121[d].touched();  // 12-bit mask
      if( SERIAL_DEBUG)  Serial.println(deviceTouch, BIN);
      //Serial.println(deviceTouch);
      // if(deviceTouch == 0xFFFF) {
      //   m370_cap_begin();
      //   if(SERIAL_DEBUG) Serial.println("bad sensor reeading");
      //   statusLed(1);
      //   return;
      // }
      touchStatus |= ((uint64_t)deviceTouch) << (d * 12);

      // uint8_t reg = mpr121[d].readRegister8(0x5E);
      // if (reg == 0xFF) {
      //   Serial.println("MPR121 readRegister failed");
      //   m370_cap_begin();
      //   if(SERIAL_DEBUG) Serial.println("bad sensor reeading");
      //   statusLed(1);
      // }
    }
  }
  //if( SERIAL_DEBUG) Serial.print("touch ");
  //if( SERIAL_DEBUG) Serial.println(touchStatus, BIN);
  for (uint8_t i = 0; i < 48; i++) {
    bool wasTouched = lastTouchStatus & ((uint64_t)1 << i);
    bool isTouched  = touchStatus     & ((uint64_t)1 << i);

    if (!wasTouched && isTouched) {
      sendTouch(i, 1);  // rising edge
    } else if (wasTouched && !isTouched) {
      sendTouch(i, 0);  // falling edge
    }
  }

  lastTouchStatus = touchStatus;

}

/* MIDI SEND NOTE */

void sendTouch(byte num, byte val){
  statusLed(5);
  uint8_t midiNotes[48];
  const byte notes[] = {0,2,4,5,7,9,11};

  for(byte i = 0; i<48; i++) midiNotes[i] = notes[i%7] + i/7*12 + 36;

  if(SERIAL_DEBUG){
    Serial.print("touch\t");
    Serial.print(num);
    Serial.print("\t");
    Serial.println(val);
  } else{
    sendMidiNoteOn(midiNotes[num], val*127);
  }
}

/*****/

uint8_t m370_cap_begin() {
  statusLed(5);
  delay(100);
  if(SERIAL_DEBUG) Serial.println("starting mpr121");
  const uint8_t addresses[4] = {0x5A, 0x5B, 0x5C, 0x5D};

  uint8_t count = 0;

  for (uint8_t i = 0; i < 4; i++) {
    if( SERIAL_DEBUG) Serial.println(i);
    mpr121_present[i] = false;
    if (mpr121[i].begin(addresses[i])) {
      if( SERIAL_DEBUG) Serial.println(addresses[i], HEX);
      mpr121_present[i] = true;
      //mpr121[i].setThresholds(12, 6);
      mpr121[i].chargeCurrent(63);
      mpr121[i].chargeTime(3);
      count++;
    }
  }
  delay(10);

  return count;  // number of MPR121s successfully initialized
}

/******
MIDI INPUT
******/
void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {  
}

void handleNoteOff(uint8_t channel, uint8_t note) {
}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  statusLed(1);
  if( number == 100) { updateDataRate(float(value)); return;} 
  if( number == 101) { m370_cap_begin(); return;} 

  return;
}

void statusLed(int num){
  byte color[][3] = {
    {0,0,0},
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
}//status LED

void updateDataRate(float val){
  ccRateScalar = (val/13)+1;

  int minCCRates [] = {hallEffectRate, hallEffectRate, potEffectRate, potEffectRate, capEffectRate, capEffectRate};

  for(byte i =0; i<6; i++){
    cc[i].minInterval = (int)(ccRateScalar * minCCRates[i]);
  }
}
