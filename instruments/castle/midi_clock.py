# midi_clock.py


class MidiClock:
    """
    MIDI timing clock source (24 PPQN).

    Schedules each tick from the *ideal* previous timestamp rather than
    the actual firing time. This prevents timing errors from accumulating
    over long sessions.
    """

    PPQN = 24  # MIDI standard: 24 pulses per quarter note

    def __init__(self, clock, send_fn, bpm=120):
        """
        Args:
            clock:   Shared Clock instance
            send_fn: Callable — sends one MIDI timing clock message
            bpm:     Initial tempo in beats per minute
        """
        self._clock = clock
        self._send = send_fn
        self._bpm = bpm
        self._interval_ns = self._calc_interval(bpm)
        self._running = False
        self._event_id = None
        self._next_ns = 0
        self.pulse_count = 0        # total pulses since start()
        # Cache the bound method to avoid creating a new object each tick
        self._tick_ref = self._tick

    # ------------------------------------------------------------------ #
    # Properties                                                           #
    # ------------------------------------------------------------------ #

    @property
    def bpm(self):
        return self._bpm

    @bpm.setter
    def bpm(self, value):
        """Change tempo — takes effect on the next scheduled tick."""
        self._bpm = value
        self._interval_ns = self._calc_interval(value)

    @property
    def beat(self):
        """Current beat number (0-indexed) since start()."""
        return self.pulse_count // self.PPQN

    @property
    def pulse_in_beat(self):
        """Pulse position within the current beat (0 – 23)."""
        return self.pulse_count % self.PPQN

    @property
    def is_on_beat(self):
        """True on pulse 0 of each beat."""
        return self.pulse_in_beat == 0

    # ------------------------------------------------------------------ #
    # Transport                                                            #
    # ------------------------------------------------------------------ #

    def start(self):
        """Start sending MIDI clock pulses."""
        if self._running:
            return
        self._running = True
        self.pulse_count = 0
        self._next_ns = self._clock.time_ns   # first tick fires immediately
        self._event_id = self._clock.schedule_at(self._next_ns, self._tick_ref)

    def stop(self):
        """Stop sending MIDI clock pulses."""
        if not self._running:
            return
        self._running = False
        if self._event_id is not None:
            self._clock.cancel(self._event_id)
            self._event_id = None

    def reset(self):
        """Stop and reset pulse count to zero."""
        self.stop()
        self.pulse_count = 0

    # ------------------------------------------------------------------ #
    # Internal                                                             #
    # ------------------------------------------------------------------ #

    @staticmethod
    def _calc_interval(bpm):
        """Nanoseconds between pulses at given BPM."""
        return int(60_000_000_000 / (bpm * MidiClock.PPQN))

    def _tick(self):
        if not self._running:
            return
        # Send first — minimize send latency from the tick point
        self._send()
        self.pulse_count += 1
        # *** Advance from the IDEAL time, not time_ns ***
        # This is what prevents drift accumulation
        self._next_ns += self._interval_ns
        self._event_id = self._clock.schedule_at(self._next_ns, self._tick_ref)