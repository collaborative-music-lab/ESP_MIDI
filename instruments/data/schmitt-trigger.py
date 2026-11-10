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