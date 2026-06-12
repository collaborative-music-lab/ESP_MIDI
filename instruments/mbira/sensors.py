# sensors.py
import analogio, digitalio, touchio, board, math

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

    def read(self):
        value = self.pot.value >> 9
        new_value = False
        if value != self.prev_value:
            new_value = True
        self.prev_value = value
        if new_value is True:
            return value
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
        self.button.threshold = 100
        self.prev_state = False
        self.value = self.button.raw_value
        self.prev_raw = self.button.raw_value
        self.state = "up"
        self.smoothing = 0.5
        self.baseline = self.button.raw_value
        self.baseline_smoothing = 10
        self.threshold = 100
        
    def update(self):
        raw = self.button.raw_value * (1-self.smoothing) + self.prev_raw * self.smoothing
        self.prev_raw = raw
#         print(raw, self.prev_raw, self.baseline)
        if not self.button.value:
            if raw < self.baseline:
                self.baseline = raw
                new_threshold =  self.baseline + self.threshold 
                if new_threshold < 65535 :  self.button.threshold = math.floor(new_threshold)
            else: self.baseline += self.baseline_smoothing
        self.value = raw - self.baseline
#         print(raw, self.value, self.baseline)
#         self.state = self.get_state()
        
    def get_state(self):
        """
        Returns:
        - "pressed"
        - "released"
        - None
        """
        current = self.button.value

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
        current = self.button.value

        if current != self.prev_state:
            self.prev_state = current
            return "pressed" if current else "released"

        return False
    
    def raw(self):
        return self.button.raw_value
    
    def set_threshold(self, value):
        try:
            self.threshold = value
            self.button.threshold = self.button.raw_value + self.threshold
        except Exception as e:
            print("bad threshold ", self.button.raw_value + self.threshold, e)
    
    