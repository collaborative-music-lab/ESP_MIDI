/*

This file initializes USB MIDI, and handles sending and receiving MIDI messages. 

You will never need to modify this file.

*/
#include "USB.h"
#include "USBMIDI.h"
USBMIDI usbMIDI;

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
#define MIDI_CLOCK_TICK 0xF8
#define MIDI_START 0xFA
#define MIDI_CONTINUE 0xFB
#define MIDI_STOP 0xFC


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

  usbMIDI.begin();
  USB.begin();
}

// ========================
// MIDI Note Send Throttle
// ========================

struct MidiNoteMessage {
  byte note;
  byte velocity;
};

// Configurable max send interval
unsigned long maxSendIntervalMs = 2;

// Send timing state
unsigned long lastSendTime = 0;

// Fixed-size buffer
const int midiBufferSize = 128;
MidiNoteMessage midiNoteBuffer[midiBufferSize];
volatile int bufferHead = 0;
volatile int bufferTail = 0;

// Add message to buffer
void sendMidiNoteOn(byte note, byte velocity) {
  velocity = constrain(velocity, 0, 127);

  int nextHead = (bufferHead + 1) % midiBufferSize;
  if (nextHead == bufferTail) {
    // Buffer full — could add overflow handling here
    if( SERIAL_DEBUG) Serial.println("MIDI buffer full! Dropping note.");
    return;
  }

  midiNoteBuffer[bufferHead].note = note;
  midiNoteBuffer[bufferHead].velocity = velocity;
  bufferHead = nextHead;
}

void sendMidiNoteOff(byte note) {
  usbMIDI.noteOff(note, 1);
}

// Send one message if enough time has passed
void processMidiSendQueue() {
  if (bufferTail != bufferHead && millis() - lastSendTime >= maxSendIntervalMs) {
    MidiNoteMessage msg = midiNoteBuffer[bufferTail];
    bufferTail = (bufferTail + 1) % midiBufferSize;

    if( msg.velocity > 0) usbMIDI.noteOn(msg.note, msg.velocity, 1);  // channel 1
    else sendMidiNoteOff( msg.note);
    lastSendTime = millis();
  }
}


uint8_t ccBuffer[64];

void sendMidiCC(uint8_t num, uint8_t val) {
  if(num >=64) return;
  val = constrain(val, 0, 127);
  ccBuffer[num] = val;
  //usbMIDI.controlChange(num, val);
}

void processCCBuffer(){
  static uint32_t timer = 0;
  static byte ccIndex = 0;
  if(millis()-timer > ccSendRate){
    timer = millis();

    for(byte i=0;i<64;i++){
      byte curIndex = (ccIndex+i) %64;
      if(ccBuffer[ curIndex] < 255 ){
        usbMIDI.controlChange(curIndex, ccBuffer[ curIndex]);
        ccBuffer[ curIndex] = 255;
        ccIndex = (ccIndex+1) % 64;
        return;
      }
    }
  }
  //usbMIDI.controlChange(num, val);
}

void sendMidiClockMessage(uint8_t msg) {
  if (tud_midi_mounted()) {
    tud_midi_stream_write(0, &msg, 1);
  }
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
