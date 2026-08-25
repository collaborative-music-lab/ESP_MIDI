import board

# Print all available pin names
print("Available pins:")
for attr in dir(board):
    if not attr.startswith('_'):
        print(f"  board.{attr}")