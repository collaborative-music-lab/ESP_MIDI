import usb_hid
import usb_midi
import microcontroller
import time

clock_speed = 80 #in MHz
microcontroller.cpu.frequency = clock_speed * 1000000

time.sleep(0.1)

# On the S3, we usually have to disable HID to make room for MIDI
usb_hid.disable()
usb_midi.enable()

print("USB MIDI enabled, USB HID disabled")
print("Clock speed set to ", microcontroller.cpu.frequency/1000000, "MHz")