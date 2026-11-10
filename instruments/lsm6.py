import busio
import struct
import time

class IMU:
    def __init__(self, scl_pin, sda_pin, address=0x6B):
        self.address = address
        self.i2c = busio.I2C(scl_pin, sda_pin)
        
        # 1. Wake up the sensor (Set accelerometer to 104Hz, +/- 4g)
        # Register 0x10 is CTRL1_XL
        self._write_register(0x10, 0x48) 
        print("LSM6DS3 (0x69) Raw Driver Initialized!")

    def _write_register(self, reg, value):
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto(self.address, bytes([reg, value]))
        finally:
            self.i2c.unlock()

    def _read_registers(self, reg, count):
        while not self.i2c.try_lock(): pass
        buffer = bytearray(count)
        try:
            self.i2c.writeto_then_readfrom(self.address, bytes([reg]), buffer)
        finally:
            self.i2c.unlock()
        return buffer

    @property
    def acceleration(self):
        # Read 6 bytes starting at 0x28 (X, Y, Z low/high bytes)
        data = self._read_registers(0x28, 6)
        
        # Unpack little-endian signed shorts (<hhh)
        raw_x, raw_y, raw_z = struct.unpack('<hhh', data)
        
        # Conversion for +/- 4g range (0.122 mg/LSB)
        # Logic: (raw_value * 0.122 / 1000) * 9.80665
        conv = 0.001196  # Pre-calculated multiplier for m/s^2
        return (raw_x * conv, raw_y * conv, raw_z * conv)

    def get_led_values(self, sensitivity=10.0):
        ax, ay, az = self.acceleration
        r = min(abs(ax) / sensitivity, 1.0)
        g = min(abs(ay) / sensitivity, 1.0)
        b = min(abs(az) / sensitivity, 1.0)
        return (r, g, b)
