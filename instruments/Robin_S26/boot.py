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

if microcontroller.nvm[0] == 1:
    # Edit mode: everything on
    usb_midi.enable()
    storage.enable_usb_drive()
    usb_cdc.enable(console=True, data=False)
    print("Booting with USB drive ENABLED for editing")
else:
    # Normal mode: MIDI on, mass storage off
    storage.disable_usb_drive()
    usb_midi.enable()
    usb_cdc.enable(console=True, data=False)
    print("Booting with USB drive disabled, MIDI enabled")