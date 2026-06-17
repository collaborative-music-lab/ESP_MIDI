#####
# MIDI to CV adaptor
####

import board, analogio, analogio, neopixel, pwmio
import time, math, random

#local files to import
from mcp4728 import MCP4728
import utils, midi, setup, event_handler

from esp_now import ESPNowPeerManager

handler = event_handler.EventHandler()

DEBUG = True

dac = MCP4728()

# use led for status monitoring
pixel = neopixel.NeoPixel(board.NEOPIXEL, 1)
pixel.brightness = 0.1

value = 0
step = (4096/5)/12
pitch = 0
scale = [0,2,4,5,7,9,11,12]



# Initialize manager
manager = ESPNowPeerManager(
    device_name="robin",
    device_type="clock",
    broadcast_interval=1.0
)

# Main loop
prev_time = 0
send_time = 0

def test_print():
    handler.schedule_ms(500,test_print)
    print('time')
    
handler.schedule_ms(1000, test_print )

value = 0
step = math.floor((4095/5)/12)
pitch = 0
scale = [0,2,4,5,7,9,11,12]
index = 0
def handle_dac():
    global value, step, scale, index
    dac.buffer_set(0, value)
    if index % 8 == 0: dac.buffer_set(1, 4095)
    elif index % 8 == 1: dac.buffer_set(1, 0)
    dac.buffer_set(2, value)
    if index % 8 == 0: dac.buffer_set(3, scale[math.floor(index/8)%len(scale)]*step)
    value = (value+500)%4095
    index += 1
    dac.write_all()
    handler.schedule_ms(100,handle_dac)
    print(index, scale[math.floor(index/8)%len(scale)]*step / 4095 * 5)

# handler.schedule_ms(100,handle_dac)
    
clock_timer = 0
bpm = 100
bpm_seconds = 60/bpm

while True:
    now = time.monotonic()
    
    if now  - clock_timer > bpm_seconds:
        clock_timer = now
        dac.set(0,4095)
        handler.schedule_ms(10, lambda: dac.set(0,0))
    
    current_time = time.monotonic()
    handler.update()
    
    # check for new msgs
    if now - prev_time >= 0.001:
        prev_time = now
        # Broadcast presence periodically
        manager.broadcast_presence()
        
        # Listen for incoming messages and peer discoveries
        msg = manager.handle_incoming()
        if msg: print('incoming', msg)
        
    # send msgs
    if now - send_time >= 1:
        send_time = now
        
#         # Send to specific peer by index
#         if len(manager.peers) > 0:
#             manager.send_to_peer(0, "Hello peer!")
        
        # Send to specific peer by name
        if len(manager.peers) > 0:
            manager.send_to_peer("testy01", "Hi testy01!")
        
#         # Broadcast to all peers
#         if len(manager.peers) > 0:
#             manager.broadcast("Broadcast message!")
        
        # Print peers periodically
        manager.print_peers()



    
    
                    
