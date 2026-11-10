import usb_midi
import adafruit_midi
from adafruit_midi.note_on import NoteOn
from adafruit_midi.note_off import NoteOff
from adafruit_midi.control_change import ControlChange
from adafruit_midi.system_exclusive import SystemExclusive

# 1. Set up MIDI
# We use the first available MIDI output port
midi = adafruit_midi.MIDI(
    midi_in=usb_midi.ports[0],
    midi_out=usb_midi.ports[1],
    in_channel=(0),
    out_channel=(0),
    debug=False,
)

def msg_to_sysex(msg):
    """Convert a message list to 7-bit values for MIDI SysEx"""
    sysex_data = " ".join(str(item) for item in msg)
    encoded = sysex_data.encode('utf-8')
    
    # Add delimiters
    delimiter_start = b'\x00'
    delimiter_end = b'|'
    return encoded + delimiter_end

def send_sysex(data, manufacturer_id=0x01):
    # manufacturer_id: single byte (int) or 3 bytes (list/bytearray)
    # data: list or bytearray of 7-bit values (0-127)
    
    sysex = SystemExclusive(manufacturer_id, msg_to_sysex(data))
    midi.send(sysex)

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
        
# def send_msg (name, msg):

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

def send_param(name, instance, param, value):
    # send an automatonism parameter message over midi sysex. 
    if not isinstance(name, str):
        raise TypeError(f"name must be a string, got {type(name).__name__}")
    if not isinstance(param, str):
        raise TypeError(f"param must be a string, got {type(param).__name__}")
    if not isinstance(instance, int):
        raise TypeError(f"instance must be an integer, got {type(instance).__name__}")
    if not isinstance(value, (int, float)):
        raise TypeError(f"value must be an integer or float, got {type(value).__name__}")
    
    # Format checks
    if name != name.lower():
        raise ValueError(f"name must be lowercase, got '{name}'")
    if param != param.upper():
        raise ValueError(f"param must be all uppercase, got '{param}'")
    
    # Now use the validated parameters
    send_sysex([name, instance, param, value])
    
def send_event(name, data):
    # send an arbitrary message over midi sysex. 
    if not isinstance(name, str):
        raise TypeError(f"name must be a string, got {type(name).__name__}")
    
    # Now use the validated parameters
    send_sysex(["send", name, data ])
    
def send_mapping(midi_type, num, name, instance, param, min=0, max=127):
    """sends a message to PD to associate a midi message with a parameter change."""
    if midi_type != "cc" and midi_type != "note":
        raise TypeError(f"type must be cc or note, got", midi_type)
    if not isinstance(num, int):
        raise TypeError(f"midi cc or note number must be an integer, got {type(num).__name__}")
    if num<0 or num > 127:
        raise TypeError(f"num must be between 0 and 127, got ", num)
    if not isinstance(name, str):
        raise TypeError(f"name must be a string, got {type(name).__name__}")
    if not isinstance(param, str):
        raise TypeError(f"param must be a string, got {type(param).__name__}")
    if not isinstance(instance, int):
        raise TypeError(f"instance must be an integer, got {type(instance).__name__}")
    if not isinstance(min, int) or not isinstance(max, int):
        raise TypeError(f"min and max must be integers between 0 and 127, got ", min, max)
    if min < 0 or min > 127 or max < 0 or max > 127:
        raise TypeError(f"min and max must be integers between 0 and 127, got ", min, max)
    
    # Format checks
    if name != name.lower():
        raise ValueError(f"name must be lowercase, got '{name}'")
    if param != param.upper():
        raise ValueError(f"param must be all uppercase, got '{param}'")
    
    # Now use the validated parameters
    send_sysex(["map", midi_type, num, name, instance, param, min, max])