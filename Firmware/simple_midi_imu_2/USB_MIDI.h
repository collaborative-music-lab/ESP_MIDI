#include "USB.h"
#include "esp32-hal-tinyusb.h"

static const char *TAG = "usbdmidi";

/** TinyUSB descriptors **/
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
  uint8_t str_index = tinyusb_add_string_descriptor("TinyUSB MIDI");
  uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
  TU_VERIFY(ep_num != 0);
  uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
      TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)};
  *itf += 1;
  memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
  return TUD_MIDI_DESC_LEN;
}

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


void usbMidiSetup(){
  tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor);
  USB.begin();
}

void sendMidiNoteOn(byte note, byte velocity) {
  if (tud_midi_mounted()) {
    uint8_t note_on[3] = {NOTE_ON | channel, note, velocity};
    tud_midi_stream_write(cable_num, note_on, 3);
  }
}

void sendMidiNoteOff(byte note) {
  if (tud_midi_mounted()) {
    uint8_t note_off[3] = {NOTE_OFF | channel, note, 0};
    tud_midi_stream_write(cable_num, note_off, 3);
  }
}

void sendMidiCC(uint8_t num, uint8_t val) {
  if (tud_midi_mounted()) {
    uint8_t cc[3] = {CC | channel, num, val};
    tud_midi_stream_write(cable_num, cc, 3);
  }
}