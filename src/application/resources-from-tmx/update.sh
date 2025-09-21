#!/bin/bash
set -e
cd $(dirname "$0")

./parse-tmx.py map.tmx 30 0xfffffff > ../resources/tile_map.hpp
./parse-tmx-flags.py map.tmx 30 > ../resources/tile_map_flags.hpp