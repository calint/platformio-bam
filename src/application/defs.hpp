#pragma once
// constants used by engine, game objects and 'application.hpp'

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

#include <cstdint>

// requested display mode
// 0: portrait, 1: landscape
static uint8_t constexpr display_orientation = 0;

// engine time step in milliseconds
// 0 to calculate dt based on previous frame time
static int constexpr clk_locked_dt_ms = BAM_TIME_STEP_MS;

// update rate of fps calculation
// 0 to update fps every frame and make no output
static int constexpr clk_fps_update_ms = 2000;

// number of sprite images
static int constexpr sprite_img_count = 256;
// defined in 'resources/sprite_imgs.hpp'

// type used to index in the 'sprite_imgs' array
using sprite_img_ix = uint8_t;

// sprite dimensions
static int constexpr sprite_width = 16;
static int constexpr sprite_height = 16;
// note: when changing dimensions update 'png-to-resources/update.sh'

// number of layers of sprites
// 0: ground, 1: air, 2: overlay
static int constexpr sprite_layer_count = 3;

// number of tile images
static int constexpr tile_img_count = 256;
// defined in 'resources/tile_imgs.hpp'

// type used to index in the 'tile_imgs' array from 'tile_map'
using tile_img_ix = uint8_t;

// tile dimensions
static int constexpr tile_width = 16;
static int constexpr tile_height = 16;
// note: when changing dimensions update 'png-to-resources/update.sh'

//
// example configuration for more sprites and tile_imgs
//
// static int constexpr sprite_img_count = 512;
// using sprite_img_ix = uint16_t;
// static int constexpr tile_img_count = 512;
// using tile_img_ix = uint16_t;

// tile map dimension
static int constexpr tile_map_width = 17;
static int constexpr tile_map_height = 90;
// defined in 'resources/tile_map.hpp'

// type used to index a 'sprite'
// note: 8-bit for 'collision_map' to fit in a contiguous block on heap
using sprite_ix = uint8_t;

// sprites available for allocation using 'sprites'
// note: maximum is one less than limit of type 'sprite_ix' due to the reserved
//       sprite index (maximum limit) used at collision detection
static int constexpr sprite_count = 255;

// objects available for allocation using 'objects'
static int constexpr object_count = 255;

// used by 'engine.hpp' as seed for random numbers
static int constexpr random_seed = 0;

// enumeration of game object classes
// defined in 'objects/*'
// note: not 'enum class' because of code ergonomics
enum object_class : uint8_t {
    cls_hero,
    cls_bullet,
    cls_dummy,
    cls_fragment,
    cls_ship1,
    cls_ship2,
    cls_upgrade,
    cls_upgrade_picked,
    cls_ufo2,
    cls_ben,
    cls_ned
};

// size that fits any instance of game object
static int constexpr object_instance_max_size_B = 128;

// define the size of collision bits
using collision_bits = uint16_t;

// collision bits
static collision_bits constexpr cb_none = 0;
static collision_bits constexpr cb_hero = 1 << 0;
static collision_bits constexpr cb_hero_bullet = 1 << 1;
static collision_bits constexpr cb_fragment = 1 << 2;
static collision_bits constexpr cb_enemy = 1 << 3;
static collision_bits constexpr cb_enemy_bullet = 1 << 4;
static collision_bits constexpr cb_upgrade = 1 << 5;
