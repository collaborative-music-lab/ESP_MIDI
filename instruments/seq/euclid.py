class EuclideanSequencer:
    def __init__(self, length=8, hits=3, rotate = 0):
        self.length = length
        self.hits = hits
        self.rotate = rotate
        self.sequence = [0]
        self.make_sequence()
        self.index = 0
        
    def set_length(self, length):
        self.length = length
        self.make_sequence()
        
    def set_hits(self, hits):
        if hits == self.hits: return
        self.hits = hits
        self.make_sequence()
        
    def set_rotate(self, rotate):
        self.rotate = rotate
        self.make_sequence()
        
    def update(self, index = None):
        if index == None:
            self.index += 1
            index = self.index
        self.index = index % len(self.sequence)
        return self.sequence[self.index]
    
    def get(self, index = None):
        return self.update(index)
        
    def make_sequence(self, hits=None, length=None):
        length = self.length if length is None else length
        hits = self.hits if hits is None else hits
        if hits > length: hits = length
        
        self.sequence = [0]*length
        self.original_sequence = [0]*length
        
        bucket = length
        for i in range(length):
            if bucket >= length:
                self.sequence[i] = 1
                self.original_sequence[i] = 1
                bucket -= length
            bucket += hits
            
        self.sequence.reverse()
        if self.rotate > 0 : self.rotate_sequence()
        self.length = length
        self.hits = hits
            
    def rotate_sequence(self, rotation=None):
        if rotation is None: rotation = self.rotate
        n = len(self.sequence)
        if n == 0:
            return []
        
        shift = rotation % n
        shift = n-shift
        self.rotate = shift
        
        self.sequence =  self.original_sequence[shift:] + self.original_sequence[:shift]
        
