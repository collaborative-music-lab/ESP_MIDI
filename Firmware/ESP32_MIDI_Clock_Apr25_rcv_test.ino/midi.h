// Define the structure for the expected MIDI data.
typedef struct struct_midiData {
  uint8_t status;
  uint8_t data1;
  uint8_t data2;
} struct_midiData;

// Create an instance for the MIDI output UART (using UART1).
// Pin 5 will be used for TX. We are not using an RX, so set it to -1.
HardwareSerial MidiSerial(1);

//
// Callback function that is executed when data is received via ESP‑NOW.
//
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  // Retrieve sender's MAC address from the info structure
  Serial.print("Received data from: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           recv_info->src_addr[0],
           recv_info->src_addr[1],
           recv_info->src_addr[2],
           recv_info->src_addr[3],
           recv_info->src_addr[4],
           recv_info->src_addr[5]);
  Serial.println(macStr);

  // Ensure the incoming data is the expected size.
  if (len == sizeof(struct_midiData)) {
    // Cast incomingData to our structure type.
    struct_midiData *midiData = (struct_midiData *)incomingData;

    // Parse the MIDI status byte for the message type.
    String msgType = "";
    uint8_t status = midiData->status;
    uint8_t data1 = midiData->data1;
    uint8_t data2 = midiData->data2;
    
    if (status >= 0xF0) {
      // System common and real-time messages
      switch(status) {
        case 0xF8:
          msgType = "Clock";
          break;
        case 0xFA:
          msgType = "Transport: Start";
          break;
        case 0xFB:
          msgType = "Transport: Continue";
          break;
        case 0xFC:
          msgType = "Transport: Stop";
          break;
        default:
          msgType = "System Message";
          break;
      }
    } else {
      // Channel Voice Messages
      switch(status & 0xF0) {
        case 0x80:
          msgType = "Note Off";
          break;
        case 0x90:
          msgType = "Note On";
          break;
        case 0xA0:
          msgType = "Polyphonic Aftertouch";
          break;
        case 0xB0:
          msgType = "Control Change (CC)";
          break;
        case 0xC0:
          msgType = "Program Change";
          break;
        case 0xD0:
          msgType = "Channel Pressure";
          break;
        case 0xE0:
          msgType = "Pitch Bend";
          break;
        default:
          msgType = "Unknown";
          break;
      }
    }
    
    // Print the parsed data to the USB serial debug output.
    Serial.print("Message Type: ");
    Serial.print(msgType);
    Serial.print(" | Data1: ");
    Serial.print(data1, DEC);
    Serial.print(" | Data2: ");
    Serial.println(data2, DEC);

    // Send the MIDI message via the Serial MIDI output on pin 5.
    // MIDI messages are usually 3 bytes (status, data1, data2).
    MidiSerial.write(status);
    MidiSerial.write(data1);
    MidiSerial.write(data2);
  } else {
    Serial.print("Received unexpected data length: ");
    Serial.println(len);
  }
}