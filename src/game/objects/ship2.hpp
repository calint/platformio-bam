#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class ship2 final : public game_object {
  // animation definition
  static const sprite_img_ix animation_frames[];
  static const int animation_frames_len;
  static const int animation_rate_ms;
  // animation state
  clk::time animation_frame_ms = 0;
  uint8_t animation_frames_ix = 0;

public:
  ship2() : game_object{ship2_cls} {
    col_bits = cb_hero;
    col_mask = cb_enemy_bullet;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[6];
    spr->layer = 1;
    spr->flip = 0;

    animation_frame_ms = clk.ms;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }

    if (dy > 0) {
      spr->flip = 2; // vertical
    }

    if (y > display_height) {
      return true;
    }

    // animation logic
    // note. approximation that does not skip frames and displays a frame at
    // least 'animation_rate_ms'
    const clk::time ms_since_last_update = clk.ms - animation_frame_ms;
    if (ms_since_last_update >= animation_rate_ms) {
      animation_frame_ms = clk.ms;
      animation_frames_ix++;
      if (animation_frames_ix == animation_frames_len) {
        animation_frames_ix = 0;
      }
      spr->img = sprite_imgs[animation_frames[animation_frames_ix]];
    }

    return false;
  }

  void on_death_by_collision() override {
    upgrade *upg = new (objects.allocate_instance()) upgrade{};
    upg->x = x;
    upg->y = y;
    upg->dy = 30;
    upg->ddy = 20;
  }
};

const sprite_img_ix ship2::animation_frames[] = {6, 7};
const int ship2::animation_frames_len =
    sizeof(ship2::animation_frames) / sizeof(sprite_img_ix);
const int ship2::animation_rate_ms = 500;
