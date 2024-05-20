#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"
//
#include "../animator.hpp"

static constexpr animation_frame ned_animation_walk[]{
    {&sprite_imgs[48][0], 150, 2, 0},
    {&sprite_imgs[49][0], 150, 2, 0},
    {&sprite_imgs[50][0], 150, 2, 0},
    {&sprite_imgs[51][0], 150, 2, 0},
};

class ned final : public game_object {
  animator animator_{};

public:
  int8_t moving_direction = 0; // 0: still  1: right  -1s: left

  ned() : game_object{ned_cls} {
    animator_.init(ned_animation_walk,
                   sizeof(ned_animation_walk) / sizeof(animation_frame), true);
    moving_direction = 0;
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
          x += moving_direction * animator_.displace_x();
          y += animator_.displace_y();
        }
      } else {
        moving_direction = -1;
        animator_.reset();
        spr->img = animator_.sprite_image();
        spr->flip = 1;
      }
      break;
    case -1: // left
      if (x >= 0) {
        if (animator_.update()) {
          spr->img = animator_.sprite_image();
          spr->flip = 1;
          x += moving_direction * animator_.displace_x();
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
