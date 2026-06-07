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

while True:
    
    current_time = time.monotonic()
    if (current_time - prev_time) > 0.2:
        prev_time = current_time
        note = sequence[index%16]+48
        midi.send_note(prev_note, 0)
        print(prev_note)
        time.sleep(.01)
        midi.send_note(note, 127)
        print(note)
        prev_note = note
        index += 1
        
    
    

#     current_time = time.monotonic()
#     send_midi = True
#     if current_time > send_delay: send_midi = True
#         
#     # send CC data and check for incoming midi
#     if (current_time - serial_timer) >= DATA_RATE/100:
#         serial_timer = current_time
#         
#         msg = []
#         for i in range(len(pots)):
#             value = pots[i].read()
#             if value:
# #                 print(i, value)
#                 midi.send_cc(i, value)
#             
#             value = buttons[i].read()
#             if value:
# #                 print(i, value)
#                 midi.send_note(i, 127 if value is "pressed" else 0)
#             
#         leds[cur_led].duty_cycle = led_value*255 #led_value/255.0
#         led_value = led_value+8
#         if led_value > 255:
#             led_value = 0
#             cur_led = (cur_led+1) %3
# #         print(led_value*255)
# 
#     
#         if midi.check_input():
#                 pixel.fill((100, 0, 0))
#                 send_delay =  time.monotonic() + 3
#         elif send_midi: pixel.fill((0, 100, 0))
#         else:  pixel.fill((0, 0, 100))
        

