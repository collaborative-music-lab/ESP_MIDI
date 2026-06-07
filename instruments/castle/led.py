import pwmio

class RGBLed:
    def __init__(self, red_pin, green_pin, blue_pin):
        # Initialize PWM on each pin at 5000Hz for flicker-free light
        self.red = pwmio.PWMOut(red_pin, frequency=5000, duty_cycle=0)
        self.green = pwmio.PWMOut(green_pin, frequency=5000, duty_cycle=0)
        self.blue = pwmio.PWMOut(blue_pin, frequency=5000, duty_cycle=0)

    def set_color(self, r, g, b):
        """
        Sets brightness for R, G, and B.
        Expects values from 0.0 to 1.0.
        """
        # Map 0.0-1.0 to 0-65535
        self.red.duty_cycle = int(r * 65535)
        self.green.duty_cycle = int(g * 65535)
        self.blue.duty_cycle = int(b * 65535)

    def off(self):
        self.set_color(0, 0, 0)