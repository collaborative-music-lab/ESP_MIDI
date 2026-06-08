import usb_midi
import adafruit_midi
from adafruit_midi.note_on import NoteOn
from adafruit_midi.note_off import NoteOff
from adafruit_midi.control_change import ControlChange

# 1. Set up MIDI
# We use the first available MIDI output port
midi = adafruit_midi.MIDI(
    midi_in=usb_midi.ports[0],
    midi_out=usb_midi.ports[1],
    in_channel=(0),
    out_channel=(0),
    debug=False,
)

midi_debug = False

def send_note(note,vel):
    if midi_debug: print('NOTE: ', note,vel)
    if vel > 0:
        if vel > 127: vel = 127
        if note < 0: note = 0
        elif note > 127: note = 127
        midi.send(NoteOn(note,vel)) # Send Middle C
    else:
        midi.send(NoteOff(note, 0))
    
cc_values = [0] * 128
def send_cc(num, val):
    val = int(val)
    if val > 127: val = 127
    elif val < 0: val = 0
    if num < 0: num = 0
    elif num > 127: num = 127
    
    if val != cc_values[num]:
        if midi_debug: print('CC: ', num,val)
#         if num == 12: print(val)
        cc_values[num] = val
        midi.send(ControlChange(num, int(val)))

def check_input():
    msg = midi.receive()
#     print('checking midi input', msg)

    if msg is not None:
        if isinstance(msg, NoteOn):
            if msg.note == 60 and msg.velocity > 0:
                return 1
            print(f"Note On: {msg.note} Velocity: {msg.velocity}")
        elif isinstance(msg, NoteOff):
            print(f"Note Off: {msg.note}")
    return 0