#pragma once

// implements sprite animation
// note. maximum 127 frames in an animation

struct animation_frame {
  constexpr animation_frame(sprite_img _sprite_img, clk::time _duration_ms,
                            float _displace_x, float _displace_y)
      : spr_img{_sprite_img}, duration_ms{_duration_ms},
        displace_x{_displace_x}, displace_y{_displace_y} {}

  sprite_img spr_img = nullptr;
  clk::time duration_ms = 0;
  // displacement of object due to this frame
  float displace_x = 0;
  float displace_y = 0;
};

class animator final {
  animation_frame const *frames_ = nullptr;
  clk::time next_frame_ms_ = 0;
  int8_t frames_count_ = 0;
  int8_t current_frame_ix_ = 0;
  int8_t frame_ix_dir_ = 0; // forward: 1  backwards: -1
  bool ping_pong_ = false;  // back-and-forth animation

public:
  auto init(animation_frame const *frames, int8_t frames_count,
            bool ping_pong) -> void {
    frames_ = frames;
    frames_count_ = frames_count;
    ping_pong_ = ping_pong;
    reset();
  }

  auto reset() -> void {
    current_frame_ix_ = 0;
    frame_ix_dir_ = 1;
    next_frame_ms_ = clk.ms + frames_[0].duration_ms;
  }

  auto update() -> bool {
    if (clk.ms < next_frame_ms_) {
      return false;
    }
    current_frame_ix_ += frame_ix_dir_;
    if (current_frame_ix_ >= frames_count_) {
      if (ping_pong_) {
        frame_ix_dir_ = -1;
        current_frame_ix_ -= 2;
      } else {
        current_frame_ix_ = 0;
      }
    } else if (current_frame_ix_ < 0) {
      if (ping_pong_) {
        frame_ix_dir_ = 1;
        current_frame_ix_ += 2;
      } else {
        current_frame_ix_ = frames_count_ - 1;
      }
    }
    next_frame_ms_ = clk.ms + frames_[current_frame_ix_].duration_ms;
    return true;
  }

  auto sprite_img() -> sprite_img { return frames_[current_frame_ix_].spr_img; }

  auto displace_x() -> float { return frames_[current_frame_ix_].displace_x; }

  auto displace_y() -> float { return frames_[current_frame_ix_].displace_y; }
};