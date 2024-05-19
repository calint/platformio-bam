#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class ben final : public game_object {
  static constexpr uint8_t const *spr_img_still = &sprite_imgs[32][0];
  static constexpr uint8_t const *spr_img_walk = &sprite_imgs[33][0];
  static constexpr uint8_t const *spr_img_jump = &sprite_imgs[34][0];
  static constexpr clk::time anim_ms = 500;
  static constexpr float walk_speed = 4;
  clk::time anim_last_frame_ms = 0;
  uint8_t moving_direction = 0; // 0: still  1: right  2: left

public:
  ben() : game_object{ben_cls} {
    spr = sprites.alloc();
    spr->obj = this;
    spr->img = spr_img_still;
    spr->flip = 0;
    moving_direction = 1;
  }

  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    switch (moving_direction) {
    case 1: // right
      if (x < display_width - sprite_width) {
        if (clk.ms > anim_last_frame_ms + anim_ms) {
          spr->img = (spr->img == spr_img_still) ? spr_img_walk : spr_img_still;
          spr->flip = 0;
          x += walk_speed;
          anim_last_frame_ms = clk.ms;
        }
      } else {
        moving_direction = 2;
        spr->img = spr_img_still;
        spr->flip = 1;
      }
      break;
    case 2: // left
      if (x >= 0) {
        if (clk.ms > anim_last_frame_ms + anim_ms) {
          spr->img = (spr->img == spr_img_still) ? spr_img_walk : spr_img_still;
          spr->flip = 1;
          x -= walk_speed;
          anim_last_frame_ms = clk.ms;
        }
      } else {
        moving_direction = 1;
        spr->img = spr_img_still;
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
