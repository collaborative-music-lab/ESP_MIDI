# Driver for IS31FL3218 I2C LED Driver
# 18 LED outputs

import board
import busio
import time
import struct

class LedDriver:
    def __init__(self, i2c, address=0x54):
        self.i2c = i2c
        self.DEVICE_ADDRESS = address
        
        # Default Registers (LSM6DS3)
        self.LED_START_REG = 0x01
        self.WRITE_REG = 0x16
        self.RESET_REG = 0x17  # Accel config
        self.SHUTDOWN_REG = 0x00   # Gyro config
        
        self.led_buffer = [0 for i in range(18)] # 18 PWM bytes
        
        
        self.init_driver()
        self.new_data = False
        
        self.poll_rate = 0.05 #in seconds
        self.prev_time = 0
        
        # 64-step gamma correction table
        self.gamma_table = [
                0,   1,   2,   3,   4,   5,   6,   7,
                8,  10,  12,  14,  16,  18,  20,  22,
                24,  26,  29,  32,  35,  38,  41,  44,
                47,  50,  53,  57,  61,  65,  69,  73,
                77,  81,  85,  89,  94,  99, 104, 109,
                114, 119, 124, 129, 134, 140, 146, 152,
                158, 164, 170, 176, 182, 188, 195, 202,
                209, 216, 223, 230, 237, 244, 251, 255
            ]
        
    def init_driver(self):
        # Shutdown Register - 0x00: 1 = Normal, 0 = Shutdown
        self.write_register(0x00, 0x01)
        
        # Enable all 18 channels (3 control registers)
        # 0x13, 0x14, 0x15: Each bit enables one channel (1-6, 7-12, 13-18)
        self.write_register(0x13, 0x3F)  # 0011 1111 = channels 1-6 enabled
        self.write_register(0x14, 0x3F)  # channels 7-12 enabled
        self.write_register(0x15, 0x3F)  # channels 13-18 enabled
        
        # Reset: Write to Update Register to latch initial state
        self.write_register(0x16, 0x00)

    def update(self):
        if time.monotonic() - self.prev_time > self.poll_rate:
            self.prev_time = time.monotonic()
        else: return 0
        
        if not self.new_data: return 0
        
        status = True
        
        try:
            self.write_registers(self.LED_START_REG, self.led_buffer)
            self.new_data = False
        except Exception as e:
            print(e)
            status = False
        
        return status
    
    def set(self, num, val):
        val = max(0, min(63, val/4))  # Clamp to 0-63
        val = int(val)  # Convert float to int
        self.led_buffer[num] = self.gamma_table[val]
        self.new_data = True
        
    def write_register(self, reg, value):
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto(self.DEVICE_ADDRESS, bytes([reg, value]))
        finally:
            self.i2c.unlock()

    # Single I2C write with auto-increment
    def write_registers(self, start_register, arr):
        while not self.i2c.try_lock(): pass
        try:
            # Build entire message: [register, value1, value2, ..., value18]
            message = bytes([start_register]) + bytes(arr)
            self.i2c.writeto(self.DEVICE_ADDRESS, message)
            
            # Latch the values
            self.i2c.writeto(self.DEVICE_ADDRESS, bytes([self.WRITE_REG, 0xFF]))
        finally:
            self.i2c.unlock()

