#!/bin/bash
set -e
cd $(dirname "$0")

./parse-tmx.py map.tmx 30 > ../resources/tile_map.hpp