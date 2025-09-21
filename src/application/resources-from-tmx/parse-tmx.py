#!/bin/python3
import xml.etree.ElementTree as ET
import sys

if len(sys.argv) < 4:
    print(
        "Usage: ./parse-tmx.py <tmx file> <output newline every n'th line> <bit mask for tile id>"
    )
    sys.exit(1)

input_file = sys.argv[1]
newline_sep_every_nth_line = int(sys.argv[2])
bit_mask = int(sys.argv[3], 16)

root = ET.parse(input_file).getroot()
width = int(root.attrib["width"])
height = int(root.attrib["height"])
data = root.find("layer").find("data").text.strip().split(",")

for row in range(height):
    row_data = []
    for col in range(width):
        tile_id = data[row * width + col]
        row_data.append((int(tile_id) - 1) & bit_mask)
    row_str = "{" + ",".join(map(str, row_data)) + "},"
    print(row_str)
    if newline_sep_every_nth_line and (row + 1) % newline_sep_every_nth_line == 0:
        print("")
