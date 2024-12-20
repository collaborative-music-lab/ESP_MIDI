#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <FastLED.h>
#include "settings.h"

// UDP settings
WiFiUDP udp;
unsigned int localUdpPort = 4210; // Local port to listen on
uint8_t incomingPacket[255];  // Buffer for incoming packets
uint8_t replyPacket[] = "Packet received and mirrored";  // Acknowledgment message
IPAddress local_ip; // Store the local IP address
IPAddress server_ip(192, 168, 5, 40); // Replace with your server's IP address
unsigned int server_port = 8005;


//LED FUNCTIONS
CRGB leds[LED_COUNT];
static bool led1Blinking = false;
static bool led2Blinking = false;

void showStatus(int num, CRGB led) {
    if( num == 0) {
      leds[0] = led;
      led1Blinking = true;
    }
    else if ( num == 1) {
      leds[1] = led;
      led2Blinking = true;
    }
    FastLED.show();
}

void updateLEDs() {
  // Timing variables for non-blocking blinking
  static unsigned long led1_blink_timer = 0;
  static unsigned long led2_blink_timer = 0;
  const unsigned long led_blink_interval = 100; // Blink duration in milliseconds
    unsigned long current_time = millis();
    
    // Check if the blink interval has passed for LED1
    if (led1Blinking && current_time - led1_blink_timer > led_blink_interval) {
        leds[0] = CRGB::Black; // Turn off the LED
        led1Blinking = false;
        FastLED.show();
    }

    // Check if the blink interval has passed for LED2
    if (led2Blinking && current_time - led2_blink_timer > led_blink_interval) {
        leds[1] = CRGB::Black; // Turn off the LED
        led2Blinking = false;
        FastLED.show();
    }
}

// config AP SSID
void configDeviceAP() {
  bool result = WiFi.begin(SSID, WIFI_PW);

  if (!result) {
    Serial.println("Wifi Config failed.");
    showStatus(1, CRGB::Red);
  } else {
    Serial.println("Wifi Config Success. Connected as STA on: " + String(SSID));
    showStatus(1, CRGB::Green);
  }
}//configure AP

void pingServer() {
  static long timer = 0;
  if (millis() - timer > 5000) {
    timer = millis();

    // Convert IPAddress to string
    char ipStr[16];
    sprintf(ipStr, "%d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);

    // Create an OSC message
    OSCMessage msg("/ping");
    msg.add(ipStr); // Add the IP address as a string argument to the OSC message

    // Send the OSC message
    udp.beginPacket(server_ip, server_port);
    msg.send(udp); // Send the OSC message through the UDP connection
    udp.endPacket();

    msg.empty(); // Free the OSC message buffer

    Serial.println("Ping sent to server with IP address:");
    Serial.println(ipStr);
  }
}

String convertOSCToASCII(OSCMessage &msg);

void checkUDP(){
  // Check if there are any incoming packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Read the packet into the buffer
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0; // Null-terminate the string
    }

    // Create an OSCMessage object to decode the incoming packet
    OSCMessage msg;
    msg.fill((uint8_t *)incomingPacket, packetSize);

    // Check if the message is complete and correctly formatted
    if (!msg.hasError()) {
      Serial.println("Received valid OSC message:");
      // Convert OSC message to an ASCII string
      String asciiMessage = convertOSCToASCII(msg);
      asciiMessage += '\n';

      // Send the ASCII message over Serial
      Serial.println("Sending ASCII message over Serial2:");
      Serial.println(asciiMessage);
      Serial2.write(asciiMessage.c_str(), asciiMessage.length());
      showStatus(0, CRGB::Purple);
    } else {
      Serial.println("Received an invalid OSC message.");
    }

    msg.empty(); // Clear the message buffer for the next packet

    // Send a reply, mirroring the received packet
    udp.beginPacket(server_ip, server_port);
    udp.write(incomingPacket, len);
    udp.endPacket();

    // // Optionally send an additional acknowledgment message
    // udp.beginPacket(udp.remoteIP(), udp.remotePort());
    // udp.write(replyPacket);
    // udp.endPacket();
    showStatus(0, CRGB::Blue);
  }
}

String convertOSCToASCII(OSCMessage &msg) {
  String asciiString = "";

  // Add the OSC address to the string
  asciiString += msg.getAddress();
  
  // Add a space after the address
  asciiString += " ";

  // Loop through all arguments and add them to the string
  for (int i = 0; i < msg.size(); i++) {
    if (msg.isInt(i)) {
      asciiString += msg.getInt(i);
    } else if (msg.isFloat(i)) {
      asciiString += msg.getFloat(i);
    } else if (msg.isString(i)) {
      char argStr[255];
      msg.getString(i, argStr, 255);
      asciiString += argStr;
    }

    // Add a space between arguments
    if (i < msg.size() - 1) {
      asciiString += " ";
    }
  }

  return asciiString;
}


void setup() {
  // Start the built-in USB serial (Serial0)
  Serial.begin(115200);
  
  // Start Serial2 with the desired baud rate
  Serial2.begin(115200, SERIAL_8N1, GATEWAY_RXD2, GATEWAY_TXD2);

  FastLED.addLeds<WS2812B, ESP1_LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  showStatus(CRGB::White, CRGB::White);

  WiFi.begin(SSID, WIFI_PW);

  // Wait for connection
  int max_retries = 20; // Define max retries
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < max_retries) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    local_ip = WiFi.localIP();
    Serial.println("\nConnected to Wi-Fi");
    Serial.print("Local IP address: ");
    Serial.println(local_ip);
  } else {
    Serial.println("\nFailed to connect to Wi-Fi");
    // Handle failed connection, reset or retry
    ESP.restart(); // Optionally restart to retry
  }

  // Start UDP
  udp.begin(localUdpPort);

  // Store the local IP address
  local_ip = WiFi.localIP();
  Serial.print("Local IP address: ");
  Serial.println(local_ip);

  Serial.println("ESP32 Wi-Fi to Serial Bridge Initialized");
}

void loop() {
  checkUDP();
  // Check if data is available on Serial (USB)
  if (Serial.available()) {
    // Read the data from Serial
    char incomingByte = Serial.read();
    
    // Transmit the data to Serial2
    Serial2.write(incomingByte);
    showStatus(0, CRGB::Green);
  }

  // Check if data is available on Serial2
  if (Serial2.available()) {
    // Read the data from Serial2
    char incomingByte = Serial2.read();
    
    // Transmit the data to Serial (USB)
    Serial.print(incomingByte);
    showStatus(1, CRGB(255, 165, 0)); // Custom color for orange
  }
  pingServer();
  updateLEDs();
}
