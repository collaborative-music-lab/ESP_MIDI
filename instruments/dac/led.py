import pwmio

class Led:
    def __init__(self, pin):
        # Initialize PWM on each pin at 5000Hz for flicker-free light
        self.pin = pwmio.PWMOut(pin, frequency=5000, duty_cycle=0)

    def set(self, val):
        """
        Expects values from 0.0 to 1.0.
        """
        # Map 0.0-1.0 to 0-65535
        self.pin.duty_cycle = int(val * 65535)
    def off(self):
        self.set(0)