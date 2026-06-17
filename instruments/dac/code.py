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
pixel.brightness = 1
pixel[0] = (255, 100, 0)

led = [led_pwm.Led(12),led_pwm.Led(11)]

clock_timer = 0
bpm = 330
bpm_seconds = 60/bpm
index = 0
prev_time = 0

def volt_hertz(pitch, hz_per_volt=1000):
    frequency = 440 * (2 ** ((pitch - 69) / 12))
    volts = frequency / hz_per_volt
    return volts

while True:
    now = time.monotonic()
    handler.update()
    
    current_time = time.monotonic()
    
    if now - prev_time > 0.1:
        prev_time = now
        
    
        available = manager.update()
        manager.broadcast_presence()
#         print( available )
        if available is not False: print(available)
        
        while manager.available > 0:
            incoming = manager.read()
            if incoming is None: break
            print(incoming)
            if incoming['type'] == 'clock':
                handler.scheule_ms(100,  lambda: dac.set(0,4095))
                handler.schedule_ms(200,  lambda: dac.set(0,0))
                handler.schedule_ms(100, lambda: led[0].set(0))
                handler.schedule_ms(200, lambda: led[0].set(1))
            elif incoming['type'] == 'note':
                note = int(incoming['name'])
                vel = int(incoming['data'][0])
                delay = int(incoming['data'][1])
                if delay < 1: led[0].set(0)
                else: handler.schedule_ms(delay, lambda: led[0].set(1))
                
        
    if now  - clock_timer > bpm_seconds:
        clock_timer = now
                 
        dac.buffer_set(0,4095)
        handler.schedule_ms(math.floor(14), lambda: dac.set(0,0))
        
        dac.buffer_set(1,4095)
        handler.schedule_ms(10, lambda: dac.set(1,0))
        
        pitch = scale[index%8]+72
        volt = volt_hertz(pitch)
        
        pitch2 = scale[(index+2)%8]+72
        volt2 = volt_hertz(pitch2)
        volt_scalar = 4095/5
        
        
        dac.buffer_set(2,math.floor(volt*volt_scalar))
        
        dac.buffer_set(3,math.floor(volt2*volt_scalar))
        
        dac.write_all()
        
#         print(volt, volt2, math.floor(volt*volt_scalar), math.floor(volt2*volt_scalar))
   
        
        handler.schedule_ms(50, lambda: led[0].set(0))
        index += 1
#         print('clock', index)

        led[0].set(1)
        
          