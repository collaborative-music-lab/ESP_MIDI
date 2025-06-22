#include <string.h>

// --- Packet Type Definitions ---
#define ANNOUNCEMENT_PACKET  0x01
#define HEARTBEAT_PACKET     0x02

// --- Announcement Packet Structure ---
struct AnnouncementPacket {
  uint8_t packetType;      // Identifier: 0x01 for announcement
  uint8_t macAddr[6];      // Device's MAC address (included for redundancy)
  uint8_t deviceType;      // Device role/type (e.g., 0x01 for MIDI client)
  uint8_t firmwareMajor;   // Firmware version (major)
  uint8_t firmwareMinor;   // Firmware version (minor)
  uint8_t capabilities;    // Bitfield for supported features
  char deviceName[16];     // Human-friendly name (optional)
  // You could include additional fields (e.g., timestamp) if needed
};

// --- Heartbeat Packet Structure ---
struct HeartbeatPacket {
  uint8_t packetType;      // Identifier: 0x02 for heartbeat
  uint8_t macAddr[6];      // Device's MAC address (redundant but useful)
  uint16_t sequence;       // Sequence number (helps detect missed heartbeats)
  uint32_t uptime;         // Device uptime in seconds (or milliseconds)
  uint8_t status;          // Status flags (e.g., errors, battery low)
  uint8_t batteryLevel;    // Battery level percentage (if applicable)
};

// --- Global Variables ---
uint16_t heartbeatSequence = 0;  // Sequence counter for heartbeats

// --- Helper: Fill the local device's MAC into a buffer ---
void getLocalMAC(uint8_t *mac) {
  // Retrieve the MAC address as a string (e.g., "AA:BB:CC:DD:EE:FF")
  String macStr = WiFi.macAddress();  
  int values[6] = { 0 };

  // Convert the MAC string into six integer values
  if (6 == sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
                  &values[0], &values[1], &values[2],
                  &values[3], &values[4], &values[5])) {
    // Copy the values into the byte array
    for (int i = 0; i < 6; i++) {
      mac[i] = (uint8_t) values[i];
    }
  } else {
    Serial.println("Failed to parse MAC address!");
  }
}

// --- Function to Send an Announcement Packet ---
void sendAnnouncementPacket() {
  AnnouncementPacket annPkt;
  annPkt.packetType = ANNOUNCEMENT_PACKET;
  getLocalMAC(annPkt.macAddr);
  annPkt.deviceType     = 0x01;   // For example: 0x01 = MIDI client
  annPkt.firmwareMajor  = 1;
  annPkt.firmwareMinor  = 0;
  annPkt.capabilities   = 0x00;   // Set capability flags as needed
  strncpy(annPkt.deviceName, "ESP-MIDI", sizeof(annPkt.deviceName));

  // Broadcast address: all 0xFF's
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&annPkt, sizeof(annPkt));
  if (result == ESP_OK) {
    Serial.println("Announcement packet sent successfully");
  } else {
    Serial.printf("Error sending announcement: %d\n", result);
  }
}

// --- Function to Send a Heartbeat Packet ---
void sendHeartbeatPacket() {
  HeartbeatPacket hbPkt;
  hbPkt.packetType = HEARTBEAT_PACKET;
  getLocalMAC(hbPkt.macAddr);
  hbPkt.sequence = heartbeatSequence++;
  hbPkt.uptime   = millis() / 1000;  // uptime in seconds
  hbPkt.status   = 0x00;             // Set any status flags as needed
  hbPkt.batteryLevel = 100;          // Example: Assume 100% (or measure if necessary)

  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&hbPkt, sizeof(hbPkt));
  if (result == ESP_OK) {
    Serial.println("Heartbeat packet sent successfully");
  } else {
    Serial.printf("Error sending heartbeat: %d\n", result);
  }
}