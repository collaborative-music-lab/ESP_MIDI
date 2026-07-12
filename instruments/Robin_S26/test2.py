import board
import busio

i2c = busio.I2C(board.D43, board.D44)
while not i2c.try_lock(): pass

address = 0x69
buffer = bytearray(1)

# Try reading Register 0x75 (Common for MPU series)
i2c.writeto_then_readfrom(address, bytes([0x75]), buffer)
print(f"ID at Reg 0x75: {hex(buffer[0])}")

# Try reading Register 0x00 (Common for Bosch/BMI series)
i2c.writeto_then_readfrom(address, bytes([0x00]), buffer)
print(f"ID at Reg 0x00: {hex(buffer[0])}")

i2c.unlock()