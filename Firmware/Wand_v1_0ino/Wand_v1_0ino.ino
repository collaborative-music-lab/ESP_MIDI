/*
  File Name: Wave_v1_0.ino
  Version: v1_0
  Creator: Ian Hattwick
  Date: Jun 30, 2025

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

#include <Wire.h>
#include "USB_MIDI.h"
#include "potentiometer.h"
#include "buttons.h"
#include "controller.h"
#include "src/LSM6.h"

#include "parameters.h"
#include <Preferences.h>
Preferences preferences;
ParameterObject parameters;

#include <FastLED.h>

LSM6 imu;

//CRGB leds[2]; // Array for LEDs on pin 43
//CRGB built_in[1]; // Array for LEDs on pin 21
bool startup_leds = true; 

int monitorInput = -1;
/************** DEFINE INPUTS ******************/
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[] =  {Potentiometer(1), Potentiometer(2)};

Button buttons[] = {Button(7), Button(8), Button(9), Button(10), Button(0)}; //pin x is the boot button
 
//(int ccNumber, int minInterval = 20, int inLow = 0, int inHigh = 127, int deltaThreshold = 2, float alpha = 0.2))
CController cc[] = {
  CController(0, 50, 1900, 3550, 3, .5), //hall1
  CController(1, 50, 1900, 3550, 3, .5), //hall2
  CController(2, 100, 100, 3900, 10, .4), //pot1
  CController(3, 100, 100, 3900, 10, .4), //pot2
  CController(4, 100, 20, 400, 10, .4),  //Capsense1
  CController(5, 100, 20, 400, 10, .4), //Capsense2

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

  Wire.begin(6,5);

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();

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

char report[80];

void loop() {
  
  // statusLed(0);

  // // Handle MIDI input
  static uint32_t timer = 0;
  int interval = 100; 

  if(millis()-timer > interval){
    timer= millis();

      imu.read();

      if(1){
        snprintf(report, sizeof(report), "A: %6d %6d %6d    G: %6d %6d %6d",
        imu.a.x, imu.a.y, imu.a.z,
        imu.g.x, imu.g.y, imu.g.z);
        Serial.println(report);
        Serial.print(analogRead(1));
        Serial.print("\t");
        Serial.print(analogRead(2));
        Serial.print("\t");
        Serial.print(digitalRead(7));
        Serial.print("\t");
        Serial.print(digitalRead(8));
        Serial.print("\t");
        Serial.print(digitalRead(9));
        Serial.print("\t");
        Serial.print(digitalRead(10));
        Serial.print("\t");
        Serial.print(digitalRead(0));
        Serial.println();
      }

      else{

        //cc[0].send(analogRead(12));
        cc[0].send(analogRead(1)); //12    2
        cc[1].send(analogRead(2)); // 5     9
        cc[2].send(digitalRead(7)); //11    3
        cc[3].send(digitalRead(8)); //10    4
        cc[4].send(digitalRead(9)); //4       10
        cc[5].send(digitalRead(10)); //7       7
      }

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