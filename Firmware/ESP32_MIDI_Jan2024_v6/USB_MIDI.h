/*

This file initializes USB MIDI, and handles sending and receiving MIDI messages. 

You will never need to modify this file.

*/
#include "USB.h"
#include "USBMIDI.h"
USBMIDI usbMIDI;

// Define custom device and manufacturer names
#define MIDI_DEVICE_NAME "Creativitas Robin" 
#define MIDI_MANUFACTURER_NAME "Ian Hattwick"

/** TinyUSB descriptors **/
/*
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
    // Add custom string descriptors
    //uint8_t manufacturer_index = tinyusb_add_string_descriptor(custom_manufacturer_name);
    //uint8_t product_index = tinyusb_add_string_descriptor(custom_device_name);

    // Get a free duplex endpoint for MIDI
    uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
    TU_VERIFY(ep_num != 0);

    // Create the MIDI descriptor with custom manufacturer and product indices
    // uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
    //     TUD_MIDI_DESCRIPTOR(*itf, product_index, ep_num, (uint8_t)(0x80 | ep_num), 64)};
    uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
    TUD_MIDI_DESCRIPTOR(*itf, 1, ep_num, (uint8_t)(0x80 | ep_num), 64)
};
    
    *itf += 1;

    // Copy the descriptor into the destination buffer
    memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
    return TUD_MIDI_DESC_LEN;
}
*/

// From usb.org MIDI 1.0 specification. This 4 byte structure is the unit
// of transfer for MIDI data over USB.
typedef struct __attribute__((__packed__)) {
  uint8_t code_index_number : 4;
  uint8_t cable_number : 4;
  uint8_t MIDI_0;
  uint8_t MIDI_1;
  uint8_t MIDI_2;
} USB_MIDI_t;

// Basic MIDI Messages
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define CC 0xB0

static uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
static uint8_t const channel = 0;   // 0 for channel 1
static uint32_t note_pos = 0;
unsigned long last_midi_time = 0;
const int tempo = 112; // Milliseconds per note

// Extern declarations for handlers (defined in the .ino file)
extern void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
extern void handleNoteOff(uint8_t channel, uint8_t note);
extern void handleControlChange(uint8_t channel, uint8_t number, uint8_t value);

void usbMidiSetup(){
  
  //tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor);
  //USB.begin();
  
  usbMIDI.begin();
  USB.begin();
}

void sendMidiNoteOn(byte note, byte velocity) {
  // if (tud_midi_mounted()) {
  //   uint8_t note_on[3] = {NOTE_ON | channel, note, velocity};
  //   tud_midi_stream_write(cable_num, note_on, 3);
  // }
  usbMIDI.noteOn(note,velocity, 1);
}

void sendMidiNoteOff(byte note) {
  // if (tud_midi_mounted()) {
  //   uint8_t note_off[3] = {NOTE_OFF | channel, note, 0};
  //   tud_midi_stream_write(cable_num, note_off, 3);
  // }
  usbMIDI.noteOff(note, 1);
}

void sendMidiCC(uint8_t num, uint8_t val) {
  // if (tud_midi_mounted()) {
  //   uint8_t cc[3] = {CC | channel, num, val};
  //   tud_midi_stream_write(cable_num, cc, 3);
  // }
}

void processIncomingMidi() {
  /*
  uint8_t buffer[4]; // Buffer to hold raw MIDI packet data (4 bytes)

  while (tud_midi_available()) {
    if (tud_midi_packet_read(buffer)) {
      //led[0].color = [255,255,255];
      USB_MIDI_t* packet = reinterpret_cast<USB_MIDI_t*>(buffer);

      uint8_t status = packet->MIDI_0 & 0xF0; // Extract the message type
      uint8_t channel = packet->MIDI_0 & 0x0F; // Extract the channel
      uint8_t data1 = packet->MIDI_1;         // Note number or CC number
      uint8_t data2 = packet->MIDI_2;         // Velocity or CC value

      // Call the external handlers
      switch (status) {
        case NOTE_ON:
          if (data2 > 0) {
            handleNoteOn(channel, data1, data2);
          } else {
            handleNoteOff(channel, data1);
          }
          break;

        case NOTE_OFF:
          handleNoteOff(channel, data1);
          break;

        case CC:
          handleControlChange(channel, data1, data2);
          break;

        default:
          // Handle other MIDI messages if needed
          break;
      }
    }
    //led[0].color = [0,0,0];
  }
  */
}

