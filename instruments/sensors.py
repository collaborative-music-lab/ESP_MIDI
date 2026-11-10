# sensors.py
import analogio, digitalio

class Pot:
    """
    Handles ADC input, optimized for potentiometers.
    - argument pin: raw integer of pin number
    - invert: flips output to account for potentiometers wired backwards
    - threshold: test for whether the value of the potentiometer has changed
    
    Methods:
    - read(): returns raw value
    - new(): returns False is the value has not changed past the threshold, else returns the value
    """
    
    def __init__(self, pin):
        self.pot = analogio.AnalogIn(pin)
        self.prev_value = 0
        self.new_value = False
        self.invert = False
        self.threshold = 100

    def new(self):
        """returns false if the value has not changed"""
        value = self.read()
        if new_value is True:
            return value
        else:
            return False
        
    def read(self):
        """ returns current value every time"""
        value = self.pot.value >> 4
        new_value = False
        if abs( value-self.prev_value ) > self.threshold:
            new_value = True
        self.prev_value = value
        return value

class Button:
    """
    Represents one digital button.

    Handles:
    - Digital input setup
    - Edge detection (press / release)
    - Previous state tracking
    """

    def __init__(self, pin):
        self.button = digitalio.DigitalInOut(pin)
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
    
    