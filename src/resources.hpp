#pragma once

#include <cstdint>

#include "application/defs.hpp"

// palette used when rendering tile images
// converts uint8_t to uint16_t rgb 565 (red being the highest bits)
// note: lower and higher byte swapped
static uint16_t constexpr palette_tiles[256]{
#include "application/resources/palette_tiles.hpp"
};

// palette used when rendering sprites
static uint16_t constexpr palette_sprites[256]{
#include "application/resources/palette_sprites.hpp"
};

// images used by tile map
static uint8_t constexpr tile_imgs[tile_img_count][tile_width * tile_height]{
#include "application/resources/tile_imgs.hpp"
};

// initial tile map
static tile_img_ix tile_map[tile_map_height][tile_map_width]{
#include "application/resources/tile_map.hpp"
};

// initial tile map cell flags
static uint8_t tile_map_flags[tile_map_height][tile_map_width]{
#include "application/resources/tile_map_flags.hpp"
};

// images used by sprites
static uint8_t constexpr sprite_imgs[sprite_img_count]
                                    [sprite_width * sprite_height]{
#include "application/resources/sprite_imgs.hpp"
                                    };
