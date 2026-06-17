#####
# MIDI to CV adaptor
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random

#local files to import
from mcp4728 import MCP4728
import utils, midi, setup, esp_now, event_handler, led_pwm

DEBUG = True

## COMMS SETUP

handler = event_handler.EventHandler()

# Initialize manager
manager = esp_now.ESPNowPeerManager(
    device_name="robin",
    device_type="clock",
    broadcast_interval=1.0
)

# DAC
dac = MCP4728()

value = 0
step = (4096/5)/12
pitch = 0
scale = [0,2,4,5,7,9,11,12]

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

led = [led_pwm.Led(12),led_pwm.Led(13)]

clock_timer = 0
bpm = 100
bpm_seconds = 60/bpm
index = 0


while True:
    now = time.monotonic()
    
    current_time = time.monotonic()
    handler.update()
    
    if now  - clock_timer > bpm_seconds:
        clock_timer = now
        dac.set(0,4095)
        handler.schedule_ms(10, lambda: dac.set(0,0))
        index += 1
        print('clock', index)
        
        led[0].set(index%4/4)
        led[1].set(index%16/16)
    
#     dac.buffer_set(0, value)
#     dac.buffer_set(1, 1 if value>2000 else 0)
#     dac.buffer_set(2, value)
#     dac.buffer_set(3, math.floor(scale[math.floor(pitch)]*step))
# #     dac.set(0, value)    
#     value = (value+500)%4095
#     pitch = (pitch+.2)%8
#     print(value)
#     dac.write_all()
#     time.sleep(0.1)
    
    
                    