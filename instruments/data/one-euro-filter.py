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