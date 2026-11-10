import board
import time
import analogio
import digitalio
from sensors.mpr121 import MPR121
import sensors

cap = MPR121()
cap.set_sensitivity(63, 0) # (0-63) and (0-7)

values = [0]*12

pot = sensors.Potentiometer(4)

button = [
    sensors.Button(6),
    sensors.Button(7),
    sensors.Button(8),
    sensors.Button(9)
]

# Create digital output objects for each LED
led1 = digitalio.DigitalInOut(board.D10)
led2 = digitalio.DigitalInOut(board.D11)
led3 = digitalio.DigitalInOut(board.D12)
led4 = digitalio.DigitalInOut(board.D13)

# Set all LEDs as outputs
led1.direction = digitalio.Direction.OUTPUT
led2.direction = digitalio.Direction.OUTPUT
led3.direction = digitalio.Direction.OUTPUT
led4.direction = digitalio.Direction.OUTPUT

index = 0

class Sequence:
    def __init__(self):
        self.start = 0
        self.stop = 8
        self.length = 8
        self.vals = [0]
        
    

while True:
    
    for i in range(12):
        values[i] =   cap.baseline_data(i) - cap.filtered_data(i)
            # print(f"Electrode {i} touched!")
    print( values )
    print( "pot", pot.read() )
    vals = [0,0,0,0]
    for i in range(4):
        vals[i] = button[i].read()
    print("buttons", vals)
    
    index = (index+1) % 8
        
    if index % 2 == 0:
        if index == 0 : led1.value = True
        if index == 2 : led2.value = True
        if index == 4 : led3.value = True
        if index == 6 : led4.value = True
#         print("on")

    else:
        if index == 1 : led1.value = False
        if index == 3 : led2.value = False
        if index == 5 : led3.value = False
        if index == 7 : led4.value = False
#         print("off")
    
    time.sleep(.1)
    


