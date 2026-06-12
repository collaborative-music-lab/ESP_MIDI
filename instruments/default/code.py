#####
# Castle exploration
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random

#local files to import
from imu_driver import IMU
from imu_processing import *
import utils, midi, setup, sensors
from castle import Castle

DEBUG = True

imu = IMU(board_type="LSM6", SDA=board.IO44, SCL=board.IO43)
print(imu)

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

voice = [
    Castle(1),
    Castle(2),
    Castle(4),
    Castle(7)
    ]
divisions = [1,2,3,4, 5,6,8,16]



# separate timers to do sensor reading and midi sending
read_timer = time.monotonic()
serial_timer = time.monotonic()
current_time = time.monotonic()
prev_time = 0
clock_counter = 0


scale = [0,2,4,5,7,9,11]
root = scale[0]                 
velocity = 100


def dtop(val, scale):
    # converts a scale degree to a midi note
    length  = len(scale)
#     print(val, scale[val%length] + math.floor(val/length)*12 , scale)
    return scale[val%length] + math.floor(val/length)*12 


    

index = 0
button_down = False
subindex = 0
is_down = [0,0,0,0]
button_state = [False,False,False,False]

def send_root(note,velocity):
    pass
    

while True:
    current_time = time.monotonic()
    imu.update()
    
    if (current_time - clock_counter) > 0.14:
        clock_counter = current_time
        
        index += 1
        subindex = 0
        
        note = 0
        for i in range(4):
            note += voice[i].get(index)
        
        if button_down:
                
            note = dtop(note, scale) + 60
                
            midi.send_note(note, 127-note)
            time.sleep(0.01)
            midi.send_note(note, 0)
            
            msg = []
            for i in range(4): msg.append(voice[i].subdivide)
#             print(index, note, msg)
        tilt = get_tilt_angles(imu.accel_cur)
        volume = (tilt[0]/180 + 0.5)*127
        damping = (tilt[1]/180 + 0.5)*127
        midi.send_cc(0, volume)
        midi.send_cc(1, damping)
#         print(volume, damping)
        print(tilt)
        #tilt[0] = volume
        #tilt[1] = damping
            
    # send CC data and check for incoming midi
    if (current_time - prev_time) > 0.01:
        prev_time = current_time
        button_down = False
        
        for i in range(len(pots)):
            # buttons
            value = buttons[i].read()
            
            if value:
                voice[i].press(value, index)
                button_state[i] = True if value == "pressed" else False

            if button_state[i]: button_down = True
            
            #pots
            value= False
            value = pots[i].read()
                
            if value:
                value = value - 255
                if value >=10:
#                 print(i, value)
#                     print(i, value, math.floor(abs(value-10)/32))
                    voice[i].subdivide = divisions[math.floor((value-10)/32)]
                    voice[i].note = abs(voice[i].note)
                elif value < -10:
#                     print(i, value, math.floor(abs(value+10)/32))
                    voice[i].subdivide = divisions[math.floor(abs(value+10)/32)]
                    voice[i].note = abs(voice[i].note) * -1
                

                    