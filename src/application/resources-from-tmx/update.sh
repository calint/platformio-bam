#!/bin/bash
set -e
cd $(dirname "$0")

SIZE=${1:-30}

./parse-tmx.py map.tmx $SIZE 0xfffffff >../resources/tile_map.hpp
./parse-tmx-flags.py map.tmx $SIZE >../resources/tile_map_flags.hpp

