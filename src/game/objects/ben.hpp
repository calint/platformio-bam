#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"
//
#include "../animator.hpp"

static constexpr animation_frame ben_animation_walk[]{
    {&sprite_imgs[32][0], 300, 2, 0},
    {&sprite_imgs[33][0], 300, 2, 0},
    {&sprite_imgs[34][0], 300, 2, 0},
    {&sprite_imgs[35][0], 300, 2, 0},
};

class ben final : public game_object {
  animator animator_{};
  uint8_t moving_direction = 0; // 0: still  1: right  2: left

public:
  ben() : game_object{ben_cls} {
    animator_.init(ben_animation_walk,
                   sizeof(ben_animation_walk) / sizeof(animation_frame), false);
    moving_direction = 1;
    spr = sprites.alloc();
    spr->obj = this;
    spr->img = animator_.sprite_image();
    spr->flip = 0;
  }

  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    switch (moving_direction) {
    case 1: // right
      if (x <= display_width - sprite_width) {
        if (animator_.update()) {
          spr->img = animator_.sprite_image();
          spr->flip = 0;
          x += animator_.displace_x();
          y += animator_.displace_y();
        }
      } else {
        moving_direction = 2;
        animator_.reset();
        spr->img = animator_.sprite_image();
        spr->flip = 1;
      }
      break;
    case 2: // left
      if (x >= 0) {
        if (animator_.update()) {
          spr->img = animator_.sprite_image();
          spr->flip = 1;
          x -= animator_.displace_x();
          y += animator_.displace_y();
        }
      } else {
        moving_direction = 1;
        animator_.reset();
        spr->img = animator_.sprite_image();
        spr->flip = 0;
      }
      break;
    default: // other
      break;
    }
    return false;
  }

  // called before rendering the sprites
  auto pre_render() -> void override {
    // place sprite in background coordinate system
    spr->scr_x = int16_t(x - tile_map_x);
    spr->scr_y = int16_t(y - tile_map_y);
  }
};
