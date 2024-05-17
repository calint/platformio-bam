#!/bin/python3
import xml.etree.ElementTree as ET
import sys

if len(sys.argv) < 2:
    print("Usage: python update-resources.py <separator every n'th line>")
    sys.exit(1)

input_file = "map.tmx"
n = int(sys.argv[1])

tree = ET.parse(input_file)
root = tree.getroot()

width = int(root.attrib['width'])
height = int(root.attrib['height'])

layer = root.find('layer')
data = layer.find('data').text.strip().split(',')

c_array = []
for row in range(height):
    row_data = []
    for col in range(width):
        tile_id = data[row * width + col]
        row_data.append(int(tile_id)-1)
    row_str = '{' + ','.join(map(str, row_data)) + '},'
    c_array.append(row_str)
    if (row + 1) % n == 0:
        c_array.append('')

print('\n'.join(c_array))
