/*
Settings:
CPU frequency: 160Hz (minimize heat when running USB MIDI)
Flash Mode QIO 80MHz
Flash Size: 4MB
Partition: Default
PSRAM: QSPI PSRAM
Upload Speed: 460800

*/

const byte ENABLE_USB_MIDI = 0;
const byte SERIAL_DEBUG = 1;

//sensor global variables
byte ccSendRate = 1;

#include "USB_MIDI.h"
#include <Wire.h>
#include "controllers.h"
#include "tines.h"

byte hallPins[] = {1,2,3,4,5,6,7,8,9,10};

// #include <FastLED.h>
// CRGB built_in[1]; // Array for LEDs on pin 21
// bool startup_leds = true; 



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  if( ENABLE_USB_MIDI) usbMidiSetup();
  else Serial.begin(115200);
  if( SERIAL_DEBUG) {
    delay(500);
    while (!Serial && millis() < 5000) {
    delay(10);
    }
    Serial.println("Serial enabled");
    while(0) {
      Serial.println("Serial enabled");
      delay(100);
    }
  }

  analogReadResolution(12);          // 0–4095
  analogSetAttenuation(ADC_6db);    // allows ~0–3.3 V

  for(byte i=0;i<9;i++) {
    pinMode(hallPins[i], INPUT);
    // analogSetPinAttenuation(hallPins[i], ADC_6db);
  }

  // FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  // FastLED.setBrightness(25);
  // built_in[0] = CRGB(0, 0, 20);
  // FastLED.show();
  Serial.println("foo");

}//setup

// the loop function runs over and over again forever
void loop() {
  // Serial.println("LED_BUILTIN, HIGH");  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
  // Serial.println("LED_BUILTIN, LOW");   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second

  //  for(byte i=0;i<4;i++) {
  //     button[i].loop();
  //      if( button[i].isPressed() ){
  //       sendMidiNoteOn( i, 127 );
  //       if( SERIAL_DEBUG ) Serial.println(i);
  //       // statusLed(2);
  //       // FastLED.show();
  //     } else if( button[i].isReleased() ){
  //       if( SERIAL_DEBUG ) Serial.println(i);
  //       sendMidiNoteOff( i );
  //     }

  //   }
  int interval = 5;
  static uint32_t timer = 0;

  static float readings[10];
  static float deltas[10];

  if(millis()-timer > interval){
    timer= millis();
    Serial.print(4000);
    Serial.print(',');
    Serial.print(100);
    Serial.print(',');
    byte toCheck[] = {0,2,5,6,7,8};
    for(byte i=0;i<6;i++) {
      byte p = toCheck[i];
      float val = analogRead(hallPins[p]);
      float delta = (val - readings[i]);
      deltas[i] = (delta + deltas[i])*.95;
      readings[i] = val;
      Serial.print(val);
      Serial.print('\t');
      // Serial.print(',');
      // Serial.print(deltas[i]*5 + 1000);
      // Serial.print(',');
    }
    Serial.println();
    
  }//timer
}
