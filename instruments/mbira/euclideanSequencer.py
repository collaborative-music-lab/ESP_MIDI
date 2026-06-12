class EuclideanSequencer:
    def __init__(self, beats=8, hits=0):
        self.beats = beats
        self.hits = hits
        self.rotate = 0
        self.sequence = [0]*self.beats
        self.original_sequence = [0]*self.beats
        
    def make_sequence(self, hits=None, beats=None):
        beats = self.beats if beats is None else beats
        hits = self.hits if hits is None else hits
        if hits > beats: hits = beats
        
        if beats == self.beats and hits == self.hits: return
        
        self.sequence = [0]*beats
        self.original_sequence = [0]*beats
        
        
        bucket = beats
        for i in range(beats):
            if bucket >= beats:
                self.sequence[i] = 1
                self.original_sequence[i] = 1
                bucket -= beats
            bucket += hits
            
        if self.rotate > 0 : self.rotate_sequence()
        self.beats = beats
        self.hits = hits
        
    # dunder methed - euclid[index] calls getitem
    def __getitem__(self, index):
        constrained_index = index % self.beats if index >= 0  else 0
        return self.sequence[constrained_index]

    def __len__(self):
        return len(self.sequence)
            
    def rotate_sequence(self, rotation=None):
        if rotation is None: rotation = self.rotate
        n = len(self.sequence)
        if n == 0:
            return []
        
        shift = rotation % n
        shift = n-shift
        self.rotate = shift
        
        self.sequence =  self.original_sequence[shift:] + self.original_sequence[:shift]
        
