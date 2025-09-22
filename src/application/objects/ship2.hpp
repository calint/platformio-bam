#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
// then other
#include "../animator.hpp"

static animator::frame constexpr ship2_animation[]{
    {64, 500, 0, 0},
    {65, 500, 0, 0},
};

class ship2 final : public game_object {
    animator animator_{};

  public:
    ship2() : game_object{cls_ship2} {
        col_bits = cb_hero;
        col_mask = cb_enemy_bullet;

        animator_.init(ship2_animation,
                       sizeof(ship2_animation) / sizeof(animator::frame), true);

        spr = sprites.alloc();
        spr->obj = this;
        spr->img = animator_.sprite_img();
        spr->layer = 1;
        spr->flip = 0;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (dy > 0) {
            spr->flip = 2; // vertical
        }

        if (y > display_height) {
            return false;
        }

        if (animator_.update()) {
            spr->img = animator_.sprite_img();
        }

        return true;
    }

    auto on_death_by_collision() -> void override {
        upgrade* upg = new (objects.alloc()) upgrade{};
        upg->x = x;
        upg->y = y;
        upg->dy = 30;
        upg->ddy = 20;
    }
};
