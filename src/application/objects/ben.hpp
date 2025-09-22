#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
// then other
#include "../animator.hpp"

static animator::frame constexpr ben_animation_walk[]{
    {32, 300, 2, 0},
    {33, 300, 2, 0},
    {34, 300, 2, 0},
    {35, 300, 2, 0},
};

class ben final : public game_object {
    animator animator_{};
    uint8_t moving_direction_ = 0; // 0: still  1: right  2: left

  public:
    ben() : game_object{cls_ben} {
        animator_.init(ben_animation_walk,
                       sizeof(ben_animation_walk) / sizeof(animator::frame),
                       false);
        moving_direction_ = 1;
        spr = sprites.alloc();
        spr->obj = this;
        spr->img = animator_.sprite_img();
        spr->flip = 0;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        switch (moving_direction_) {
        case 1: // right
            if (x <= display_width - sprite_width) {
                if (animator_.update()) {
                    spr->img = animator_.sprite_img();
                    spr->flip = 0;
                    x += animator_.displace_x();
                    y += animator_.displace_y();
                }
            } else {
                moving_direction_ = 2;
                animator_.reset();
                spr->img = animator_.sprite_img();
                spr->flip = 1;
            }
            break;
        case 2: // left
            if (x >= 0) {
                if (animator_.update()) {
                    spr->img = animator_.sprite_img();
                    spr->flip = 1;
                    x -= animator_.displace_x();
                    y += animator_.displace_y();
                }
            } else {
                moving_direction_ = 1;
                animator_.reset();
                spr->img = animator_.sprite_img();
                spr->flip = 0;
            }
            break;
        default: // other
            break;
        }

        return true;
    }

    // called before rendering the sprites
    auto pre_render() -> void override {
        // place sprite in background coordinate system
        spr->scr_x = int16_t(x - tile_map_x);
        spr->scr_y = int16_t(y - tile_map_y);
    }
};
