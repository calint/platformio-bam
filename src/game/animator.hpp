#pragma once
// implements sprite animation
// note: maximum 127 frames in an animation

class animator final {
  public:
    struct frame final {
        constexpr frame(sprite_img_ix const _sprite_img_ix,
                        clk::time const _duration_ms, float const _displace_x,
                        float const _displace_y)
            : duration_ms{_duration_ms}, displace_x{_displace_x},
              displace_y{_displace_y}, spr_img_ix{_sprite_img_ix} {}
        // note: C++11 requires constructor

        clk::time duration_ms = 0;
        // displacement of object due to this frame
        float displace_x = 0;
        float displace_y = 0;
        // index in sprite images
        sprite_img_ix spr_img_ix = 0;
    };

  private:
    frame const* frames_ = nullptr;
    clk::time next_frame_ms_ = 0;
    int8_t frame_count_ = 0;
    int8_t current_frame_ix_ = 0;
    int8_t frame_ix_dir_ = 0; // forward: 1  backwards: -1
    bool ping_pong_ = false;  // back-and-forth animation

  public:
    auto init(frame const* frames, int8_t const frame_count,
              bool const ping_pong) -> void {
        frames_ = frames;
        frame_count_ = frame_count;
        ping_pong_ = ping_pong;
        reset();
    }

    auto reset() -> void {
        current_frame_ix_ = 0;
        frame_ix_dir_ = 1;
        next_frame_ms_ = clk.ms + frames_[0].duration_ms;
    }

    // returns true if frame has changed
    auto update() -> bool {
        if (clk.ms < next_frame_ms_) {
            return false;
        }
        current_frame_ix_ += frame_ix_dir_;
        if (current_frame_ix_ >= frame_count_) {
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
                current_frame_ix_ = frame_count_ - 1;
            }
        }
        next_frame_ms_ = clk.ms + frames_[current_frame_ix_].duration_ms;
        return true;
    }

    auto sprite_img() const -> sprite_img {
        return &sprite_imgs[frames_[current_frame_ix_].spr_img_ix][0];
    }

    auto displace_x() const -> float {
        return frames_[current_frame_ix_].displace_x;
    }

    auto displace_y() const -> float {
        return frames_[current_frame_ix_].displace_y;
    }
};