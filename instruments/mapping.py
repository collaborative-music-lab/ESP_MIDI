import midi

mappings = [
    ["cc", 10, 'bob-filter', 1, 'CUTOFF', 10, 90],
    ["cc", 11, 'bob-filter', 1, 'Q', 0, 80]
    ]

def send_mapping():
    for i in range(127):
        midi.send_mapping("cc", i, 'none',0, 'NONE')
    for map in mappings:
        midi.send_mapping( *map )