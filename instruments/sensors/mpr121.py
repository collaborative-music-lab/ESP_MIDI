import board
import busio
import adafruit_mpr121
import time
import utils
import setup

# Create the I2C bus interface
# i2c = busio.I2C(board.IO44, board.IO43)

# Initialize the MPR121 sensor
# Note: The default I2C address is 0x5A
class MPR121(adafruit_mpr121.MPR121):
    def __init__(self, i2c = busio.I2C(board.IO44, board.IO43, frequency=100000), address=0x5A, num_sensors = 12):
        super().__init__(i2c, address)
        self.num_sensors = num_sensors
        
        # Local buffers for fast access
        self.filtered_cache = [0] * num_sensors
        self.baseline_cache = [0] * num_sensors
        self.delta_cache = [0] * num_sensors
        
        # keep track of touch state
        self.triggers = [utils.SchmittTrigger(lo=40, hi=100, inverted=False) for _ in range(num_sensors)]
        self.touch_states = [False] * num_sensors
        
        # buffers for raw data
        self._filt_raw = bytearray(num_sensors*2)
        self._base_raw = bytearray(num_sensors)
        
    def update(self):
        """Fetches all sensor data in two bursts for maximum speed and resolution."""

        # Read all Data at once
        i2c_read_success = 0
        try:
            self._read_register_bytes(0x04, self._filt_raw)
            self._read_register_bytes(0x1E, self._base_raw)
            i2c_read_success = 1
        except OSError as e:
            if e.args[0] == 5:
                print("I2C Communication Error - Skipping this frame")
            else:
                print(e)
        
        if i2c_read_success == 0: return
        
        for i in range(self.num_sensors):
            # Combine two 8-bit bytes into one 10-bit integer
            # self._filt_raw[i*2] is the Low Byte, self._filt_raw[i*2+1] is the High Byte
            f = self._filt_raw[i * 2] | (self._filt_raw[i * 2 + 1] << 8)
            
            # Baseline is only 8-bit (internally shifted by the chip)
            # To compare it to 10-bit filtered data, we must shift it left by 2
            b = self._base_raw[i] << 2
            
            # keep track of difference between filtered and baseline to get current capsense value
            self.filtered_cache[i] = f
            self.baseline_cache[i] = b
            self.delta_cache[i] = max(0, b - f)
            
            # Update triggers based on current data
            self.touch_states[i] = self.triggers[i].update(self.delta_cache[i])
#             print(self.touch_states)
            
    def read(self, electrode):
        """Returns the proximity/touch delta from the local cache."""
        if electrode >= self.num_sensors:
            printf("invalid sensor number. There are only {self.num_sensors} pads initialized. Check the MPR121 constructor.")
            return 0
        return self.delta_cache[electrode]
    
    def is_touched(self, i):
        if i >= self.num_sensors:
            printf("invalid sensor number. There are only {self.num_sensors} pads initialized. Check the MPR121 constructor.")
            return 0
        return self.touch_states[i]
    
    def set_sensitivity(self, current, charge_time):
        """
        Custom method to set AFE1 and AFE2 registers.
        current: 0-63 (uA)
        charge_time: 0-7 (0.5us to 32us)
        """
        # Clamp current safely
        current = min(max(current, 0), 63)
        # Clamp time bits
        charge_time = min(max(charge_time, 1), 7)
        
        # 1. Enter Stop Mode (must be 0x00 to edit config)
        self._write_register_byte(0x5E, 0x00)
        
        # 2. Set Charge Current (AFE1)
        self._write_register_byte(0x5C, current)
        
        # 3. Set Charge Time (AFE2) 
        # Shift bits to the top 3 positions of the register
        self._write_register_byte(0x5D, (charge_time << 5))
        
        print(f"MPR Charge Settings Updated: Current={current}uA, Time={charge_time}")
        
        # After changing sensitivity, the baseline needs a moment to settle
        time.sleep(0.5)
        self._write_register_byte(0x5E, 0x00)
        self._write_register_byte(0x5E, 0x3F)
#         
       
#     def update_threshold(self, threshold):
#         # Set software thresholds
#         # These are used by the hardware to trigger the 'value' property
#         for i in range(12):
#             self[i].touch_threshold = threshold
#             self[i].release_threshold = threshold/2

    def set_proximity_sensitity(self, current, charge_time):
        """
        Custom method to set AFE1 and AFE2 registers.
        current: 0-63 (uA)
        charge_time: 0-7 (0.5us to 32us)
        """
        # Clamp current safely
        current = min(max(current, 0), 63)
        # Clamp time bits
        charge_time = min(max(charge_time, 1), 7)
        
        # 1. Enter Stop Mode (must be 0x00 to edit config)
        self._write_register_byte(0x5E, 0x00)
        
        # 2. Set Charge Current (AFE1)
        self._write_register_byte(0x5F, current)
        
        # 3. Set Charge Time (AFE2) 
        # Shift bits to the top 3 positions of the register
        self._write_register_byte(0x60, (charge_time << 5))
        
        print(f"MPR Proximity Settings Updated: Current={current}uA, Time={charge_time}")
        
        # After changing sensitivity, the baseline needs a moment to settle
        time.sleep(0.5)
        self._write_register_byte(0x5E, 0x00)
        self._write_register_byte(0x5E, 0x3F)
        
    def read_proximity_raw(self):
        # The proximity data is stored in registers 0x1C (LSB) and 0x1D (MSB)
        # We must read them as a 2-byte chunk
        buf = bytearray(2)
        self._read_register_bytes(0x1C, buf)
        
        # Assemble the 10-bit value
        # Filtered Data = LSB | (MSB << 8)
        return buf[0] | (buf[1] << 8)

    def read_proximity_baseline(self):
        # Proximity Baseline is at 0x2A
        buf = bytearray(1)
        self._read_register_bytes(0x2A, buf)
        return buf[0] << 2  # Scale 8-bit baseline to 10-bit
    
    def reset(self):
        """Forces the sensor to recalibrate to the current environment. """
        self._write_register_byte(0x80, 0x63)
        time.sleep(.1)
        self._write_register_byte(0x5E, 0x00)
        self._write_register_byte(0x5E, 0x3F)
        self.set_sensitivity(current=setup.charge_current, charge_time=setup.charge_time) #current from 1-63, charge_time from 1 to 7

