#####
# Mbira
####

import board, analogio,  neopixel, pwmio, busio
import time, math, random

#local files to import
from imu_driver import IMU
from imu_processing import *
import utils, setup, sensors, tine
from led_driver import LedDriver
from utils import *
from midi_handler import MidiHandler

DEBUG = False
DEBUG_ONE = False
DEBUG_PIN = 4

SDA=board.IO44
SCL=board.IO43
i2c = busio.I2C(SCL,SDA) # Note: SCL is usually first in busio.I2C

imu = IMU(i2c, board_type="LSM6")
led = LedDriver( i2c )
midi = MidiHandler( channel=0 )

tine_pins = [6,10,9,8,7,4,3,2,1]
tine_g = [7,9,3,5, 2,18,12,14, 16]
tine_r = [8,10,4,6, 1,11, 13, 15, 17]

tines = [tine.Tine(i) for i in range(9)]
# tines = [tine.Tine(tine_pins[i]) for i in range(9)]

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

scale = [0,2,4,5,7,9,11,12]



value = 0

def note_on(note, vel):
    print("NOTE_ON", note, vel)
    if note == 0:
        for t in tines:
            t.calibrate()
        print("calibrated")
        time.sleep(2)
    elif note == 1:
        for t in tines:
            t.save_calibration()
            print("saved")
        time.sleep(2)
    
#     elif note == 1:
#         button[vel].set_threshold(10000)
midi.on_note_on = note_on


# MAIN LOOP
# separate timers to do sensor reading and midi sending
read_timer = time.monotonic()
serial_timer = time.monotonic()
print_timer = time.monotonic()+1
read_interval = 0.002
serial_interval = 0.02
print_interval = 0.1
while True:
    now = time.monotonic()
    imu.update()
    status = led.update()
    
    # read sensors
    if (now - read_timer) > read_interval:
        read_timer = now
        
        msg = []
        for tine in tines:
            tine.read()
        for i, tine in enumerate(tines):
            tine.process()
            if i==DEBUG_PIN and DEBUG_ONE: print(max(-1, min(1,tine.value)) )
                    
    # send CC data and check for incoming midi
    if (now - serial_timer) > serial_interval:
        serial_timer = now
        midi.update()
        for t in tines:
            value = t.get_delta()
            note = t.instance
#             if value > 0: note += 12
            if abs(value) > 0.1:
                value = min(127, max(0, abs(value)*127))
                midi.send_cc(note, value)
            else: midi.send_cc(note, 0)
        
    # debug
    if (now - print_timer) > print_interval:
        print_timer = now
        
        msg = []
        for i, tine in enumerate(tines):
            msg.append( max(-1, min(1,tine.get_value())) )
#             msg.append( min(10, tine.get_delta()) )
            
#         print(imu.accel_cur)
#             if i==0: print(tine.value, tine.range)
            if tine.led:
#                 print(tine.red)
                led.set(tine.red_pin-1,  tine.red)
                led.set(tine.green_pin-1,  tine.green)

        if DEBUG: print(msg)
        
                    