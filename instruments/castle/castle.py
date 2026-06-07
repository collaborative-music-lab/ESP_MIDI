class Castle:
    def __init__(self, note=0):
        self.note = note
        self.subdivide = 4
        self.start_beat = 0
        self.down = False
        
    def get(self, beat):
        if not self.down: return 0
        
        beat = beat - self.start_beat
        if beat % self.subdivide >= self.subdivide /2:
            return 0
        
        return self.note
    
    def press(self, value, beat=0):
        if value == "released": self.down = False
        elif value == "pressed":
            self.start_beat = beat
            self.down = True
        
    