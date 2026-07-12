# Event Handler
import time

class EventHandler:
    """
    Monotonic event scheduler
    
    Events are stored sorted by timestamp. Call update() continuously
    in the main loop — all overdue events are fired in a single pass.
    """

    def __init__(self):
        self._events = []       # list of (timestamp_ns, event_id, callback)
        self._next_id = 0
        self._origin_ns = time.monotonic_ns()

    # ------------------------------------------------------------------ #
    # Time access                                                          #
    # ------------------------------------------------------------------ #

    @property
    def time_ns(self):
        """Clock time in nanoseconds since instantiation."""
        return time.monotonic_ns() - self._origin_ns

    @property
    def time_ms(self):
        """Clock time in milliseconds."""
        return self.time_ns // 1_000_000

    # ------------------------------------------------------------------ #
    # Scheduling                                                           #
    # ------------------------------------------------------------------ #

    def schedule(self, delay_ns, callback):
        """Fire callback after delay_ns nanoseconds. Returns event ID."""
        return self.schedule_at(self.time_ns + delay_ns, callback)

    def schedule_ms(self, delay_ms, callback):
        """Fire callback after delay_ms milliseconds. Returns event ID."""
        return self.schedule(delay_ms * 1_000_000, callback)

    def schedule_at(self, timestamp_ns, callback):
        """
        Fire callback at an absolute clock timestamp (nanoseconds).
        Returns an event ID for later cancellation.
        """
        eid = self._next_id
        self._next_id += 1
        entry = (timestamp_ns, eid, callback)

        # Linear insertion sort — fast and allocation-light for small lists
        pos = len(self._events)
        for i in range(len(self._events)):
            if timestamp_ns < self._events[i][0]:
                pos = i
                break
        self._events.insert(pos, entry)
        return eid

    def cancel(self, event_id):
        """Cancel a scheduled event by ID. Returns True if found."""
        for i, e in enumerate(self._events):
            if e[1] == event_id:
                del self._events[i]
                return True
        return False

    def clear(self):
        """Cancel all scheduled events."""
        self._events.clear()

    # ------------------------------------------------------------------ #
    # Main loop                                                            #
    # ------------------------------------------------------------------ #

    def update(self):
        """
        Fire all events whose time has arrived.
        Call this on every iteration of your main loop.
        """
        now = self.time_ns
        while self._events and self._events[0][0] <= now:
            _, _, callback = self._events.pop(0)
            callback()
