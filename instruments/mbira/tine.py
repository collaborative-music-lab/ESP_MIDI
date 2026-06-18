import board, analogio, json, storage, math

class Tine:
    def __init__(self, instance):
        self.instance = instance
        self.settings = self.read_json("tine_settings.json")
        print(self.settings)
        self.pin =  self.settings["pin"]
        self.hall = analogio.AnalogIn(getattr(board, f'D{self.pin}'))
        
        #disable leds if pins are not set
        self.led = self.settings["red_pin"]
        self.red_pin = self.settings["red_pin"]
        self.green_pin = self.settings["green_pin"]
        self.red = 0
        self.green = 255
        
        #data
        self.raw = self.hall.value
        self.value = self.raw
        self.baseline = self.raw
        self.min = self.settings["min_range"]
        self.max = self.settings["max_range"]
        self.inc = 0.1 # for range adjustment
        self.smoothing = 0.2
        self.midpoint = self.settings["midpoint"]
        self.delta = 0
        self.inverted = self.settings["inverted"]
        
    def read(self):
        """Read and average multiple samples"""
        samples = 0
        for _ in range(4):
            samples += self.hall.value  # 10 conversions
        self.raw = samples // 4
    
    def process(self):
        # read data and normalize
        value = self.raw
        self.update_range(value)
        if (self.max - self.min) > 0: value = (value-self.min) / (self.max - self.min)
        value = value*2 - 1
        if self.inverted: value = value * -1
        #calc delta
        delta = abs(self.value - value)
        if delta > 0.05: self.delta += delta
        #smooth self.value
        self.value = self.smoothing*value + (1-self.smoothing)*self.value
        #update midpoint
        self.midpoint = 0.01*value + (1-0.01)*self.midpoint
        
        if self.led:
            if self.value > 0:
                self.green = 0
                self.red = self.value * 255
            else:
                self.red = -self.value * 255
                self.green = -self.value * 255
    
        return self.value - self.midpoint
    
    def update_range(self, value):
        self.min += self.inc
        self.max -= self.inc
        if value < self.min: self.min = value
        elif value > self.max: self.max = value
        
    def calibrate(self):
        self.process()
        self.midpoint = self.value
        print(self.midpoint, self.min, self.max)
        self.min = 60000
        self.max = 100
        
        
    def get_delta(self):
        val = self.delta
        self.delta = 0
        return val
    
    def get_value(self):
        return self.value - self.midpoint

    def read_json(self, filepath="tine_settings.json"):
        try:
            with open(filepath, 'r') as f:
                config = json.load(f)
        except (OSError, ValueError) as e:
            print(f"Failed to load {filepath}: {e}")
            return {}
        
        try:
            settings = config[str(self.instance)]  
            return settings
        except KeyError:
            print(f"Instance {self.instance} not found in config")
            return {}

    def save_calibration(self, filepath='tine_settings.json'):
        try:
            storage.remount('/', readonly=False)  # Unlock temporarily
            print(self.instance, self.min)
            
            with open(filepath, 'r') as f:
                config = json.load(f)
            
            config[str(self.instance)] = {
                'pin': self.pin,
                'red_pin': self.red_pin,
                'green_pin': self.green_pin,
                'min_range': self.min,
                'max_range': self.max,
                'midpoint': self.midpoint,
                'inverted': self.inverted
            }
            
            # Manually format with newlines
            with open(filepath, 'w') as f:
                json_str = json.dumps(config)
                # Add newlines after commas and colons
                json_str = json_str.replace(',', ',\n')
                f.write(json_str)
            
            storage.remount('/', readonly=True)  # Lock again
            print("Saved successfully")
            return True
        except Exception as e:
            print(f"Failed to save: {e}")
            return False



