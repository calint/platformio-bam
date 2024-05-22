#pragma once
// constants used by engine, game objects and 'main.hpp'

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

#include <cstdint>

// requested display mode
// 0: portrait, 1: landscape
static constexpr uint8_t display_orientation = 0;

// engine time step in milliseconds
// 0 to calculate dt based on previous frame time
static constexpr int clk_locked_dt_ms = BAM_TIME_STEP_MS;

// update rate of fps calculation
// 0 to update fps every frame and make no output
static constexpr int clk_fps_update_ms = 2000;

// number of sprite images
static constexpr int sprite_img_count = 256;
// defined in 'resources/sprite_imgs.hpp'

// type used to index in the 'sprite_imgs' array
using sprite_img_ix = uint8_t;

// sprite dimensions
static constexpr int sprite_width = 16;
static constexpr int sprite_height = 16;
// note. when changing dimensions update 'png-to-resources/update.sh'

// number of layers of sprites
// 0: ground, 1: air, 2: overlay
static constexpr int sprite_layers = 3;

// number of tile images
static constexpr int tile_img_count = 256;
// defined in 'resources/tile_imgs.hpp'

// type used to index in the 'tile_imgs' array from 'tile_map'
using tile_img_ix = uint8_t;

// tile dimensions
static constexpr int tile_width = 16;
static constexpr int tile_height = 16;
// note. when changing dimensions update 'png-to-resources/update.sh'

//
// example configuration for more sprites and tile_imgs
//
// static constexpr int sprite_img_count = 512;
// using sprite_img_ix = uint16_t;
// static constexpr int tile_img_count = 512;
// using tile_img_ix = uint16_t;

// tile map dimension
static constexpr int tile_map_width = 17;
static constexpr int tile_map_height = 90;
// defined in 'resources/tile_map.hpp'

// type used to index a 'sprite'
// note. 8-bit for 'collision_map' to fit in a contiguous block on heap
using sprite_ix = uint8_t;

// sprites available for allocation using 'sprites'
// note. maximum is one less than limit of type 'sprite_ix' due to the reserved
//       sprite index (maximum limit) used at collision detection
static constexpr int sprite_count = 255;

// objects available for allocation using 'objects'
static constexpr int objects_count = 255;

// used by 'engine.hpp' as seed for random numbers
static constexpr int random_seed = 0;

// enumeration of game object classes
// defined in 'objects/*'
// note. not 'enum class' because of code ergonomics
enum object_class : uint8_t {
  hero_cls,
  bullet_cls,
  dummy_cls,
  fragment_cls,
  ship1_cls,
  ship2_cls,
  upgrade_cls,
  upgrade_picked_cls,
  ufo2_cls,
  ben_cls,
  ned_cls
};

// size that fits any instance of game object
static constexpr int object_instance_max_size_B = 128;

// define the size of collision bits
using collision_bits = uint16_t;

// collision bits
static constexpr collision_bits cb_none = 0;
static constexpr collision_bits cb_hero = 1 << 0;
static constexpr collision_bits cb_hero_bullet = 1 << 1;
static constexpr collision_bits cb_fragment = 1 << 2;
static constexpr collision_bits cb_enemy = 1 << 3;
static constexpr collision_bits cb_enemy_bullet = 1 << 4;
static constexpr collision_bits cb_upgrade = 1 << 5;
