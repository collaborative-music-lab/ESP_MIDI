# utility classes for data processing
# Includes:
#     - SchmittTrigger
#     - OnePole
#     - OneEuroFilter
#     - Scale

import math
import time

class SchmittTrigger:
    # States
    states = {
    'RELEASED', #  # On transition from high ot low
    'HIGH',  # While being held down
    'PRESSED',   # On transition from low to high
    'LOW'     # When button is not held down
    }

    def __init__(self, lo, hi, inverted=False):
        """
        inverted=False: Trigger when value > hi (Capacitive, Active High)
        inverted=True:  Trigger when value < lo (Pulled-up Button, Active Low)
        """
        self.lo = lo
        self.hi = hi
        self.inverted = inverted
        self.state = 'RELEASED'

    def update(self, value):
        # Normalize the logic: if inverted, we treat 'low' as 'high'
        is_above_hi = value > self.hi
        is_below_lo = value < self.lo

        # Determine if the physical signal meets our "Active" criteria
        active_signal = is_below_lo if self.inverted else is_above_hi
        inactive_signal = is_above_hi if self.inverted else is_below_lo

        if self.state == 'LOW':
            if active_signal:
                self.state = 'PRESSED'
        
        elif self.state == 'PRESSED':
            self.state = 'HIGH'
            
        elif self.state == 'HIGH':
            if inactive_signal:
                self.state = 'RELEASED'
            
        elif self.state == 'RELEASED':
            self.state = 'LOW'
            
        return self.state
    
class OnePole:
    def __init__(self, alpha=0.2):
        self.alpha = alpha
        self.output = 0

    def update(self, value):
        if self.output is None:
            self.output = value
        else:
            # Formula: y[n] = α * x[n] + (1 - α) * y[n-1]
            self.output = (self.alpha * value) + (1.0 - self.alpha) * self.output
        return self.output
    
    def read(self):
        return self.output

class OneEuroFilter:
    def __init__(self, min_cutoff=1.0, beta=0.01, d_cutoff=1.0):
        self.min_cutoff = min_cutoff
        self.beta = beta
        self.d_cutoff = d_cutoff
        self.x_prev = None
        self.dx_prev = 0
        self.last_time = None

    def _alpha(self, cutoff, dt):
        tau = 1.0 / (2 * math.pi * cutoff)
        return 1.0 / (1.0 + tau / dt)

    def update(self, x):
        now = time.monotonic()
        if self.last_time is None:
            self.last_time, self.x_prev = now, x
            return x

        dt = now - self.last_time
        if dt <= 0: return self.x_prev # Prevent div by zero

        # Filter the derivative (velocity)
        dx = (x - self.x_prev) / dt
        edx = self.dx_prev + self._alpha(self.d_cutoff, dt) * (dx - self.dx_prev)
        
        # Calculate adaptive cutoff
        cutoff = self.min_cutoff + self.beta * abs(edx)
        
        # Filter the signal
        out = self.x_prev + self._alpha(cutoff, dt) * (x - self.x_prev)
        
        # Save state
        self.x_prev, self.dx_prev, self.last_time = out, edx, now
        return out

def scale(input, in_low = 0, in_high = 1, out_low = 0, out_high = 127, curve = 1):
    value = (input - in_low)/(in_high-in_low) # normalize to 0 to 1
    value = value ** curve
    value = value * (out_high-out_low) + out_low
    return value