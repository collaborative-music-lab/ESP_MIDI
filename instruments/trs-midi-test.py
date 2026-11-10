import board
import trs_midi
from mcp4728 import MCP4728

# Initialize MIDI output on GPIO pins (adjust to your pins)
midi = trs_midi.MIDIOutput(tx_pin=board.GP4, channel=0)

# Send a note
midi.send_note_on(60, velocity=100)  # Middle C
time.sleep(0.5)
midi.send_note_off(60)

# Send CC (volume)
midi.send_cc(7, 100)

# Send transport messages
midi.send_start()
for i in range(24):  # 6 quarter notes at 4 clocks per beat
    midi.send_clock()
    time.sleep(0.05)
midi.send_stop()

ppq_counter = 0
while True:
    if ppq_counter == 0:
        midi.send_start()
    midi.send_clock()
    ppq_counter += 1
    if ppq_counter >= 24*4:
        midi.send_stop()
        ppq_counter = 0
    time.sleep(0.05)
    
    
