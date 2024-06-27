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

// Example melody stored as an array of note values
uint8_t const note_sequence[] = {
    74, 78, 81, 86,  90, 93, 98, 102, 57, 61,  66, 69, 73, 78, 81, 85,
    88, 92, 97, 100, 97, 92, 88, 85,  81, 78,  74, 69, 66, 62, 57, 62,
    66, 69, 74, 78,  81, 86, 90, 93,  97, 102, 97, 93, 90, 85, 81, 78,
    73, 68, 64, 61,  56, 61, 64, 68,  74, 78,  81, 86, 90, 93, 98, 102};

static uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
static uint8_t const channel = 0;   // 0 for channel 1
static uint32_t note_pos = 0;
unsigned long last_midi_time = 0;
const int tempo = 112; // Milliseconds per note


void usbMidiSetup(){
  tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor);
  USB.begin();
}

void sendMidiNote() {
  int previous = note_pos - 1;
  if (previous < 0) {
    previous = sizeof(note_sequence) - 1;
  }

  if (tud_midi_mounted()) {
    uint8_t note_on[3] = {NOTE_ON | channel, note_sequence[note_pos], 127};
    tud_midi_stream_write(cable_num, note_on, 3);

    uint8_t note_off[3] = {NOTE_OFF | channel, note_sequence[previous], 0};
    tud_midi_stream_write(cable_num, note_off, 3);
  }

  note_pos++;
  if (note_pos >= sizeof(note_sequence)) {
    note_pos = 0;
  }
}

void sendMidiCC(uint8_t val) {
  if (tud_midi_mounted()) {
    uint8_t cc[3] = {CC | channel, 0, val};
    tud_midi_stream_write(cable_num, cc, 3);
  }
}