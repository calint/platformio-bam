#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
// then other
#include "../animator.hpp"

static animator::frame constexpr ned_animation_walk[]{
    {48, 150, 2, 0},
    {49, 150, 2, 0},
    {50, 150, 2, 0},
    {51, 150, 2, 0},
};

class ned final : public game_object {
    animator animator_{};

  public:
    int8_t moving_direction = 0; // 0: still  1: right  -1: left

    ned() : game_object{cls_ned} {
        animator_.init(ned_animation_walk,
                       sizeof(ned_animation_walk) / sizeof(animator::frame),
                       false);
        spr = sprites.alloc();
        spr->obj = this;
        spr->img = animator_.sprite_img();
        spr->flip = 0;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        switch (moving_direction) {
        case 1: // right
            if (x <= display_width - sprite_width) {
                if (animator_.update()) {
                    spr->img = animator_.sprite_img();
                    spr->flip = 0;
                    x += moving_direction * animator_.displace_x();
                    y += animator_.displace_y();
                }
            } else {
                moving_direction = -1;
                animator_.reset();
                spr->img = animator_.sprite_img();
                spr->flip = 1;
            }
            break;
        case -1: // left
            if (x >= 0) {
                if (animator_.update()) {
                    spr->img = animator_.sprite_img();
                    spr->flip = 1;
                    x += moving_direction * animator_.displace_x();
                    y += animator_.displace_y();
                }
            } else {
                moving_direction = 1;
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

    auto pre_render() -> void override {
        // place sprite in background coordinate system
        spr->scr_x = int16_t(x - tile_map_x);
        spr->scr_y = int16_t(y - tile_map_y);
    }
};
