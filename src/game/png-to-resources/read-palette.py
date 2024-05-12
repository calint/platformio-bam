#!/bin/python3
from PIL import Image
import sys

def print_palette_as_game_resource(filename):
    try:
        with Image.open(filename) as img:
            palette = img.getpalette()
            if palette is None:
                print("Error: The PNG is not paletted.")
                return

            for i in range(0, len(palette), 3):
                red, green, blue = palette[i], palette[i + 1], palette[i + 2]
                rgb565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
                print("0x{:02X}{:02X},".format(rgb565 & 0xFF, (rgb565 >> 8) & 0xFF))

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: read-palette <filename>")
        sys.exit(1)
    print_palette_as_game_resource(sys.argv[1])
