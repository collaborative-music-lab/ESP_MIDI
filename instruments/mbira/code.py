#####
# Mbira
####

import board, analogio,  neopixel, pwmio, busio
import time, math, random

#local files to import
from imu_driver import IMU
from imu_processing import *
import utils, midi, setup, sensors, tine
from led_driver import LedDriver
from utils import *

DEBUG = True
DEBUG_ONE = False
DEBUG_PIN = 8

SDA=board.IO44
SCL=board.IO43
i2c = busio.I2C(SCL,SDA) # Note: SCL is usually first in busio.I2C

imu = IMU(i2c, board_type="LSM6")
print(imu)

led = LedDriver( i2c )
print(led)

tine_pins = [6,10,9,8,7,4,3,2,1]
tine_g = [7,9,3,5, 2,18,12,14, 16]
tine_r = [8,10,4,6, 1,11, 13, 15, 17]

tines = [tine.Tine(tine_pins[i],tine_r[i],tine_g[i]) for i in range(9)]
# tines = [tine.Tine(tine_pins[i]) for i in range(9)]

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1



# separate timers to do sensor reading and midi sending
read_timer = time.monotonic()
serial_timer = time.monotonic()
current_time = time.monotonic()
prev_time = 0
clock_counter = 0
print_timer = 0

value = 0


while True:
    current_time = time.monotonic()
    imu.update()
    status = led.update()
#     if status == True: print('sent')
    
    if (current_time - clock_counter) > 0.14:
        clock_counter = current_time
#         for i in range(18):
#             led.set(i, value)
#         value = (value+10)%255
            
    # send CC data and check for incoming midi
    if (current_time - prev_time) > 0.01:
        prev_time = current_time
        
        msg = []
        for i, tine in enumerate(tines):
            tine.update()
            if i==DEBUG_PIN and DEBUG_ONE: print(max(-1, min(1,tine.value)) )
        
    # send CC data and check for incoming midi
    if (current_time - print_timer) > 0.02:
        print_timer = current_time
        
        msg = []
        for i, tine in enumerate(tines):
            msg.append( max(-1, min(1,tine.value)) )
            
#         print(imu.accel_cur)
#             if i==0: print(tine.value, tine.range)
            if tine.led:
#                 print(tine.red)
                led.set(tine.red_pin-1,  tine.red)
                led.set(tine.green_pin-1,  tine.green)

        if DEBUG: print(msg)
        
                    