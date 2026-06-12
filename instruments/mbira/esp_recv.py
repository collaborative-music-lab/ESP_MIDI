import espnow
import wifi
import time

receiver = True

# Initialize WiFi, set to channel 6 where ESP-NOW lives
wifi.radio.start_ap(" ", "", channel=6, max_connections=0)
wifi.radio.stop_ap()
my_mac = wifi.radio.mac_address
print(f"My MAC: {my_mac.hex()}")

e = espnow.ESPNow()
peer = espnow.Peer(mac=b'\xFF\xFF\xFF\xFF\xFF\xFF',channel=6)
e.peers.append(peer)
print("Peer added")

e.send("Starting...", peer)
if receiver: print("listening")

prev_time = 0
while True:
    if time.monotonic() - prev_time > 1:
        prev_time = time.monotonic()
    
        if receiver:
            if not e:  # wait for a packet
                continue
            packet = e.read()
            print("packet message:", packet.msg)
            print("full packet:", packet)
            
        else:
            for i in range(10):
                e.send(str(i)*20, peer)
            e.send(b'end', peer)
            print('sending')
