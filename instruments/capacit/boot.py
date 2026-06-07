import usb_hid
import usb_midi

# On the S3, we usually have to disable HID to make room for MIDI
usb_hid.disable()
usb_midi.enable()

print("USB MIDI enabled, USB HID disabled")