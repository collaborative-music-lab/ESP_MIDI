#include <WiFi.h>
#include <esp_now.h>

#define WIFI_CHANNEL 1

// Global container for storing peers as added
std::vector<esp_now_peer_info_t> peers;

// Callback function to handle incoming ESPNOW messages from remote devices
// This callback is called for every received ESPNOW data packet.
// void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
//   char macStr[18];
//   snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
//            mac_addr[0], mac_addr[1], mac_addr[2],
//            mac_addr[3], mac_addr[4], mac_addr[5]);

//   Serial.printf("Received broadcast message from: %s, length: %d bytes\n", macStr, len);

//   // Here you can add additional filtering if your announcement message is encapsulated;
//   // for example, checking a specific header or device type.
//   // For this sample, every received broadcast is considered an announcement.

//   // Check if the device already exists in our peer list
//   bool exists = false;
//   for (auto &peer : peers) {
//     if (memcmp(peer.peer_addr, mac_addr, 6) == 0) {
//       exists = true;
//       break;
//     }
//   }

//   // If this is a new device, register it
//   if (!exists) {
//     Serial.printf("New device detected: %s. Adding peer...\n", macStr);
//     esp_now_peer_info_t newPeer;
//     memset(&newPeer, 0, sizeof(newPeer));
//     memcpy(newPeer.peer_addr, mac_addr, 6);
//     newPeer.channel = WIFI_CHANNEL;
//     newPeer.encrypt = false;  // Assuming no encryption for simplicity

//     esp_err_t status = esp_now_add_peer(&newPeer);
//     if (status == ESP_OK) {
//       Serial.printf("Peer %s added successfully.\n", macStr);
//       peers.push_back(newPeer);
//     } else {
//       Serial.printf("Failed to add peer %s. Error: %d\n", macStr, status);
//     }
//   }
// }

void onDataRcv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  if (len < 1) return; // need at least one byte

  // Assume the first byte represents the MIDI message
  uint8_t midiMsg = data[0];

  // Log the received MIDI message value for debugging
  Serial.print("Received MIDI message: 0x");
  Serial.println(midiMsg, HEX);

  // Check for known MIDI realtime messages and forward via usbMIDI
  switch (midiMsg) {
    case 0xF8: // MIDI Timing Clock
      // Forward as noteOff on note 60 on channel 1 (adjust note value as needed)
      //usbMIDI.noteOff(60, 1);
      sendMidiClock();
      Serial.println("Forwarded Timing Clock as noteOff(60, 1)");
      break;
    case 0xFA: // MIDI Start
      // Forward as noteOff on note 61 on channel 1
      //usbMIDI.noteOff(61, 1);
      Serial.println("Forwarded Transport Start as noteOff(61, 1)");
      break;
    case 0xFC: // MIDI Stop
      // Forward as noteOff on note 62 on channel 1
      //usbMIDI.noteOff(62, 1);
      Serial.println("Forwarded Transport Stop as noteOff(62, 1)");
      break;
    default:
      // Handle other messages if desired
      Serial.println("Unrecognized MIDI message; no action taken.");
      break;
  }
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

// Define peer MAC address (replace with the actual MAC address)
uint8_t peerAddress[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};

void addPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;  // use the current WiFi channel
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  } else {
    Serial.println("Peer added successfully");
  }
}

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void addBroadcastPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
  } else {
    Serial.println("Broadcast peer added successfully");
  }
}

void sendMIDImessageToPeers(const uint8_t *message, size_t messageSize) {
  for (auto &peer : peers) {
    esp_err_t status = esp_now_send(peer.peer_addr, message, messageSize);
    // Convert MAC address to a string for output
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             peer.peer_addr[0], peer.peer_addr[1], peer.peer_addr[2],
             peer.peer_addr[3], peer.peer_addr[4], peer.peer_addr[5]);
    if (status == ESP_OK) {
      Serial.printf("MIDI message sent to %s successfully.\n", macStr);
    } else {
      Serial.printf("Error sending message to %s: %d\n", macStr, status);
    }
  }
}