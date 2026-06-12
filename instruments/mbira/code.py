#####
# Robin
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random, sensors, midi
from imu_driver import IMU
from imu_processing import *

DEBUG_TOUCH = False
DEBUG_CAP = False

pot = [sensors.Pot(11)]

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

thresholds= [ 5000 for i in range(11)]
# thresholds[2] = 1000
# thresholds[8] = 25000
# thresholds[9] = 25000
# thresholds[10] = 25000


def update_thresholds():
    for i in range(num_touch):
        button[i].set_threshold(thresholds[i])
    print('updated thresholds')
    
update_thresholds()

# left = [4,3,2,1]
# right = [9,10,5]
# thumb = 8
# arrors = [13,6]
# strip = 7
    
imu = IMU(board_type="LSM6", SDA=board.IO44, SCL=board.IO43)
# imu = IMU(board_type="BMI160", SDA=board.IO44, SCL=board.IO43)

cap_timer = 0
print_timer = 0

scale = [0,2,4,5,7,9,11,12,14,16,17,19,21,23,24]
left_note = 0
right_note = 0
level = 0

touch_interval = 0.1 if DEBUG_TOUCH else 0.001
imu.update()
accel = [imu.accel_cur[i] for i in range(3)]
magnitude = get_magnitude(accel)
tilt = [0,0,0]

level_smoothing = 0.9

while True:

    if time.monotonic()-cap_timer >= touch_interval:
        cap_timer= time.monotonic()
        for i in range(num_touch):
            button[i].update()
            
        msg = []
        for i in range(num_touch):
            value = button[i].get_state()
            if value == "pressed":
#                 if i == 0: update_thresholds()
                msg.append(1)
            elif value == "down": msg.append(1)
            elif value == "released": msg.append(0)
            elif value == "up": msg.append(0)
            else: msg.append(0)
        if DEBUG_TOUCH: print(msg, button[10].value, button[10].threshold, button[10].button.threshold, button[10].get_state())
            
        rnote = scale[ msg[1] + msg[2]*2 + msg[3]*4 + 4] - 12
        lnote = scale[ msg[4] + msg[5]*2 + msg[6]*4 + msg[7]*4 ]
        
        if rnote!= right_note:
            right_note = rnote
            midi.send_note(right_note+48, 127)
#             print('right', right_note)
        if lnote!= left_note:
            left_note = lnote
            midi.send_note(left_note+48, 127, 1)
#             print('left', left_note)
            
    control_interval = .1 if DEBUG_CAP else 0.02
    if time.monotonic()-print_timer >= 0.02:
        print_timer= time.monotonic()
        
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
#             print(level_smoothing)
        
        #pitch bend
        r_bend = 0
        count = 0
        for i in range (1,4):
            if button[i].state == 'pressed' or button[i].state == 'down':
                r_bend += button[i].value
                count += 1
        
        if count > 0:
            r_bend = r_bend/500/count*20
            midi.force_send_cc(0, r_bend)
#             print('R', count, r_bend)
        
        l_bend = 0
        count = 0
        for i in range (4,7):
            if button[i].state == 'pressed' or button[i].state == 'down':
                l_bend += button[i].value
                count += 1
        if count > 0:
            l_bend = l_bend/500/count*20
            midi.force_send_cc(1, l_bend)
#             print('L', count, l_bend)
        
        # amplitude
        level = level*0.2 + (button[0].value/20) * .8
        level = level/100 + magnitude*1000
        level = math.pow( level/127,0.5) * 127
        midi.force_send_cc(2, level)
#         print(level)
#         print(button[0].value,button[1].value,button[4].value  )
        if DEBUG_CAP: 
            msg = []
            for i in range (11):
                msg.append(button[i].value)
            print(msg)

        
        
