#####
# Strummy exploration
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random

#local files to import
from imu_driver import IMU
from imu_processing import *
import utils, midi, setup, sensors
from clock import Clock
from midi_clock import MidiClock

clock = Clock()

imu = IMU(board_type="LSM6", SDA=board.IO44, SCL=board.IO43)
print(imu)

DEBUG = True
NUM_NOTES = 12

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

# user interface
pots = [sensors.Pot(i) for i in [board.IO9,board.IO8,board.IO2,board.IO7]]
buttons = [sensors.Button(i) for i in [board.IO6,board.IO3,board.IO4,board.IO5]]
leds = [
    pwmio.PWMOut(board.IO11, frequency=1000, duty_cycle=65535),
    pwmio.PWMOut(board.IO13, frequency=1000, duty_cycle=65535),
    pwmio.PWMOut(board.IO12, frequency=1000, duty_cycle=65535)
]

sequence = [0,2,4,7, 2,4,7,9, 4,7,9,11, 12,7,5,4]
index = 0  
prev_note = 0

note_distance = 20
base_range = 220
thresholds = [
    [base_range + note_distance*i for i in range(NUM_NOTES+2)],
    [base_range - note_distance + note_distance*i for i in range(NUM_NOTES+2)]
    ]
cur_val = pots[0].read()
cur_string = 0
cur_chord = 0

print(thresholds)

button_down = False

# separate timers to do sensor reading and midi sending
read_timer = time.monotonic()
serial_timer = time.monotonic()
current_time = time.monotonic()
prev_time = 0
clock_counter = 0


scale = [0,2,4,5,7,9,11]
root = scale[0]                 
pitches = [scale[i%3*2] for i in range(NUM_NOTES)]       
strings = [0]
for i in range(1,12):
    strings.append(strings[i-1]+math.floor(random.random()*2+2))
strings = [0,3,6,8,10,12,13,15,16,18,20,21]
chord = [0,2,4]
seventh = False
ninth = False
velocity = 100
print(pitches)


def dtop(val, scale):
    # converts a scale degree to a midi note
    length  = len(scale)
#     print(val, scale[val%length] + math.floor(val/length)*12 , scale)
    return scale[val%length] + math.floor(val/length)*12 
    
def tune_string(num,chord):
    note = chord[0]
    index = 0
    length = len(chord)
#     print(num,chord)
    while note < strings[num]:
        note = chord[index%length] + math.floor(index/length)*7
#         print('wh', index, note)
        index = index + 1
#     print(note)
    return note
    
def make_chord(num, state):
#     global seventh, ninth, chord, scale,pitches
#     if state == "pressed" and num == 1:
#         chord = [0,2,4]
#     elif state == "pressed" and num == 1:
#         chord = [0,2,4]  
#     chord = [0,2,4]
#     if seventh: chord.append(6)
#     if ninth: chord.append(8)
# #     print('make', chord, seventh, ninth)
    
    for i in range( len(pitches)):
        pitches[i] = tune_string(i, chord)
        
#     print('pitches2', pitches)
    

index = 0
notes_to_play = []
button_down = False
subindex = 0
new_chord = False
is_down = [0,0,0,0]

def send_root(note,velocity):
    pitch = dtop(note, scale) + 36
    midi.send_note(pitch, velocity, 1)
    time.sleep(0.01)
    midi.send_note(pitch, 0, 1)
    

while True:
    current_time = time.monotonic()
    imu.update()
    
    if (current_time - clock_counter) > 0.14:
        clock_counter = current_time
        
        index += 1
        subindex = 0
        
        if index%4 == 0:
            if buttons[0].prev_state == 0: # play root
                send_root(root, 127)
        
        
        if button_down:
            button_down = False
            for i in range(4):
                is_down[i] = 1 if buttons[i].prev_state==0 else 0
            root = (
                is_down[1]*1 +
                is_down[2]*2 +
                is_down[3]*4
                )
        
            # play notes that have been strummed
            play_note = len(notes_to_play) > 0
            if play_note:
                note = notes_to_play.pop(0)
                new_chord = True
#             print(pitch, notes_to_play)        
                midi.send_note(note, velocity - math.floor(note/1))
                print( note, velocity - math.floor(note/1) )
            
                time.sleep(0.01) # time delay for note off
                midi.send_note(note, 0)
                
            
    elif (current_time - clock_counter) > 0.02 * subindex: # strum more than one note
        if len(notes_to_play) > 0 and new_chord:
            note = notes_to_play.pop(0)
            midi.send_note(note, velocity - math.floor(note/1))
            time.sleep(0.01)
            midi.send_note(note, 0)
        if len(notes_to_play) == 0: new_chord = False
        subindex += 1
            
        
    # send CC data and check for incoming midi
    if (current_time - prev_time) > 0.01:
        prev_time = current_time
        
#         print(imu.accel_cur)

        for i in range(len(pots)):
            if buttons[i].prev_state == False and i == 0:
             button_down = True
        is_down = [0,0,0,0]
        for i in range(len(pots)):
            # buttons
            value = buttons[i].read()
            if value: make_chord(i, value)
            
            #pots
            value= False
            value = pots[i].read()
#                 velocity = pots[i].vel + 20
            if value:
#                 print(i, value)
                
                # look for a strum up and down
                if i == 3:
                    value = 512-value
                    if value > thresholds[0][cur_string]:
                        cur_string += 1
                        if cur_string >= NUM_NOTES+2: cur_string = NUM_NOTES+1
                        if button_down and cur_string > 0 and cur_string < 13:
                            pitch = dtop(pitches[cur_string-1] + root, scale) + 36
                            notes_to_play.append( pitch)
                    elif value < thresholds[1][cur_string]:
                        cur_string -=1
                        if cur_string < 0: cur_string = 0
                        if button_down and cur_string > 0 and cur_string < 13:
                            pitch = dtop(pitches[cur_string-1] + root, scale) + 36
                            notes_to_play.append( pitch)
                    
                # chord chord voicing
                elif i == 0:
                    voicing = math.floor(value/512*4)
                    if voicing == 0: chord = [0,2,4]
                    elif voicing == 1: chord = [0,2,4,6]
                    elif voicing == 2: chord = [0,2,4,8]
                    elif voicing == 3: chord = [0,2,4,6,8,12]
                    
                    for i in range( len(pitches)):
                        pitches[i] = tune_string(i, chord)
                    print(pitches)
                    
                    

