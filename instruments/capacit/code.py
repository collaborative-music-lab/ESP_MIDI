#####
# Capacit firmware v0.5
####

import board
import time
import analogio
import digitalio
import neopixel

#local files to import
from mpr121 import MPR121
import utils, midi, setup


# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

# setup mpr121
number_sensors = 4
cap = MPR121(num_sensors = number_sensors)
cap.set_sensitivity(current=setup.charge_current, charge_time=setup.charge_time) #current from 1-63, charge_time from 1 to 7
monitor = ""

# two separate timers to do sensor reading and midi sending
capsense_timer = time.monotonic()
serial_timer = time.monotonic()
current_time = time.monotonic()

# onepole filters and scaling for proximity and touch sensing
prox_lpf = [utils.OnePole(0.1) for i in range(number_sensors)]
touch_lpf = [utils.OnePole(0.3) for i in range(number_sensors)]

def scale_prox(num):
    val = cap.read(num)
    val = utils.scale( val, 0, 60, 0, 127 )
    return prox_lpf[num].update(val)

def scale_touch(num):
    val = cap.read(num)
    val = utils.scale( val, 300, 700, 0, 127 )
    return touch_lpf[num].update(val)


# main loop
# the send_delay prevents random data being sent when the mpr121 is reset
send_delay =  time.monotonic() + 3
while True:
    current_time = time.monotonic()
    send_midi = False
    if current_time > send_delay: send_midi = True
    
    # check capsense data for button presses
    if (current_time - capsense_timer) >= .025:
        capsense_timer = current_time
        cap.update()
        proximity_value = [ scale_prox(i) for i in range(number_sensors)]
        touch_value = [scale_touch(i) for i in range(number_sensors)]
        if monitor == "proximity": print(proximity_value, sep=", ")
        elif monitor == "touch": print(touch_value, sep=", ")
        for i in range(number_sensors):
            if cap.is_touched(i) == 'PRESSED':
                midi.send_note(i, 127) #note on
                midi.send_cc(i,  cap.read(i) )
                # set proximity to 127
                if send_midi:
                    prox_lpf[i].output = 127
                    midi.send_cc(i+12,  127 )
            elif cap.is_touched(i) == 'RELEASED':
                midi.send_note(i, 0) #note off
                midi.send_cc(i,  0 )
        
            
    # send CC data and check for incoming midi
    if (current_time - serial_timer) >= .100:
        serial_timer = current_time
        if send_midi:
            for i in range(number_sensors):
                if cap.is_touched(i) == 'HIGH':
                    midi.send_cc(i,  touch_value[i] )
                elif cap.is_touched(i) == 'LOW':
                    midi.send_cc(i+12, proximity_value[i] )
        if midi.check_input():
                cap.reset()
                pixel.fill((100, 0, 0))
                send_delay =  time.monotonic() + 3
        elif send_midi: pixel.fill((0, 100, 0))
        else:  pixel.fill((0, 0, 100))
        