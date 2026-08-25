# sensors.py
import analogio, digitalio, touchio, board, math, time

class Pot:
    """
    Represents one potentiometer.
    
    Stores:
    - ADC reference
    - previous value (for change detection)
    """
    
    def __init__(self, pin):
        self.pin =  getattr(board, f'D{pin}')
        self.pot = analogio.AnalogIn(self.pin)
        self.prev_value = 0
        self.filtered = self.pot.value
        self.alpha = 0.95
        self.threshold = 400
        self.max = 61960
        self.dead = [0,60000]

    def read(self):
        self.filtered = (1-self.alpha)*self.pot.value + self.alpha*self.filtered
        value = math.floor(self.filtered)
#         if value < 50000:
#             if value < self.dead[1]: self.dead[1] = value
#             if value > self.dead[0]: self.dead[0] = value
#             print(self.dead)
# #         print (self.pot.value)
        if value > 33000 and value < 34000: return False
        
        new_value = False
        if abs(value -  self.prev_value) > self.threshold:
            new_value = True
            self.prev_value = value
        if new_value is True:
#             print (value)
            return value >> 9
        else:
            return False
        
    def raw(self):
        self.prev_value = self.pot.value >> 9
        return self.prev_value

class Button:
    """
    Represents one digital button.

    Handles:
    - Digital input setup
    - Edge detection (press / release)
    - Previous state tracking
    """

    def __init__(self, pin):
        self.pin =  getattr(board, f'D{pin}')
        self.button = digitalio.DigitalInOut(self.pin)
        self.button.direction = digitalio.Direction.INPUT
        self.button.pull = digitalio.Pull.UP

        self.prev_state = self.button.value

    def read(self):
        """
        Returns:
        - "pressed"
        - "released"
        - None
        """
        current = self.button.value

        if current != self.prev_state:
            self.prev_state = current
            return "pressed" if not current else "released"

        return False
    
class TouchButton:
    """
    Represents one capsense button.

    Handles:
    - Digital input setup
    - Edge detection (press / release)
    - Previous state tracking
    """

    def __init__(self, pin):
        self.pin =  getattr(board, f'D{pin}')
        self.button = touchio.TouchIn(self.pin)
        time.sleep( 0.1 )
        self.button.threshold = 100
        self.prev_state = False
        self.value = 0
        self.prev_raw = 0
        self.state = "up"
        self.smoothing = 0.5
        self.baseline = 40000
        self.baseline_smoothing = 1
        self.threshold = 0.3
        self.calibration_mode = False
        self.peak = 50000
        
    def calibrate(self):
        self.calibration_mode = True
        self.button.threshold = 10000
        
    def update(self):
        # read and lowpass filter
        raw = self.button.raw_value * (1-self.smoothing) + self.prev_raw * self.smoothing
        self.prev_raw = raw
        
        # updatebaseline and peak
        if raw < self.baseline:
                self.baseline = raw
        if raw > self.peak: self.peak = raw
        
        if (self.peak - self.baseline) > 5000:
            self.peak -= 1
            self.baseline += self.baseline_smoothing
            
        elif self.peak > 65000: self.peak = 65000
        else: self.peak = self.baseline+5000
        
        if self.peak - self.baseline > 0:
            self.value = (raw - self.baseline) / (self.peak - self.baseline)
        
    def get_state(self):
        """
        Returns:
        - "pressed"
        - "released"
        - None
        """
        current = self.value > self.threshold

        if current != self.prev_state:
            self.prev_state = current
            self.state = current
            self.state =  "pressed" if current else "released"
        else:
            self.state = "down" if current else "up"
        return self.state
        
    def read(self):
        """
        Returns:
        - "pressed"
        - "released"
        - None
        """
        current = self.value > self.threshold

        if current != self.prev_state:
            self.prev_state = current
            return "pressed" if current else "released"

        return False
    
    def raw(self):
        return self.button.raw_value
    
    def set_threshold(self, value):
        try:
            self.threshold = value
            self.baseline = self.button.raw_value
#             self.button.threshold = self.button.raw_value + self.threshold
        except Exception as e:
            print("bad threshold ", self.button.raw_value + self.threshold, e)
    
    