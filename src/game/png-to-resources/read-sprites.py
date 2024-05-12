#!/bin/python3
from PIL import Image
import sys


def print_sprites_as_game_resource(spr_width: int, spr_height: int, filename: str):
    try:
        with Image.open(filename) as img:
            if img.mode != "P":
                print("Error: The PNG is not paletted.")
                return

            width, height = img.size
            ix = 0
            for row in range(0, height, spr_height):
                for column in range(0, width, spr_width):
                    print("{ //", ix)
                    ix += 1
                    for y in range(row, row + spr_height):
                        for x in range(column, column + spr_width):
                            pixel = img.getpixel((x, y))
                            print(f"0x{pixel:02X},", end="")
                        print()
                    print("},")

    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("usage: read-sprites <width> <height> <filename>")
        sys.exit(1)
    print_sprites_as_game_resource(int(sys.argv[1]), int(sys.argv[2]), sys.argv[3])
