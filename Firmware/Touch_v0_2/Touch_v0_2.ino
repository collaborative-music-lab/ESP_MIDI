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
//CRGB leds[2]; // Array for LEDs on pin 43
//CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 

//#include "capsense.h"

m370_MPR121 mpr121(43,44); //SDA,SCL
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
CController cc[] = {
  CController(0, 5, 10, 4095, 5,   .001, 0.01, 4), //hall1
  CController(1, 5, 10, 4095, 5,    .001, 0.01, 4), //hall2
  CController(2, 50, 50, 4000, 16, 1, 0.02, 2), //pot1
  CController(3, 50, 50, 4000, 16, 1, 0.02, 2), //pot2
  CController(4, 50, 20, 400, 32, 1, 0.02, 2),  //Capsense1
  CController(5, 50, 20, 400, 32, 1, 0.02, 2), //Capsense2
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

  m370_cap_begin();

  setupESPNow();
  setupClockTimer();

}//setup


void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {  
  // startup_leds = false;
  // byte color[][3] = {
  //   {100, 0, 0},   // Red
  //   {0, 100, 0},   // Green
  //   {0, 0, 100},   // Blue
  //   {100, 0, 100}, // Magenta
  //   {100, 100, 100} // White
  // };
  // if (channel < 2) { //Use channels 1 and 2 to change color (velocity is brightness)
  //   FastLED.setBrightness(velocity);
  //   leds[channel] = CRGB(color[note % 5][0], color[note % 5][1], color[note % 5][2]);
  //   FastLED.show();
  //   Serial.printf("Note On: Channel %d, Note %d, Velocity %d\n", channel + 1, note, velocity);
  // }else if(channel < 4){ //Use channels 3 and 4 to flash a light
  //   FastLED.setBrightness(velocity);
  //   leds[channel-2] = CRGB(color[note % 5][0], color[note % 5][1], color[note % 5][2]);
  //   FastLED.show();
  //   delay(5);
  //   leds[channel-2] = CRGB(0, 0, 0);
  //   FastLED.show();
  // }
}

void handleNoteOff(uint8_t channel, uint8_t note) {
  // if (channel < 2) {
  //   leds[channel] = CRGB(0, 0, 0);
  //   FastLED.show();
  //   Serial.printf("Note Off: Channel %d, Note %d\n", channel + 1, note); 
  // }

}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  // statusLed(1);
  // if( number == 127) {monitorInput = -1; return;} //disable all monitoring
  // else if( number >=80 && number<100){ monitorInput = number - 80; return;} //monitor raw data
  // else if( number >=60 && number<80){ cap[number-60].upperThreshold = value*20; return;} //update high threshold
  // else if( number >=40 && number<60){ cap[number-40].lowerThreshold = value*20;} //update low threshold
  // else if( number >=20 && number<40){ cap[number-20].upperThreshold = value*20;} //update velocity depth
  // //else if( number >=0 && number<20){ monitorInput = number - 80; return;}
  // // 
  // // if (number == 120 && value == 120) { // CC 120 triggers the parameter query
  // //       sendStoredParameters();
  // //       return;
  // //   } else updateParameterValue(channel, number, value);
}


/************** LOOP ******************/

void loop() {
  processMidiSendQueue();
  // statusLed(0);

  // // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 2; 

  if(millis()-timer > interval){
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

      if(0){
      Serial.print( touchRead(11) );
      Serial.print( "\t" );
      Serial.print( touchRead(7) );
      Serial.print( "\t" );
      Serial.println();
    }
  }
}//loop

void readCap() {
  static uint16_t lastTouchStatus = 0;               // previous touch bitmask
  uint16_t touchStatus = mpr121.touched();           // current touch bitmask

  for (uint8_t i = 0; i < 12; i++) {
    bool wasTouched = lastTouchStatus & (1 << i);    // bit i from last read
    bool isTouched  = touchStatus     & (1 << i);    // bit i from current read

    if (!wasTouched && isTouched) {
      sendTouch(i, 1);  // rising edge
    } else if (wasTouched && !isTouched) {
      sendTouch(i, 0);  // falling edge
    }
  }

  lastTouchStatus = touchStatus;  // update once, after loop
}

void sendTouch(byte num, byte val){
  const byte midiNotes[] = {48,50,52,53,55,57,59,60,62,64,65,67,69,71,72};
  if(SERIAL_DEBUG){
    Serial.print("touch\t");
    Serial.print(num);
    Serial.print("\t");
    Serial.println(val);
  } else{
    sendMidiNoteOn(midiNotes[num], val*127);
  }
}

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
        //built_in[0] = CRGB(0, 0, 0);
        //FastLED.show();
      }
      return;
    } else return;
  } 

  if(num>5) return;

  //flash led
  timer = millis();
  state=1;
  //built_in[0] = CRGB(color[num-1][0], color[num-1][1], color[num-1][2]);
  //FastLED.show();
}

void m370_cap_begin(){
  mpr121.begin();
  mpr121.chargeCurrent(63); //0-63, def 16
  mpr121.chargeTime(1); //1-7, def 1
  //mpr121.setThresholds(200, 100);
  //mpr121.proxChargeCurrent(63); //0-63, def 16
  //mpr121.proxChargeTime(3); //1-7, def 1

}