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
#include "buttons.h"
#include "controllers.h"
#include "encoder.h"


// #include <FastLED.h>
// CRGB built_in[1]; // Array for LEDs on pin 21
// bool startup_leds = true; 

Button button[] = {
  Button(1),
  Button(2),
  Button(3),
  Button(4)
};

Esp32Encoder enc[] = {
  Esp32Encoder (6,5,-1,1,50),//A,B,Switch, Divider, CC number
  Esp32Encoder (7,8,-1,1, 51)
};


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
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

  enc[0].begin([]{enc[0].readEncoder_ISR();});
  enc[1].begin([]{enc[1].readEncoder_ISR();});

  // FastLED.addLeds<WS2811, 21, GRB>(built_in, 1);
  // FastLED.setBrightness(25);
  // built_in[0] = CRGB(0, 0, 20);
  // FastLED.show();

}//setup

// the loop function runs over and over again forever
void loop() {
  // Serial.println("LED_BUILTIN, HIGH");  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
  // Serial.println("LED_BUILTIN, LOW");   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second

   for(byte i=0;i<4;i++) {
      button[i].loop();
       if( button[i].isPressed() ){
        sendMidiNoteOn( i, 127 );
        if( SERIAL_DEBUG ) Serial.println(i);
        // statusLed(2);
        // FastLED.show();
      } else if( button[i].isReleased() ){
        if( SERIAL_DEBUG ) Serial.println(i);
        sendMidiNoteOff( i );
      }

    }

    static uint32_t timer = 0;
    if(millis()-timer > 50){
      timer = millis();
      for(byte i=0;i<2;i++) readEncoder(i);
      //Serial.println();
      // for(byte i=1;i<13;i++) {
      //     Serial.print(digitalRead(i));
      //     Serial.print("\t");
      // }
      // Serial.println();
    }
}

void readEncoder(byte num){
  int val = enc[num].delta(); //get encoder  count
  //Serial.print(num);
  //Serial.print(val);
  if(val != 0){
    if(SERIAL_DEBUG){
      Serial.print(enc[num].ccNumber);
      Serial.print(" ");
      Serial.print("count: ");
      Serial.println(val);
    }
    else{
      sendMidiCC(enc[num].ccNumber, 64+val);
    }
  }
}