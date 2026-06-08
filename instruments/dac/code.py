#####
# MIDI to CV adaptor
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random

#local files to import
from mcp4728 import MCP4728
import utils, midi, setup


DEBUG = True

dac = MCP4728()

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

value = 0
step = (4096/5)/12
pitch = 0
scale = [0,2,4,5,7,9,11,12]

while True:
    current_time = time.monotonic()
    
    dac.buffer_set(0, value)
    dac.buffer_set(1, 1 if value>2000 else 0)
    dac.buffer_set(2, value)
    dac.buffer_set(3, math.floor(scale[math.floor(pitch)]*step))
#     dac.set(0, value)    
    value = (value+500)%4095
    pitch = (pitch+.2)%8
    print(value)
    dac.write_all()
    time.sleep(0.1)
    
    
                    