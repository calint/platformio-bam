#pragma once

// implements sprite animation

struct animation_frame {
  constexpr animation_frame(uint8_t const *_sprite_image, int _duration_ms,
                            float _displace_x, float _displace_y)
      : sprite_image{_sprite_image}, duration_ms{_duration_ms},
        displace_x{_displace_x}, displace_y{_displace_y} {}

  uint8_t const *sprite_image = nullptr;
  int duration_ms = 0;
  // displacement of object due to this frame
  float displace_x = 0;
  float displace_y = 0;
};

class animator final {
  animation_frame const *frames_ = nullptr;
  clk::time next_frame_ms_ = 0;
  uint8_t frames_count_ = 0;
  int8_t current_frame_ix_ = 0;
  int8_t frame_ix_dir_ = 1;
  bool back_forth_ = false;

public:
  auto init(animation_frame const *frames, uint8_t frames_count,
            bool back_forth) -> void {
    frames_ = frames;
    frames_count_ = frames_count;
    back_forth_ = back_forth;
    reset();
  }

  auto reset() -> void {
    current_frame_ix_ = 0;
    next_frame_ms_ = clk.ms + frames_[0].duration_ms;
  }

  auto update() -> bool {
    if (clk.ms < next_frame_ms_) {
      return false;
    }
    current_frame_ix_ += frame_ix_dir_;
    if (current_frame_ix_ >= frames_count_) {
      if (back_forth_) {
        frame_ix_dir_ = -1;
        current_frame_ix_ -= 2;
      } else {
        current_frame_ix_ = 0;
      }
    } else if (current_frame_ix_ < 0) {
      if (back_forth_) {
        frame_ix_dir_ = 1;
        current_frame_ix_ += 2;
      } else {
        current_frame_ix_ = 0;
      }
    }
    next_frame_ms_ = clk.ms + frames_[current_frame_ix_].duration_ms;
    return true;
  }

  auto sprite_image() -> uint8_t const * {
    return frames_[current_frame_ix_].sprite_image;
  }

  auto displace_x() -> float { return frames_[current_frame_ix_].displace_x; }

  auto displace_y() -> float { return frames_[current_frame_ix_].displace_y; }
};