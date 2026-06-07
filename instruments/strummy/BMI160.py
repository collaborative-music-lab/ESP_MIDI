import board
import busio
import time
import struct

class IMU:
    def __init__(self, board_type="LSM6", SDA=board.IO43, SCL=board.IO44):
        self.i2c = busio.I2C(SCL, SDA) # Note: SCL is usually first in busio.I2C
        self.BOARD = board_type
        # LSM6 0x6B, BMI 0x69
        self.DEVICE_ADDRESS = 0x6B if self.BOARD == "LSM6" else 0x69 
        
        # Default Registers (LSM6DS3)
        self.ACCEL_START_REG = 0x28 
        self.GYRO_START_REG = 0x22 
        self.CTRL1_XL = 0x10  # Accel config
        self.CTRL2_G = 0x11   # Gyro config
        self.STATUS_REG = 0x1E 
        self.POWER_CMD = 0x7E # Specific to BMI160
        
        if self.BOARD == "BMI160":
            self.ACCEL_START_REG = 0x12 
            self.GYRO_START_REG = 0x0C 
            self.CTRL1_XL = 0x40 
            self.CTRL2_G = 0x42  
            self.STATUS_REG = 0x1B 
        
        self.accel_cur = [0.0, 0.0, 0.0]
        self.gyro_cur = [0.0, 0.0, 0.0]
        self.buffer = bytearray(6)
        
        self.init_sensor()
        
    def init_sensor(self):
        if self.BOARD == "LSM6":
            # Enable Accel (104Hz, 2g) and Gyro (104Hz, 2000 dps)
            self.write_register(self.CTRL1_XL, 0x40) 
            self.write_register(self.CTRL2_G, 0x40)
        elif self.BOARD == "BMI160":
            # BMI160 Power-up sequence
            self.write_register(self.POWER_CMD, 0x11) # Accel to Normal Mode
            time.sleep(0.1)
            self.write_register(self.POWER_CMD, 0x15) # Gyro to Normal Mode
            time.sleep(0.1)

    def update(self):
        status = self.read_register(self.STATUS_REG)
        
        # Handle different status bits per chip
        if self.BOARD == "LSM6":
            accel_ready = (status & 0x01)
            gyro_ready = (status & 0x02)
        else: # BMI160
            accel_ready = (status & 0x80)
            gyro_ready = (status & 0x40)
        
        if accel_ready:
            self.read_into_buffer(self.ACCEL_START_REG)
            raw = struct.unpack('<hhh', self.buffer)
            # 0.000061 is for +/- 2g scale
            self.accel_cur[:] = [val * 0.000061 for val in raw]
        
        if gyro_ready:
            self.read_into_buffer(self.GYRO_START_REG)
            raw = struct.unpack('<hhh', self.buffer)
            # 0.0001527 is for 125/250 dps (Check your sensor's FS setting!)
            self.gyro_cur[:] = [val * 0.0001527 for val in raw] 
            
        return status

    def read_into_buffer(self, reg):
        """Helper to handle I2C locking and reading."""
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto_then_readfrom(self.DEVICE_ADDRESS, bytes([reg]), self.buffer)
        finally:
            self.i2c.unlock()

    def write_register(self, reg, value):
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto(self.DEVICE_ADDRESS, bytes([reg, value]))
        finally:
            self.i2c.unlock()
            
    def read_register(self, reg):
        while not self.i2c.try_lock(): pass
        try:
            result = bytearray(1)
            self.i2c.writeto_then_readfrom(self.DEVICE_ADDRESS, bytes([reg]), result)
            return result[0]
        finally:
            self.i2c.unlock()