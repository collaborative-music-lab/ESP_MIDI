# Robin v2

# Recorder inspired instrument With an IMU, 10 touchpads, a touchstrip, and a potentiometer with touchsense on its body.
# Note: this board has a BMI160
# 
# Pins:
# * I2C [43,44] # SCL, SDA
# * 6-DoF IMU, either a BMI160 or a LSM6DS3
# * touchPins[11] = {8, 9, 10, 5, 4, 3, 2, 1, 13, 6, 7};
# * Potentiometer: 11
# * RGB LEDS WS2811 on pin 12
# * RGB LED built-iin WS2811 on pin 21