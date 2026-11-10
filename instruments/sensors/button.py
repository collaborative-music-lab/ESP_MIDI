import board, digitalio

class Button:
    """
    Represents one digital button.

    Handles:
    - Digital input setup
    - Edge detection (press / release)
    - Previous state tracking
    """

    def __init__(self, pin):
        self.pin =  getattr(board, f'D{pin}')
        self.button = digitalio.DigitalInOut(self.pin)
        self.button.direction = digitalio.Direction.INPUT
        self.button.pull = digitalio.Pull.UP

        self.prev_state = self.button.value

    def read(self):
        """
        Returns:
        - "pressed"
        - "released"
        - None
        """
        current = self.button.value

        if current != self.prev_state:
            self.prev_state = current
            return "pressed" if not current else "released"

        return False