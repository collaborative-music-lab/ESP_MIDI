import board
import time
import analogio
import digitalio

# my code from other scripts
import sensors, midi

buttons = []
pots = []

analog_pins = [
    board.IO7,
    board.IO2,
    board.IO8,
    board.IO9
]
for pin in analog_pins:
    pots.append( sensors.Pot(pin) )

digital_pins = [
    board.IO3,
    board.IO4,
    board.IO5,
    board.IO6
]
for pin in digital_pins:
    buttons.append( sensors.Button(pin) )

while True:
    value = 0
    for i, pot in enumerate(pots):
        value = pot.read()
        if value is not False:
            print('pot', i, value)
            midi.sendCC(i, value)
    for i, button in enumerate(buttons):
        value = button.read()
        if value is not False:
            print('button', i, value)
            velocity = 100 if value == 'pressed' else 0
            midi.sendNote(i, velocity)
    time.sleep(.1)
    
