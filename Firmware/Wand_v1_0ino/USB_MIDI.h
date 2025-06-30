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
  usbMIDI.controlChange(num, val);
  // if (tud_midi_mounted()) {
  //   uint8_t cc[3] = {CC | channel, num, val};
  //   tud_midi_stream_write(cable_num, cc, 3);
  // }
  usbMIDI.controlChange(num, val, 1);
}

void processIncomingMidi() {
  

  midiEventPacket_t midi_packet_in = {0, 0, 0, 0};

  if (usbMIDI.readPacket(&midi_packet_in)) {
    uint8_t cable_num = MIDI_EP_HEADER_CN_GET(midi_packet_in.header);
    midi_code_index_number_t code_index_num = MIDI_EP_HEADER_CIN_GET(midi_packet_in.header);

    switch (code_index_num) {
      case MIDI_CIN_MISC:        Serial.println("This a Miscellaneous event"); break;
      case MIDI_CIN_CABLE_EVENT: Serial.println("This a Cable event"); break;
      case MIDI_CIN_SYSCOM_2BYTE:  // 2 byte system common message e.g MTC, SongSelect
      case MIDI_CIN_SYSCOM_3BYTE:  // 3 byte system common message e.g SPP
        Serial.println("This a System Common (SysCom) event");
        break;
      case MIDI_CIN_SYSEX_START:      // SysEx starts or continue
      case MIDI_CIN_SYSEX_END_1BYTE:  // SysEx ends with 1 data, or 1 byte system common message
      case MIDI_CIN_SYSEX_END_2BYTE:  // SysEx ends with 2 data
      case MIDI_CIN_SYSEX_END_3BYTE:  // SysEx ends with 3 data
        Serial.println("This a system exclusive (SysEx) event");
        break;
      case MIDI_CIN_NOTE_ON:{
        handleNoteOn(midi_packet_in.byte1 & 0x0F, midi_packet_in.byte2, midi_packet_in.byte3);
        break;
      }
      case MIDI_CIN_NOTE_OFF:       
        handleNoteOff(midi_packet_in.byte1 & 0x0F, midi_packet_in.byte2);
        break;
      case MIDI_CIN_POLY_KEYPRESS: Serial.printf("This a Poly Aftertouch event for Note %d and Value %d\n", midi_packet_in.byte2, midi_packet_in.byte3); break;
      case MIDI_CIN_CONTROL_CHANGE:
        handleControlChange(midi_packet_in.byte1 & 0x0F, midi_packet_in.byte2, midi_packet_in.byte3);
        break;
      case MIDI_CIN_PROGRAM_CHANGE:   Serial.printf("This a Program Change event with a Value of %d\n", midi_packet_in.byte2); break;
      case MIDI_CIN_CHANNEL_PRESSURE: Serial.printf("This a Channel Pressure event with a Value of %d\n", midi_packet_in.byte2); break;
      case MIDI_CIN_PITCH_BEND_CHANGE:
        Serial.printf("This a Pitch Bend Change event with a Value of %d\n", ((uint16_t)midi_packet_in.byte2) << 7 | midi_packet_in.byte3);
        break;
      case MIDI_CIN_1BYTE_DATA: Serial.printf("This an embedded Serial MIDI event byte with Value %X\n", midi_packet_in.byte1); break;
    }
  }
}
