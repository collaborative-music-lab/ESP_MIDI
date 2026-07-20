#####
# Robin
####

import board, analogio, analogio, neopixel, pwmio, storage, microcontroller
import time, math, random, sensors, midi_handler
from imu_driver import IMU
from imu_processing import *
from midi_handler import MidiHandler

DEBUG_TOUCH = False
DEBUG_CAP = False

pot = [sensors.Pot(11)]
midi = midi_handler.MidiHandler( channel=0 )

num_touch = 11
touchPins = [8, 9, 10, 5, 4, 3, 2, 1, 13, 6, 7]
button = [sensors.TouchButton(touchPins[i]) for i in range(num_touch)]

thresholds= [ 0.5 for i in range(11)]
thresholds[8] = 0.9
# thresholds[9] = 0.4

def update_thresholds():
    for i in range(num_touch):
        button[i].set_threshold(thresholds[i])
    print('updated thresholds')
    button[i].calibration_mode = True

    
update_thresholds()

def note_on(note, vel):
    print("NOTE_ON", note, vel)
midi.on_note_on = note_on

def on_cc(num,val):
    print("CC", num, val)
    if num == 127:
        if val == 0 and microcontroller.nvm[0] == 1:
            microcontroller.nvm[0] = 0
            microcontroller.reset()
        elif microcontroller.nvm[0] == 0:
            microcontroller.nvm[0] = 1
            microcontroller.reset()
midi.on_control_change = on_cc

imu = IMU(board_type="LSM6", SDA=board.IO44, SCL=board.IO43)
# imu = IMU(board_type="BMI160", SDA=board.IO44, SCL=board.IO43)

cap_timer = 0
print_timer = 0

scale = [0,1,2,3,4,5,6,7,8,9,10]
left_note = 0
right_note = 0
level = 0

touch_interval = 0.2 if DEBUG_TOUCH else 0.001
control_interval = 0.2 if DEBUG_CAP else 0.02
imu.update()
accel = [imu.accel_cur[i] for i in range(3)]
magnitude = get_magnitude(accel)
tilt = [0,0,0]

level_smoothing = 0.9

while True:
    
    if time.monotonic()-cap_timer >= touch_interval:
        cap_timer= time.monotonic()
        midi.update()
        
        for i in range(num_touch):
            button[i].update()
            
        msg = []
        for i in range(num_touch):
            value = button[i].get_state()
            if value == "pressed":
                midi.send_note(scale[i], 127)
                msg.append(1)
            elif value == "down": msg.append(1)
            elif value == "released":
                msg.append(0)
                midi.send_note(scale[i], 0)
            elif value == "up": msg.append(0)
            else: msg.append(0)
        if DEBUG_TOUCH: print(msg)

    if time.monotonic()-print_timer >= control_interval:
        print_timer = time.monotonic()

        # IMU
        new_data = imu.update()
#         print(new_data)
        for i in range(3):
            accel[i] = onepole(imu.accel_cur[i],accel[i],level_smoothing)
        magnitude = smooth(get_magnitude(accel), magnitude, level_smoothing/2, level_smoothing/2+.5)
#         print(accel, magnitude*10)
        tilt = get_tilt_angles(imu.accel_cur)
        midi.force_send_cc(3, tilt[0]/2 + 45)
        midi.force_send_cc(4, tilt[1]/2 + 45) 
        midi.force_send_cc(5, tilt[2]/2 + 45)
        midi.force_send_cc(6, magnitude*127)

        # POTENTIOMETER
        value = pot[0].read()
        if value:
            level_smoothing = 1 - math.pow(value/128,2)
#             print(level_smoothing)
            midi.send_cc(0, value)
        

        if DEBUG_CAP: 
            msg = []
            for i in range (11):
                msg.append(button[i].raw())
            print(msg)

        
        

