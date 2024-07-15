# Arduino Setup
Download the Arduino IDE from Arduino.cc
* there is a web app for Arduino but it won't support the ESP32 so we can't use it.

## Add support for ESP32 to Arduino
Add this link to:
* (MacOS) Arduino->settings->additional board manager URLs
* (Windows) File->preferences->additional board manager URLs
https://dl.espressif.com/dl/package_esp32_index.json

more info on this here: https://medium.com/@pauljoegeorge/setup-arduino-ide-to-flash-a-project-to-esp32-34db014a7e65

# Firmware
Two sets of firmware are provided

## InputMonitor.ino
This firmware allows you to monitor all of your input in the Arduino Serial Monitor
* MAKE SURE you set “USB CDC on boot” to “enabled”. You can find this setting in the Arduino *Tools* menu. The serial monitor will not work if you do not do this!

Once uploaded, open the serial monitor and you should see data streaming from all the sensors. Play with all of the sensors and make sure the data looks good. Take note of any of the potentiometers whose ranges are backward - we can fix this in the other firmware.

## ESP32_MIDI
This is the main firmware for the project, which sets up native USB-midi support. The comment at the top of the ESP32_MIDI.ino file should explain most of the code. 