import board, analogio
import math

class Tine:
    def __init__(self, hall_pin, led_r = None, led_g = None):
        self.pin =  getattr(board, f'D{hall_pin}')
        self.hall = analogio.AnalogIn(self.pin)
        #disable leds if pins are not set
        self.led = True if led_r else False
        self.red_pin = led_r
        self.green_pin = led_g
        self.red = 0
        self.green = 255
        self.value = 0
        self.baseline = self.hall.value
        self.range = [40950,1000]
        self.inc = 0.1 # for range adjustment
        self.smoothing = 0.2
    
    def update(self):
        value = self.hall.value # - self.baseline
        
        self.update_range(value)
        if (self.range[1] - self.range[0]) > 0: value = (value-self.range[0]) / (self.range[1] - self.range[0])
        value = value*2 - 1
        self.value = self.smoothing*value + (1-self.smoothing)*self.value
        if self.led:
            if self.value > 0:
                self.green = 0
                self.red = self.value * 255
            else:
                self.red = -self.value * 255
                self.green = -self.value * 255
    
        return self.value
    
    def update_range(self, value):
        self.range[0] += self.inc
        self.range[1] -= self.inc
        if value < self.range[0]: self.range[0] = value
        elif value > self.range[1]: self.range[1] = value

