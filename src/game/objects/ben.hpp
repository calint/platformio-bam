#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

struct animation_frame {
  constexpr animation_frame(uint8_t const *_sprite_image, int _duration_ms,
                            float _displace_x, float _displace_y)
      : sprite_image{_sprite_image}, duration_ms{_duration_ms},
        displace_x{_displace_x}, displace_y{_displace_y} {}

  uint8_t const *sprite_image = nullptr;
  int duration_ms = 0;
  float displace_x = 0;
  float displace_y = 0;
};

static constexpr animation_frame ben_animation_walk[]{
    {&sprite_imgs[32][0], 500, 4, 0},
    {&sprite_imgs[33][0], 500, 4, 0},
};

class animator final {
  animation_frame const *frames_ = nullptr;
  int frames_count_ = 0;
  clk::time next_frame_ms_ = 0;
  int current_frame_num_ = 0;

public:
  auto init(animation_frame const *frames, int frames_count) -> void {
    frames_ = frames;
    frames_count_ = frames_count;
    reset();
  }

  auto reset() -> void {
    current_frame_num_ = 0;
    next_frame_ms_ = clk.ms + frames_[0].duration_ms;
  }

  auto update() -> bool {
    if (clk.ms < next_frame_ms_) {
      return false;
    }
    ++current_frame_num_;
    if (current_frame_num_ >= frames_count_) {
      current_frame_num_ = 0;
    }
    next_frame_ms_ = clk.ms + frames_[current_frame_num_].duration_ms;
    return true;
  }

  auto current_sprite_image() -> uint8_t const * {
    return frames_[current_frame_num_].sprite_image;
  }

  auto current_displace_x() -> float {
    return frames_[current_frame_num_].displace_x;
  }

  auto current_displace_y() -> float {
    return frames_[current_frame_num_].displace_y;
  }
};

class ben final : public game_object {
  animator animator_{};
  uint8_t moving_direction = 0; // 0: still  1: right  2: left

public:
  ben() : game_object{ben_cls} {
    animator_.init(ben_animation_walk,
                   sizeof(ben_animation_walk) / sizeof(animation_frame));
    moving_direction = 1;
    spr = sprites.alloc();
    spr->obj = this;
    spr->img = animator_.current_sprite_image();
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
          spr->img = animator_.current_sprite_image();
          spr->flip = 0;
          x += animator_.current_displace_x();
          y += animator_.current_displace_y();
        }
      } else {
        moving_direction = 2;
        animator_.reset();
        spr->img = animator_.current_sprite_image();
        spr->flip = 1;
      }
      break;
    case 2: // left
      if (x >= 0) {
        if (animator_.update()) {
          spr->img = animator_.current_sprite_image();
          spr->flip = 1;
          x -= animator_.current_displace_x();
          y += animator_.current_displace_y();
        }
      } else {
        moving_direction = 1;
        animator_.reset();
        spr->img = animator_.current_sprite_image();
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
