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
        # set peak
        
        if raw > self.peak: self.peak = raw
        elif self.peak > self.baseline - 10000: self.peak -= 1
        if (self.peak - self.baseline) > 10000:
            # update baseline
#             if not self.button.value or self.calibration_mode:
            if raw < self.baseline:
                self.baseline = raw
                new_threshold =  self.baseline + self.threshold 
                if new_threshold < 35535 :  self.button.threshold = math.floor(new_threshold)
            elif self.baseline < self.peak - 10000: self.baseline += self.baseline_smoothing
            
            if self.peak - self.baseline > 0: self.value = (raw - self.baseline)/(self.peak - self.baseline)
#         if self.calibration_mode:
#             if self.value < 50:
#                 self.calibration_mode = False
#                 self.set_threshold(self.threshold)
#         print(raw, self.value, self.baseline)
#         self.state = self.get_state()
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
  