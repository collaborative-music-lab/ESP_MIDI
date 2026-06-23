import usb_midi
import adafruit_midi
from adafruit_midi.note_on import NoteOn
from adafruit_midi.note_off import NoteOff
from adafruit_midi.control_change import ControlChange

midi_debug = False

class MidiHandler:
    def __init__(self, channel = 0 ):
        self.midi = adafruit_midi.MIDI(
            midi_in=usb_midi.ports[0],
            midi_out=usb_midi.ports[1],
            in_channel=(channel),
            out_channel=(channel),
            debug=False,
            )

        self.on_note_on = lambda note, velocity: print(f"Note On {msg.note} Velocity: {msg.velocity}")
        self.on_note_off = lambda note, velocity: print(f'Note Off {note} Velocity {velocity}')
        self.on_control_change = lambda num, val: print(f'CC {num} Value {val}')
#         self.on_note_on = None
#         self.on_note_off = None
#         self.on_control_change = None
        self.cc_values = [0] * 128
    
    def update(self):
        """Call this in your main loop"""
        msg = self.midi.receive()
        
        if msg is not None:
            if isinstance(msg, NoteOn):
                if self.on_note_on:
                    self.on_note_on(msg.note, msg.velocity)
            elif isinstance(msg, NoteOff):
                if self.on_note_off:
                    self.on_note_off(msg.note, msg.velocity)
            elif isinstance(msg, ControlChange):
                if self.on_control_change:
                    self.on_control_change(msg.controller, msg.value)
                    

    def send_note(self, note,vel,channel = 0):
        if midi_debug: print('NOTE: ', note,vel)
        if vel > 0:
            if vel > 127: vel = 127
            if note < 0: note = 0
            elif note > 127: note = 127
            self.midi.send(NoteOn(note,vel),channel) # Send Middle C
        else:
            self.midi.send(NoteOff(note, 0),channel)
        
    def send_cc(self, num, val,channel = 0):
        val = int(val)
        if val > 127: val = 127
        elif val < 0: val = 0
        if num < 0: num = 0
        elif num > 127: num = 127
        
        if val != self.cc_values[num]:
            if midi_debug: print('CC: ', num,val)
    #         if num == 12: print(val)
            self.cc_values[num] = val
            self.midi.send(ControlChange(num, int(val)),channel)
            
    def force_send_cc(self, num, val,channel = 0):
        val = int(val)
        if val > 127: val = 127
        elif val < 0: val = 0
        if num < 0: num = 0
        elif num > 127: num = 127
        
        if midi_debug: print('Force CC: ', num,val)
        self.cc_values[num] = val
        self.midi.send(ControlChange(num, int(val)), channel)
