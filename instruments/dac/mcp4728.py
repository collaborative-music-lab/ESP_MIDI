import board
import busio
import time
import struct

class MCP4728:
    def __init__(self, SDA=board.IO44, SCL=board.IO43):
        self.i2c = busio.I2C(SCL, SDA) # Note: SCL is usually first in busio.I2C
        self.DEVICE_ADDRESS = 0x60
        # Default Registers
        
        self.prev_time = 0
        self.poll_rate = 0
        self.val = [0,0,0,0]
        
        try:
            value = self.read_register(0x00)  # Read status or first register
            print(f"Device responded with: {hex(value)}")
        except Exception as e:
            print(f"Error: {e}")
            
    def set(self, channel, value):
        """Single channel, immediate write"""
        cmd = 0x58 | (channel << 1) | 1  # UDAC=1 to update immediately
        byte1 = (value >> 8) & 0x0F      # Upper 4 bits
        byte2 = value & 0xFF             # Lower 8 bits
        self.write_register(cmd, byte1, byte2)
        self.val[channel] = value
        
    def buffer_set(self, channel, value):
        """Store value in buffer (don't send yet)"""
        self.val[channel] = value
        
    def write_all(self):
        """Send all buffered values at once (fast write)"""
        data = []
        for channel_value in self.val:
            byte1 = (channel_value >> 8) & 0x0F  # Upper 4 bits
            byte2 = channel_value & 0xFF         # Lower 8 bits
            data.extend([byte1, byte2])
        self.write_register(*data)

    def update(self):
        if time.monotonic() - self.prev_time > self.poll_rate:
            self.prev_time = time.monotonic()
        else: return 0
        
#         status = self.read_register(self.STATUS_REG)
        
    def read_into_buffer(self, reg):
        """Helper to handle I2C locking and reading."""
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto_then_readfrom(self.DEVICE_ADDRESS, bytes([reg]), self.buffer)
        finally:
            self.i2c.unlock()

    def write_register(self, *data):
        """Write variable-length data to device."""
        while not self.i2c.try_lock(): pass
        try:
            self.i2c.writeto(self.DEVICE_ADDRESS, bytes(data))
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


