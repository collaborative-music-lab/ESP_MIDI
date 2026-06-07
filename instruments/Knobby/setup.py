from scamp import *
import math, random
from euclideanSequencer import EuclideanSequencer

print_available_midi_input_devices()
esp32_midi_port = 1 #change to the current port for your system

tempo = 80 + random.random()*40
s = Session(tempo)


# dictionary for midi cc messages
ccs = {}

# dictionary for MIDI note messages that are currently ON
# receining a note off msg removes it from this list
active_notes = {}

# our sequencers
euclid = {
    "kick":  EuclideanSequencer(beats=8, hits=4),
    "snare": EuclideanSequencer(beats=16, hits=4),
    "hihat": EuclideanSequencer(beats=8, hits=4),
    "synth":  EuclideanSequencer(beats=32, hits=4)
}
euclid["kick"].make_sequence()

# generate a sequence for the synth
synth_sequence = []
minor_scale = [0,2,3,5,7,8,11,12]
for i in range(euclid["synth"].beats):
    synth_sequence.append( minor_scale[ math.floor( math.sin(i *3.2)*7 ) ] )
print( synth_sequence )
