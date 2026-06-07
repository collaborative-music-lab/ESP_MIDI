import board
import busio
import time
import adafruit_lsm6ds.lsm6ds3  # Import the specific driver

# 1. Setup I2C
# Most boards use board.SCL and board.SDA
i2c = busio.I2C(43,44)

# 2. Initialize the Sensor
imu = adafruit_lsm6ds.lsm6ds3.LSM6DS3(i2c)
print("LSM6DS3 Initialized!")
