import touchio, board

class TouchSensor:
    def __init__(self, pin, sensitivity_offset=150):
        """
        Initializes a capacitive touch pin with custom sensitivity tuning.
        :param pin_object: The board pin (e.g., board.A2, board.IO4)
        :param sensitivity_offset: How much higher than the baseline a reading 
                                   must be to trigger a touch.
        """
        # Initialize the hardware
        self.pin = pin
        self.sensor = touchio.TouchIn(getattr(board, f"IO{pin}"))
        
        # Manually tune the sensitivity threshold relative to its initial baseline
        #self.sensor.threshold = self.sensor.raw_value + sensitivity_offset

    @property
    def raw(self):
        """Returns the raw capacitance value."""
        return self.sensor.raw_value

    @property
    def is_touched(self):
        """Returns True if touched, False if not."""
        return self.sensor.value

    @property
    def threshold(self):
        """Returns the current threshold setting."""
        return self.sensor.threshold