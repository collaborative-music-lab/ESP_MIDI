#include "USB_MIDI.h"
#include "LSM6.h"
#include "potentiometer.h"
#include "buttons.h"
#include "led.h"

const byte ENABLE_USB_MIDI = 0;

LSM6 imu;

const byte numPots = 4;
//for potentiometers the first argument is the pin number, the second is a flag if the pot is reversed
Potentiometer pots[numPots] = {Potentiometer(2), Potentiometer(9, 1), Potentiometer(8, 1), Potentiometer(6)};

const byte numButtons = 3;
Button buttons[numButtons] = { Button(3), Button(4), Button(5) };

LED led(11,12,13);

void setup() {
  Serial.begin(115200);
  if( ENABLE_USB_MIDI) usbMidiSetup();
  
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  if( ENABLE_USB_MIDI != 1) Serial.println("USB MIDI not enabled");

  imuSetup();
}

void loop() {
  // Handle MIDI input
  pollMidiInput();
  imuLoop();
  led.loop();

  for(byte i=0;i<numPots;i++){
    int val = pots[i].read();
    if( val >= 0 ) {
      Serial.println("Pot " + String(i) + ": " + String(val));
      if(i==0) led.setRed(val*2);
      if(i==1) led.setGreen(val*2);
      if(i==2) led.setBlue(val*2);
    }
  }

  for(byte i=0;i<numButtons;i++){
    buttons[i].loop();
    if( buttons[i].isPressed()) Serial.println("Button " + String(i) + " pressed ");
    else if( buttons[i].isReleased()) Serial.println("Button " + String(i) + " released ");
  }



  // Handle MIDI output
  if (millis() - last_midi_time >= tempo) {
    sendMidiNote();
    // uint16_t val = (uint16_t)sensor.getDistance();
    // val = val < 30 ? 0 : val > 500 ? 127 : (val-30)/4;
    // sendMidiCC(val);
    last_midi_time = millis();
  }

  static uint32_t timer = 0;
  if(millis()-timer > 10){
    timer = millis();
    // uint16_t val = (uint16_t)sensor.getDistance();
    // val = val < 30 ? 0 : val > 500 ? 127 : (val-30)/4;
    // sendMidiCC(val);

  }
}

void pollMidiInput() {
  uint8_t packet[4];
  while (tud_midi_available()) {
    if (tud_midi_packet_read(packet)) {
      USB_MIDI_t *m = (USB_MIDI_t *)packet;
      Serial.printf("%lld: Cable: %d Code: %01hhX, Data: %02hhX %02hhX %02hhX\n",
                    esp_timer_get_time(), m->cable_number, m->code_index_number,
                    m->MIDI_0, m->MIDI_1, m->MIDI_2);
    }
  }
}
