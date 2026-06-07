#####
# The Barre
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random, sensors, mpr121

pot = [sensors.Pot(10), sensors.Pot(8)]
button = [sensors.TouchButton(11), sensors.TouchButton(7)]
button[0].set_threshold(40000)
button[1].set_threshold(40000)
cap = mpr121.MPR121()
hall = [sensors.Pot(2), sensors.Pot(9)]



while True:
#     value = pot[0].read()
#     if value: print('Pot 0', value)
#     
#     value = pot[1].read()
#     if value: print('Pot 1', value)
#     
#     value = button[0].read()
#     if value: print('B 0', value)
#     
#     value = button[1].read()
#     if value: print('B 1', value)
# #     print( button[0].raw(), button[1].raw())
# 
#     cap.update()
#     for i in range(12):
#         value = cap.is_touched(i)
#         if value == "PRESSED": print('cap ', i, value)
#         if value == "RELEASED": print('cap ', i, value)
#         
#     value = hall[0].read()
#     if value: print('hall 0', value)
#     
#     value = hall[1].read()
#     if value: print('hall 1', value)

    print(hall[0].raw(), hall[1].raw())
    
    time.sleep(0.1)
