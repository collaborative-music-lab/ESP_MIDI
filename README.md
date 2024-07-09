# ESP_MIDI
This repo contains hardware and software components for creating MIDI controllers using the ESP32-s3 microcontroller. The native USB port on the s3 allows for class-compliant MIDI devices which should be recognized natively by any computer.
The repo consists of:
1. **PCB**: A PCB design created in KiCad, along with PDF’s of the board and schematic
2. **Enclosure**: 3d-printable faceplates and knobs
3. **Software**: Arduino-based software for programming your MIDI controller
Each subfolder contains its own readme with additional information for each stage
# FYI / FAQS
There are several things you need to know.
### Allowing for class-compliant midi
To let ESP32-s3 appear as a class compliant device, you *must* set the ‘USB Mode’ for the board as “USB OTG(TinyUSB)”. You can find this setting in the Arduino *Tools* menu.
### Reprogramming after setting USB OTG
Once the board is set to USB OTG mode it won’t show up in Arduino as a Serial device. In order to reprogram the device you must manually hold down the ‘boot’ button on the ESP32-s3, and then press press and release the ‘reset’ on the ESP32-s3.
### Allowing for Serial debugging
In order to use serial.print debugging, you *must* set “USB CDC on boot” to “enabled”. You can find this setting in the Arduino *Tools* menu.
