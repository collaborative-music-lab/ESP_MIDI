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
const byte ENABLE_USB_MIDI = 1;
const byte SERIAL_DEBUG = 0;

//sensor global variables
byte ccSendRate = 1;
int touchChargeTime = 20;
int touchSleepTime = 100;

#include "USB_MIDI.h"
#include <Wire.h>
#include "potentiometer.h"
#include "controller.h"
//#include "./src/LSM6.h"
#include "./src/BMI160Gen.h"
#include <esp_task_wdt.h>
#include "enc.h"

#include "parameters.h"
#include <Preferences.h>
Preferences preferences;
ParameterObject parameters;

#include <FastLED.h>

const byte NUM_LEDS = 9;
CRGB targets[NUM_LEDS];
bool smoothFlag[NUM_LEDS];

CRGB leds[NUM_LEDS]; // Array for LEDs on pin 7
CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 

// LSM6 imu;

#include "capsense.h"

/*********************************************
ENCODERS SETUP 
*********************************************/
//encoders rely on the  ESP32Encoder library
//Esp32Encoder rotaryEncoder = Esp32Encoder(18,2,4);//A,B,Button
//optional divider argument
//arguments:
// - A and B: digital inputs from encoder
// - Switch; pin for encoder switch, or -1 for no switch
// - divider: many encoders put out multiple pulses per detent
//   The divider helps to make encoder increments match detents

Esp32Encoder enc[] = {
  Esp32Encoder (43,44,-1,2,50),//A,B,Switch, Divider, CC number
  Esp32Encoder (8,9,-1,2, 51)
};

int monitorInput = -1;
/************** DEFINE INPUTS ******************/
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[] =  {Potentiometer(11)};

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


enum InstrumentMode { MONOPHONIC, POLYPHONIC };
InstrumentMode currentMode = MONOPHONIC;

void setMode(InstrumentMode mode) {
    currentMode = mode;
    //Serial.printf("Switched to %s mode\n", mode == MONOPHONIC ? "Monophonic" : "Polyphonic");
}


/************** SETUP ******************/

void setup() {
  Wire.begin(44,43); 

  FastLED.addLeds<WS2811, 7, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  FastLED.setBrightness(25);
  leds[0] = CRGB(20, 0, 0);
  leds[1] = CRGB(0, 0, 20);
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

  //for(byte i=0;i<2;i++) enc[i].begin([]{enc[0].readEncoder_ISR();});
  enc[0].begin([]{enc[0].readEncoder_ISR();});
  enc[1].begin([]{enc[1].readEncoder_ISR();});
  //imuSetup();


  // Initialize Preferences
    // preferences.begin("params", false); // Open namespace "params" for writing
    // // Load parameters
    // parameters.loadFromPreferences(preferences);
    // // Close Preferences
    // preferences.end();
  
  
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  //changed from 5,10
  //touchSetCycles(6, 5); //(uint16_t number of measure per cycle, uint16_t sleep, how many cycles to wait before measuring again);
  // touchSetCycles(50,100);
  touchSetCycles( touchChargeTime, touchSleepTime);
  //setupFingeringToMidiNote();
  if( SERIAL_DEBUG) Serial.println("Serial enable");

   // 1. Create configuration structure
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 500,           // 0.5-second timeout
    .idle_core_mask = 0,          // Don't monitor idle cores
    .trigger_panic = true         // Reset on timeout
  };
  
  // 2. Initialize with config struct
  esp_task_wdt_init(&wdt_config);
  
  // 3. Add current task to WDT
  esp_task_wdt_add(NULL);

   cc[1].send(127);

}//setup

//LEDS
inline uint8_t map2bitTo8(uint8_t v2) {               // 0..3 → 0..255
  return v2 * 85;                                      // 0,85,170,255
}

inline void decodeVelocity(uint8_t vel, bool &smooth, CRGB &c) {
  smooth = (vel & 0b0100'0000) != 0;
  uint8_t r2 = (vel >> 4) & 0x03;
  uint8_t g2 = (vel >> 2) & 0x03;
  uint8_t b2 = (vel >> 0) & 0x03;
  c = CRGB(map2bitTo8(r2), map2bitTo8(g2), map2bitTo8(b2));
}

// r2,g2,b2 in 0..3, smooth flag true/false → MIDI velocity 0..127
uint8_t packVelocity(uint8_t r2, uint8_t g2, uint8_t b2, bool smooth) {
  r2 &= 0x03; g2 &= 0x03; b2 &= 0x03;
  return (smooth ? 0b0100'0000 : 0) | (r2 << 4) | (g2 << 2) | b2;
}


void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {  
  /*
  startup_leds = false;
  byte color[][3] = {
    {100, 0, 0},   // Red
    {0, 100, 0},   // Green
    {0, 0, 100},   // Blue
    {100, 0, 100}, // Magenta
    {100, 100, 100} // White
  };
  if (channel < 2) { //Use channels 1 and 2 to change color (velocity is brightness)
    FastLED.setBrightness(velocity);
    leds[channel] = CRGB(color[note % 5][0], color[note % 5][1], color[note % 5][2]);
    FastLED.show();
    Serial.printf("Note On: Channel %d, Note %d, Velocity %d\n", channel + 1, note, velocity);
  }else if(channel < 4){ //Use channels 3 and 4 to flash a light
    FastLED.setBrightness(velocity);
    leds[channel-2] = CRGB(color[note % 5][0], color[note % 5][1], color[note % 5][2]);
    FastLED.show();
    delay(5);
    leds[channel-2] = CRGB(0, 0, 0);
    FastLED.show();
  }
  */

  if (note >= NUM_LEDS) return;

  // MIDI quirk: Note On with velocity 0 == Note Off
  if (velocity == 0) {
    targets[note] = CRGB::Black;
    smoothFlag[note] = true;                           // fade to off
    return;
  }

  bool s; CRGB c;
  decodeVelocity(velocity, s, c);
  targets[note] = c;
  smoothFlag[note] = s;

  if (!s) {
    leds[note] = targets[note];                        // snap instantly
  }
}

void handleNoteOff(uint8_t channel, uint8_t note) {
  if (channel < 2) {
    leds[channel] = CRGB(0, 0, 0);
    FastLED.show();
    Serial.printf("Note Off: Channel %d, Note %d\n", channel + 1, note); 
  }
}

void handleControlChange(uint8_t channel, uint8_t number, uint8_t value) {
  statusLed(1);
  if( number == 100) {ccSendRate = value; return;}

  else if( number == 101) {
    touchChargeTime = (int)value*10;
    touchSetCycles( touchChargeTime, touchSleepTime);
    delay(50);
    resetTouchBaselines();
  } else if( number == 102) {
    touchSleepTime = (int)value*10;
    touchSetCycles( touchChargeTime, touchSleepTime);
    delay(50);  
    resetTouchBaselines();
  } 
  
  else if( number == 110) {
    cc[5].alpha = (float)value/127;
  } else if( number == 111) {
    cc[6].alpha = (float)value/127;
  } else if( number == 112) {
    cc[7].alpha = (float)value/127;
  } 

  else if( number == 127) {
    ESP.restart(); // Clean restart  
  } 

}


/************** LOOP ******************/

byte currentSensor = 0;
int measureCycles = 100;   // Default measure cycles
int sleepCycles = 20;      // Default sleep cycles

void loop() {
  //imuLoop();
  statusLed(0);
  processCCBuffer();

  // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 10; 
  static byte prevValue[] = {0,0,0,0,0};
  static byte num = 0;

  if(millis()-timer > interval){
    timer= millis();
    tickSmoothingDT(interval);
    FastLED.show();

    // handleNoteOn(1, num%10, random(64)+64);
    // num++; 
    
    for(int i=0;i<9;i++){
      cap[i].update();
    }

    for(byte i=0;i<2;i++) readEncoder(i);

    //cc[0].send(analogRead(11));
   // cc[3].send(cap[10].getValue());
    //cc[4].send(cap[9].getValue());
    //cc[5].send(cap[10].getValue());
    
  
    for(int i=0;i<9;i++){
      if( cap[i].state == true && cap[i].getChange() == true){
        sendMidiNoteOn( i, touchToMidi(cap[i].getValue()) );
        // if(startup_leds){
        //   leds[0] = CRGB(255, 0, 0);
        //   leds[1] = CRGB(255, 0, 0);
        // }
        statusLed(2);
        FastLED.show();
      } else if( cap[i].state == false && cap[i].getChange() == true){
        sendMidiNoteOff( i );
        // if(startup_leds){
        //   leds[0] = CRGB(0, 255, 0);
        //   leds[1] = CRGB(0, 255, 0);
        // }
        // FastLED.show();
      }
    }

    esp_task_wdt_reset();
    processIncomingMidi();
  }//timer
        
  return;
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


uint8_t touchToMidi(int val){
  int min = velocityDepth;
  int max = 2000-velocityDepth;
  if(min>max) min = max;
  val = constrain(val,min,max);
  val = map(val, min,max, 0,127);
  return uint8_t(constrain(val,0,127));
}

void resetTouchBaselines() {
  for(int i=0; i<11; i++) {
    for(int j=0; j<100; j++) {
      cap[i].update();
      delay(1);
    }
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

void readEncoder(byte num){
  int val = enc[num].delta(); //get encoder  count
  if(val!= 0){
    if(SERIAL_DEBUG){
      Serial.print("count: ");
      Serial.println(val);
    }
    else{
      sendMidiCC(enc[num].ccNumber, 64+val);
    }
  }
}

// call with milliseconds since last frame (compute via millis())
void tickSmoothingDT(uint16_t dt_ms) {
  // time constant controls “speed” (smaller = faster)
  const float tau_ms = 120.0f; // adjust to taste
  // convert to an 8-bit blend amount ≈ 255*(1 - e^(-dt/tau))
  float frac = 1.0f - expf(-float(dt_ms)/tau_ms);
  uint8_t amt = (uint8_t)constrain(lroundf(frac * 255.0f), 1, 255);

  for (uint16_t i = 0; i < NUM_LEDS; ++i) {
    if (smoothFlag[i]) {
      nblend(leds[i], targets[i], amt);
      if (leds[i] == targets[i]) smoothFlag[i] = false;
    }
  }
}