#!/bin/python3
import xml.etree.ElementTree as ET
import sys

if len(sys.argv) < 3:
    print("Usage: ./parse-tmx.py <tmx file> <output newline every n'th line>")
    sys.exit(1)

input_file = sys.argv[1]
newline_sep_every_nth_line = int(sys.argv[2])

tree = ET.parse(input_file)
root = tree.getroot()
layer = root.find('layer')
data = layer.find('data').text.strip().split(',')

width = int(root.attrib['width'])
height = int(root.attrib['height'])

for row in range(height):
    row_data = []
    for col in range(width):
        tile_id = data[row * width + col]
        row_data.append(int(tile_id) - 1)
    row_str = '{' + ','.join(map(str, row_data)) + '},'
    print(row_str)
    if newline_sep_every_nth_line:
        if (row + 1) % newline_sep_every_nth_line == 0:
            print('')
