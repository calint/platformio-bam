# game code

## intention
* developing a toy game using platform-independent engine

## table of contents
* `main.hpp` setup initial game state, callbacks from engine, game logic
* `objects/*` game objects
* `game_state.hpp` used by game objects and `main.hpp`
* `resources/*` partial files defining tiles, sprites, palettes and tile map
* `defs.hpp` constants used by engine, game objects and `main.hpp`
* `animator.hpp` support class that implements sprite animation

## appendix
* `resources-from-png/update.sh` tool for extracting tile and sprite images resources from png files
* `resources-from-tmx/update.sh` tool for extracting tile map resource

# overview

## main.hpp
### function `main_setup`
* initiates the game by creating initial objects and sets tile map position and velocity
### function `main_on_touch`
* handles user interaction with touch screen
### function `main_on_frame_completed`
* implements game logic after a frame has been rendered and objects updated

## objects/*
* see `objects/README.md`

## game_state.hpp
* included by game objects that access game state
* included by `main.hpp` after the game objects
* provides a way for game objects to share data with `main.hpp`
* used in `main_on_frame_completed` to solve circular reference issues

## resources/*
* `tile_map.hpp` size defined in `defs.hpp` and generated from `tmx` file by tool `resources-from-tmx/update.sh`
* `tile_imgs.hpp`, `sprite_imgs.hpp` and `palette_*.hpp` generated from `png` files by tool `resources-from-png/update.sh`
* 256 tile and 256 sprite images, 16 x 16 pixels, are default settings in `defs.hpp`
* sprite and tile images are constant data stored in program memory
* separate palettes for tiles and sprites

## defs.hpp
### `enum object_class`
* each game object class has an entry named with prefix `cls_`
### `object_instance_max_size_B`
* maximum size of any game object instance
* set to 128B but should be maximum game object instance size rounded upwards to nearest power of 2 number
### `collision_bits`
* constants used by game objects to define collision bits and mask

## limitations
* due to target device not being able to allocate large (>150KB) chunks of contiguous memory some limitations are imposed
* concurrent sprites limited to 255 due to collision map having to be 8-bit to fit the screen pixels in a contiguous block of memory
* concurrent objects limited to 255 being a natural sizing considering sprites
* limits defined in `defs.hpp`
