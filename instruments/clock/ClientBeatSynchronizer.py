class ClientBeatSynchronizer:
    def __init__(self, tempo_bpm):
        self.tempo_bpm = tempo_bpm
        self.beat_interval_ms = (60_000 / tempo_bpm)
        
        # Synchronization state
        self.local_beat_number = 0
        self.next_beat_time = None
        
        # Latency & drift tracking
        self.estimated_latency = 10  # ESP-NOW baseline
        self.drift_rate = 0  # ms/beat - how much client clock deviates from timekeeper
        self.last_sync_time = None
        
        # Scheduled tempo changes (list of future changes)
        self.tempo_schedule = []  # [(beat_number, new_tempo_bpm), ...]
        
    def on_receive_sync_message(self, message, local_time_ms):
        """
        message = {
            'beat_num': int,                    # current beat on timekeeper
            'scheduled_beat_time': int,         # when beat N+1 should fire (timekeeper's clock)
            'timekeeper_current_time': int,     # timekeeper's current time
            'tempo': int,                       # current tempo bpm
            'tempo_changes': [...]              # scheduled tempo changes ahead
        }
        local_time_ms = system time when message arrived
        """
        
        # ===== Step 1: Estimate latency =====
        message_latency = local_time_ms - message['timekeeper_current_time']
        
        # Low-pass filter (ESP-NOW is stable around 10ms)
        alpha_latency = 0.1  # very smooth since ESP-NOW is consistent
        self.estimated_latency = (alpha_latency * message_latency + 
                                   (1 - alpha_latency) * self.estimated_latency)
        
        # ===== Step 2: Convert timekeeper's beat time to local clock =====
        adjusted_beat_time = message['scheduled_beat_time'] + self.estimated_latency
        
        # ===== Step 3: Calculate drift if we have a previous sync =====
        if self.next_beat_time is not None:
            # How many beats since last sync?
            beats_elapsed = message['beat_num'] - self.local_beat_number
            
            # What time should it be now based on last sync?
            expected_time = self.next_beat_time + (beats_elapsed - 1) * self.beat_interval_ms
            
            # Actual time vs expected = accumulated drift
            accumulated_drift = adjusted_beat_time - expected_time
            
            # Calculate drift per beat (for extrapolation)
            self.drift_rate = accumulated_drift / beats_elapsed
            
            # Correct next beat time with drift compensation
            # Use soft blending to avoid sudden corrections
            alpha_sync = 0.4  # higher than single-beat systems because sync is less frequent
            self.next_beat_time = (alpha_sync * adjusted_beat_time + 
                                   (1 - alpha_sync) * expected_time)
        else:
            # First sync message
            self.next_beat_time = adjusted_beat_time
            self.drift_rate = 0
        
        # ===== Step 4: Update state =====
        self.local_beat_number = message['beat_num']
        self.last_sync_time = local_time_ms
        self.tempo_bpm = message['tempo']
        self.beat_interval_ms = (60_000 / self.tempo_bpm)
        
        # ===== Step 5: Store scheduled tempo changes =====
        if 'tempo_changes' in message:
            self.tempo_schedule = message['tempo_changes']
        
        return {
            'estimated_latency_ms': self.estimated_latency,
            'drift_rate_ms_per_beat': self.drift_rate,
            'next_beat_time': self.next_beat_time,
            'sync_quality_stable': abs(self.drift_rate) < 0.5  # drift < 0.5ms/beat is good
        }
    
    def get_time_to_next_beat(self, current_local_time):
        """
        Returns time until next beat, accounting for:
        - Drift accumulation over the 4-beat sync interval
        - Scheduled tempo changes
        """
        if self.next_beat_time is None:
            return None
        
        time_to_beat = self.next_beat_time - current_local_time
        
        # ===== Handle beats in past =====
        if time_to_beat < 0:
            beats_passed = int((-time_to_beat) / self.beat_interval_ms) + 1
            self.next_beat_time += beats_passed * self.beat_interval_ms
            time_to_beat = self.next_beat_time - current_local_time
        
        return time_to_beat
    
    def apply_scheduled_tempo_change(self, beat_number):
        """
        Check if tempo should change at this beat and apply it.
        Call this in your main beat-firing logic.
        """
        for change_beat, new_tempo in self.tempo_schedule:
            if beat_number == change_beat:
                old_tempo = self.tempo_bpm
                self.tempo_bpm = new_tempo
                self.beat_interval_ms = (60_000 / self.tempo_bpm)
                
                print(f"Tempo change: {old_tempo} → {new_tempo} bpm at beat {beat_number}")
                return True
        
        return False
    
    def fire_beat_if_ready(self, current_local_time, tolerance_ms=15):
        """
        tolerance_ms: window before scheduled beat time to fire
        (accounts for reaction time, scheduling jitter)
        """
        time_to_beat = self.get_time_to_next_beat(current_local_time)
        
        if time_to_beat is not None and -5 <= time_to_beat <= tolerance_ms:
            # Beat should fire
            self.local_beat_number += 1
            
            # Apply any scheduled tempo changes
            self.apply_scheduled_tempo_change(self.local_beat_number)
            
            # Move to next beat time
            self.next_beat_time += self.beat_interval_ms
            
            # Account for drift prediction: next beat will drift further
            # (This is a subtle correction: we predict where drift will accumulate)
            beats_until_next_sync = 4 - (self.local_beat_number % 4)
            drift_prediction = self.drift_rate * beats_until_next_sync
            self.next_beat_time += drift_prediction * 0.1  # gentle prediction
            
            return True
        
        return False