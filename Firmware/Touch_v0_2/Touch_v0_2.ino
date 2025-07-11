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
const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

#include "USB_MIDI.h"
#include "potentiometer.h"
#include "controller.h"

#include "parameters.h"
#include <Preferences.h>
#include "src/m370_MPR121.h"
Preferences preferences;
ParameterObject parameters;

#include <FastLED.h>
//CRGB leds[2]; // Array for LEDs on pin 43
//CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 

//#include "capsense.h"

 m370_MPR121 mpr121(43,44); //SDA,SCL


int monitorInput = -1;
/************** DEFINE INPUTS ******************/
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2))
CController cc[] = {
  CController(0, 50, 0, 4095, 5, .3), //hall1
  CController(1, 50, 0, 4095, 5, .3), //hall2
  CController(2, 100, 10, 4000, 4, .2), //pot1
  CController(3, 100, 10, 4000, 4, .2), //pot2
  CController(4, 100, 20, 400, 10, .2),  //Capsense1
  CController(5, 100, 20, 400, 10, .2), //Capsense2

  //MPR121 cc[6] to cc[17]
  CController(10, 50, 0, 2000, 10, .2),
  CController(11, 50, 0, 2000, 10, .2),
  CController(12, 50, 0, 2000, 10, .2),
  CController(13, 50, 0, 2000, 10, .2),
  CController(14, 50, 0, 2000, 10, .2),
  CController(15, 50, 0, 2000, 10, .2),
  CController(16, 50, 0, 2000, 10, .2),
  CController(17, 50, 0, 2000, 10, .2),
  CController(18, 50, 0, 2000, 10, .2),
  CController(19, 50, 0, 2000, 10, .2),
  CController(20, 50, 0, 2000, 10, .2),
  CController(21, 50, 0, 2000, 10, .2)
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

  //setupFingeringToMidiNote();

  m370_cap_begin();

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

byte currentSensor = 0;
int measureCycles = 100;   // Default measure cycles
int sleepCycles = 20;      // Default sleep cycles

void loop() {
  
  // statusLed(0);

  // // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 50; 
  static byte prevValue[] = {0,0,0,0,0};

  static int hallEffectRange[2][2] = {
    {4000,0}, {4000,0}
  };
  int hallVal[2];
  hallVal[0] = analogRead(2);
  hallVal[1] = analogRead(9);

  if(0){
    Serial.print( touchRead(4) );
    Serial.print( "\t" );
    Serial.print( touchRead(7) );
    Serial.print( "\t" );
    Serial.println();
  }
  

  if(millis()-timer > interval){
    timer= millis();

    for(byte i=0;i<2;i++){
      if(hallVal[i] < hallEffectRange[i][0]){
        hallEffectRange[i][0] = hallVal[i];
      } else if(hallVal[i] > hallEffectRange[i][1]){
        hallEffectRange[i][1] = hallVal[i];
      }
      hallVal[i]  = map(hallVal[i], hallEffectRange[i][0], hallEffectRange[i][1], 0, 4095);
          //ccValue = constrain(ccValue, 0, 511);
    }

    //cc[0].send(analogRead(12));
    cc[0].send(hallVal[0]); //12    2
    cc[1].send(hallVal[1]); // 5     9
    cc[2].send(analogRead(10)); //11    3
    cc[3].send(analogRead(8)); //10    4
    cc[4].send(touchRead(11)/10); //4       10
    cc[5].send(touchRead(7)/10); //7       7

    readCap();
  }

  //     if(ENABLE_USB_MIDI){
  //       for(int i=0;i<10;i++){
  //         cap[i].update();
  //         if( cap[i].state == true && cap[i].getChange() == true){
  //           sendMidiNoteOn( i, touchToMidi(cap[i].getValue()) );
  //           // if(startup_leds){
  //           //   leds[0] = CRGB(255, 0, 0);
  //           //   leds[1] = CRGB(255, 0, 0);
  //           // }
  //           // statusLed(2);
  //           // FastLED.show();
  //         } else if( cap[i].state == false && cap[i].getChange() == true){
  //           sendMidiNoteOff( i );
  //           // if(startup_leds){
  //           //   leds[0] = CRGB(0, 255, 0);
  //           //   leds[1] = CRGB(0, 255, 0);
  //           // }
  //           // 
  //           // built_in[0] = CRGB(0, 0, 0);
  //           // statusLed(2);
  //           // FastLED.show();
  //         }
  //       }
  //     }
        
      
    
    //if( SERIAL_DEBUG ) Serial.println("\t");
  return;
}//loop

void readCap(){
  static byte lastTouchStatus = 0;
  uint16_t touchStatus = mpr121.touched();

  for (uint8_t i = 0; i < 12; i++) {
    bool wasTouched = lastTouchStatus & (1 << i);
    bool isTouched  = touchStatus     & (1 << i);
    lastTouchStatus = touchStatus;

    if (!wasTouched && isTouched) {
      // Touched: 0 → 1
      sendTouch(i, 1);
    } else if (wasTouched && !isTouched) {
      // Released: 1 → 0
      sendTouch(i, 0);
    }
  }
  //cc[6] to cc[17]
        // for(int i=0;i<12;i++){
        //   int filtered = mpr121.baselineData(i) - mpr121.filteredData(i);
        //   //int filtered =  mpr121.filteredData(i);
        //   //cc[i+10].send(filtered);
        //   Serial.print(filtered);
        //   Serial.print("\t");
        // }
        // Serial.println();
}

void sendTouch(byte num, byte val){
  const byte midiNotes[] = {48,50,52,53,55,57,59,60,62,64,65,67,69,71,72};
  if(SERIAL_DEBUG){
    Serial.print("touch ");
    Serial.print(num);
    Serial.print("\t");
    Serial.println(val);
  } else{
    sendMidiNoteOn(midiNotes[num], val*127);
  }
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
  int min = velocityDepth;
  int max = 2000-velocityDepth;
  if(min>max) min = max;
  val = constrain(val,min,max);
  val = map(val, min,max, 0,127);
  return uint8_t(constrain(val,0,127));
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