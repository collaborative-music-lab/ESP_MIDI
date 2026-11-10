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