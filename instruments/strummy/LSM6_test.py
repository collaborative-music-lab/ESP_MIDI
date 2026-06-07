import time
import board
import busio
# Assuming your class is in a file named imu_driver.py
from BMI160 import IMU 

# Initialize with the correct board type
# Ensure SDA/SCL pins match your physical wiring!
imu = IMU(board_type="BMI160") 

# Note: In the previous class code, init() was called inside __init__
# so calling it again here is fine, but technically redundant.
imu.init_sensor()

print("Starting IMU read loop... Press Ctrl+C to stop.")

while True:
    # Refresh sensor data
    imu.update()
    
    # Format the output for readability
    ax, ay, az = imu.accel_cur
    print(f"Accel X: {ax:.3f}g, Y: {ay:.3f}g, Z: {az:.3f}g")

    time.sleep(0.5)

