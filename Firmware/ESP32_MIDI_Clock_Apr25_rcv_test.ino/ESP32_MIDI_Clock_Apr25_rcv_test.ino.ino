#include <WiFi.h>
#include <esp_now.h>
#include "packets.h"
#include "midi.h"
#include "clock.h"


void setup() {
  // Initialize USB Serial for debug output.
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for USB Serial to be ready.
  }
  Serial.println("ESP32 Receiver starting...");

  // Initialize the UART for MIDI output on pin 5.
  // We won't use an RX pin so set it to -1.
  MidiSerial.begin(31250, SERIAL_8N1, -1, 17);
  Serial.println("MIDI Serial output initialized on pin 5 at 31250 baud");

  // Set WiFi mode to STA (station mode) to use ESP‑NOW.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Ensure we disconnect from any AP.

  // Initialize ESP‑NOW.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP‑NOW");
    return;
  }
  Serial.println("ESP‑NOW initialized");

  // Register the receive callback function.
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Receive callback registered");
 //pinMode(33,OUTPUT);
  timerSetup();
}

void loop() {
  timerLoop();
  //digitalWrite(33,HIGH);
  // The loop remains empty as all handling is done in the callback.
  delay(1);
  //digitalWrite(33,LOW);
  //delay(100);
}