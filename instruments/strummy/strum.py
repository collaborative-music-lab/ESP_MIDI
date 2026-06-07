#####
# IMU Lab code
####

import board, analogio, analogio, neopixel, pwmio
import time, math

#local files to import
from imu_driver import IMU
from imu_processing import *
import utils, midi, setup, sensors
from clock import Clock
from midi_clock import MidiClock

clock = Clock()

DEBUG = True
DATA_RATE = 200 # ms interval between sending data

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

# separate timers to do sensor reading and midi sending
read_timer = time.monotonic()

serial_timer = time.monotonic()
current_time = time.monotonic()

pots = [sensors.Pot(i) for i in [board.IO9,board.IO8,board.IO2,board.IO7]]
buttons = [sensors.Button(i) for i in [board.IO6,board.IO3,board.IO4,board.IO5]]

leds = [
    pwmio.PWMOut(board.IO11, frequency=1000, duty_cycle=65535),
    pwmio.PWMOut(board.IO13, frequency=1000, duty_cycle=65535),
    pwmio.PWMOut(board.IO12, frequency=1000, duty_cycle=65535)
]

# main loop
# the send_delay prevents random data being sent when the mpr121 is reset
send_delay =  time.monotonic() + 3
led_value = 0
cur_led = 0

sequence = [0,2,4,7, 2,4,7,9, 4,7,9,11, 12,7,5,4]
index = 0
prev_time = 0

class test_time:
    def __init__(self, clock, midi, sequence = [60], interval = 1000):
        self.clock = clock
        self.midi = midi
        self.sequence = sequence
        self.prev_time = 0
        self.index = 0
        self.interval = interval
        self.clock.schedule_ms(self.interval, self.update)
        self.cur_note = 60
    
    def update(self):
        self.clock.schedule_ms(self.interval, self.update)
        cur_time = time.monotonic()
#         print( (cur_time - (self.prev_time + self.interval/1000)) * 1000)
#         self.prev_time = cur_time
        self.index = (self.index + 1)
        if self.index%12 == 0:
            index = math.floor(self.index/12) % len(self.sequence)
            self.cur_note = self.sequence[index] + 48
            self.midi.send_note(self.cur_note, 127)
            self.clock.schedule_ms(self.interval/2, self.note_off)
            print( (cur_time - (self.prev_time)))
            self.prev_time = cur_time
        
    def note_off(self):
        self.midi.send_note(self.cur_note, 0)
        
    
testing = test_time(clock, midi, sequence = sequence, interval = 500/24)
    
prev_note = 0

chord = [0,4,7]
chords = [
    [chord[i%3]+math.floor(i/3)*12 for i in range(12)],
    [chord[i%3]+math.floor(i/3)*12+5 for i in range(12)],
    [chord[i%3]+math.floor(i/3)*12+7 for i in range(12)],
    [chord[i%3]+math.floor(i/3)*12+-2 for i in range(12)]
    ]

note_distance = 1
thresholds = [
    [60 + note_distance*i for i in range(12)],
    [60 - note_distance + note_distance*i for i in range(12)]
    ]

cur_val = pots[0].read()
cur_string = 0
cur_chord = 0

print(thresholds)

button_down = False

while True:
    current_time = time.monotonic()

    # send CC data and check for incoming midi
    if (current_time - serial_timer) > 0.001:
        serial_timer = current_time
        button_down = False

            
        for i in range(len(pots)):
            value = buttons[i].read()
            if value:
#                 print(i, value)
#                 midi.send_note(i, 127 if value is "pressed" else 0)
                cur_chord = i
                
            for i in range(len(pots)):
                if buttons[i].prev_state == False:
                    button_down = True
                
            value = pots[i].read()

            if value:
#                 print(i, value)
#                 midi.send_cc(i, value)
                value = 127-value
                if i == 3:
                    if value > thresholds[0][cur_string]:
                        print('up', cur_string, value, thresholds[1][cur_string], thresholds[0][cur_string])
                        cur_string += 1
                        if cur_string > 11: cur_string = 11
                        if button_down:
                            midi.send_note(chords[cur_chord][cur_string]+36, 127)
                            time.sleep(0.01)
                            midi.send_note(chords[cur_chord][cur_string]+36, 0)

                    elif value < thresholds[1][cur_string]:
                        print('down', cur_string, value, thresholds[1][cur_string], thresholds[0][cur_string])
#                         midi.send_note(chords[cur_chord][cur_string]+48, 0)
                        cur_string -=1
                        if cur_string < 0: cur_string = 0
                        if button_down:
                            midi.send_note(chords[cur_chord][cur_string]+36, 127)
                            time.sleep(0.01)
                            midi.send_note(chords[cur_chord][cur_string]+36, 0)
#                         print('down', cur_string, value)
            
            
                
                


