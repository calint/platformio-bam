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