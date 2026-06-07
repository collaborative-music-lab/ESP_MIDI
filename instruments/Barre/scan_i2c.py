import time
import board
import busio

# Initialize I2C using the board's default SCL and SDA pins
i2c = busio.I2C(board.D44, board.D43)

def scan_i2c():
    
    # The bus must be locked before scanning
    while not i2c.try_lock():
        pass

    try:
        print("I2C Scanner active...")
        print("Scanning...")
        
        # Scan returns a list of decimal addresses
        addresses = i2c.scan()
        
        if not addresses:
            print("No I2C devices found! Check your wiring and pull-up resistors.")
        else:
            print(f"Found {len(addresses)} device(s):")
            for address in addresses:
                # Print in both Decimal and Hexadecimal (Hex is standard for datasheets)
                print(f"- Decimal: {address} | Hex: {hex(address)}")
                
    finally:
        # Always unlock the bus, even if the scan fails
        i2c.unlock()

while True:
    scan_i2c()
    time.sleep(5)  # Rescan every 5 seconds