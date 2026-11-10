def scale(input, in_low = 0, in_high = 1, out_low = 0, out_high = 127, curve = 1):
    value = (input - in_low)/(in_high-in_low) # normalize to 0 to 1
    value = value ** curve
    value = value * (out_high-out_low) + out_low
    return value