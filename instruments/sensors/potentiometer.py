import board, analogio

            
class Potentiometer:
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
        self.pin =  getattr(board, f'D{pin}')
        self.pot = analogio.AnalogIn(self.pin)
        self.prev_value = 0
        self.new_value = False
        self.invert = False
        self.threshold = 20

    def new(self):
        """returns false if the value has not changed"""
        value = self.read()
        if self.new_value is True:
            return value
        else:
            return False
        
    def read(self):
        """ returns current value every time"""
        value = self.pot.value >> 4
        self.new_value = False
        if abs( value-self.prev_value ) > self.threshold:
            self.new_value = True
            self.prev_value = value
        return 4095 - value if self.invert else value
    
    def raw(self):
        value = self.pot.value >> 4
        return 4095 - value if self.invert else value 