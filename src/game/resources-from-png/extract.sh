#!/bin/bash
set -e
cd $(dirname "$0")

SIZE=16

./read-palette.py sprites.png > ../resources/palette_sprites.hpp
./read-sprites.py $SIZE $SIZE sprites.png > ../resources/sprite_imgs.hpp

./read-palette.py tiles.png > ../resources/palette_tiles.hpp
./read-sprites.py $SIZE $SIZE tiles.png > ../resources/tiles.hpp