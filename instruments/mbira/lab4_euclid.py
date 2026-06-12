from scamp import *
import comms
from pynput import keyboard
import threading, math, random
from setup import *

debug = True # set to false to minimize clock lagging
index = 0

def handle_note(note, velocity):
    if debug: print('note', index % 8, note, velocity)
    # comms.send_osc('newNote', note)
#     if velocity > 0 : comms.send_osc('trigger', note)
    for i, name in enumerate(euclid):
        if note == i:
            euclid[name].rotate_sequence(index)
    
def handle_cc(num, val):
    if debug: print('cc', num, val)

    if num < 4:
        euclid_names = list(euclid)
        this_seq = euclid[euclid_names[num]]
        this_seq.make_sequence( hits = math.floor(val*(this_seq.beats/127)) )
        if num<3:
            ratio = this_seq.hits / this_seq.beats
            comms.send_osc('decay', num+2, 'D', (1-ratio) * 28 + 0)
        else:
            pass

def mainLoop():
    global index
    
    while True:
        print("index: ", index)
        
        for i, name in enumerate(euclid):
            # i is the index (0, 1, 2, 3)
            # name is the string ("kick", "snare", etc.)
            
            if i in active_notes:
#                 print(i, euclid[name].sequence)
                if( euclid[name][index]):
                    print(name, ': ', euclid[name].sequence)
                    # handle synth separately
                    if name == 'synth':
                        print(index % euclid[name].beats, len(euclid[name].sequence))
                        comms.send_osc('note', synth_sequence[ index % euclid[name].beats ]/127)
                        comms.send_osc('trigger', 3)
                        # comms.send_osc('bob-filter', 1, 'CUTOFF', 15+random.random()*10 )
                    # handle drum vocies
                    else:
                        comms.send_osc('trigger', i)

        index = (index+1)
        wait(0.25)
    
def run_session():
    s.fork(mainLoop)
    s.wait_forever()

threading.Thread(target=run_session).start()

s.register_midi_listener(comms.esp32_midi_port, comms.handle_midi)
comms.handle_cc = handle_cc
comms.handle_note = handle_note

# initialize some synth parameters:
comms.send_osc( 'bwl-osc', 2, 'FM', 10)




