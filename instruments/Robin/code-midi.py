#####
# Robin
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random, sensors, midi_handler
from imu_driver import IMU
from imu_processing import *
from midi_handler import MidiHandler

DEBUG_TOUCH = True
DEBUG_CAP = False
DEBUG_ONE = False

pot = [sensors.Pot(11)]
midi = midi_handler.MidiHandler( channel=0 )
# imu = IMU(board_type="LSM6", SDA=board.IO44, SCL=board.IO43)
imu = IMU(board_type="BMI160", SDA=board.IO44, SCL=board.IO43)

num_touch = 11
touchPins = [8, 9, 10, 5, 4, 3, 2, 1, 13, 6, 7]
button = [sensors.TouchButton(touchPins[i]) for i in range(num_touch)]
# thresholds= [
#     1000,
#     1000, 300, 800,
#     500, 160, 280, 260,
#     500, 500, 400]
# button[9].set_threshold(thresholds[9])
# button[10].set_threshold(thresholds[10])

thresholds= [ 0.08 for i in range(11)]
# thresholds[0] = 700
# thresholds[8] = 1000
# thresholds[9] = 1000
# thresholds[10] = 1000
# thresholds[10] = 25000


def update_thresholds():
    for i in range(num_touch):
        button[i].set_threshold(thresholds[i])
    print('updated thresholds')
    button[i].calibration_mode = True

    
update_thresholds()

def note_on(note, vel):
    print("NOTE_ON", note, vel)
    if note == 0:
        for b in button:
            b.calibrate()
    elif note == 1:
        button[vel].set_threshold(10000)
midi.on_note_on = note_on

# left = [4,3,2,1]
# right = [9,10,5]
# thumb = 8
# arrors = [13,6]
# strip = 7

cap_timer = 0
print_timer = 0

scale = [0,2,4,5,7,9,11,12,14,16,17,19,21,23,24]
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
            if i == 0:
#                 print(thresholds[0], value, button[i].value, button[i].calibration_mode)
                button[i].calibration_mode = True
            if value == "pressed":
                midi.send_note(scale[i], 127)
#                 if i == 0: update_thresholds()
                msg.append(1)
            elif value == "down": msg.append(1)
            elif value == "released":
                midi.send_note(scale[i], 0)
                msg.append(0)
            elif value == "up": msg.append(0)
            else: msg.append(0)
        if DEBUG_TOUCH: print(msg)
        if DEBUG_ONE:
            print(DEBUG_ONE, button[DEBUG_ONE].get_state(), button[DEBUG_ONE].value, button[DEBUG_ONE].peak, button[DEBUG_ONE].baseline)
            
            
    if time.monotonic()-print_timer >= control_interval:
        print_timer = time.monotonic()

        # IMU
        imu.update()
        for i in range(3):
            accel[i] = onepole(imu.accel_cur[i],accel[i],level_smoothing)
        magnitude = smooth(get_magnitude(accel), magnitude, level_smoothing/2, level_smoothing/2+.5)
#         print(accel, magnitude*10)
        tilt = get_tilt_angles(imu.accel_cur)
        midi.force_send_cc(3, tilt[0]/2 + 45)
        midi.force_send_cc(4, tilt[1]/2 + 45) 
        midi.force_send_cc(5, tilt[2]/2 + 45)



        # POTENTIOMETER
        value = pot[0].read()
        if value:
            level_smoothing = 1 - math.pow(value/128,2)
            midi.send_cc(5, level_smoothing)
#             print(level_smoothing)
        
        if DEBUG_CAP: 
            msg = []
            for i in range (11):
                msg.append(button[i].value)
            print(msg)

        
        
