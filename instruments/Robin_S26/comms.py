from scamp import *
from pythonosc.udp_client import SimpleUDPClient
import setup
    
port = 5005
osc = SimpleUDPClient("127.0.0.1", port)
print("starting session on port", port)

esp32_midi_port = setup.esp32_midi_port


def handle_midi(message):
    # We can change this function to process our midi messages
    msg_type = (message[0] >> 4 )
#     print(msg_type, message[1], message[2])
    
    ## note on messages
    if msg_type == 9:
        handle_note(message[1], message[2])

        if message[2] > 0:
            setup.active_notes[message[1]] = message[2]
        else:
            # MIDI standard: NoteOn with 0 velocity is actually a NoteOff
            setup.active_notes.pop(message[1], None)

            
    ## note off messages
    elif msg_type == 8:
        handle_note(message[1], message[2])
        setup.active_notes.pop(message[1], None)
    
    ## cc messages
    elif msg_type == 11:
        setup.ccs[message[1]] = message[2] 
        handle_cc(message[1], message[2])    
        
    ## catch unrecognized messages
    else:
        print('other', msg_type, message[1], message[2])



# function definitions
# function definitions
def send_osc(address, *args):
    arg_ordered = args
    if len(args) == 1 and isinstance(args[0], (list, tuple)):
        args = tuple(args[0])
    if len(args) == 3:
        a, b, c = args
        arg_ordered = [b,c,a]  # define order
    else:
        arg_ordered = list(args)
    osc.send_message("/sendto",address)
    osc.send_message("/message", list(arg_ordered))
    
send_osc('/test', 0)
