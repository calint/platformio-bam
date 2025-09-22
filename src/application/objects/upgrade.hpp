#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "upgrade_picked.hpp"
// then other

class upgrade final : public game_object {
  public:
    upgrade() : game_object{cls_upgrade} {
        col_bits = cb_upgrade;
        col_mask = cb_enemy_bullet;

        spr = sprites.alloc();
        spr->obj = this;
        spr->img = sprite_imgs[8];
        spr->layer = 1;
        spr->flip = 0;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (y >= display_height) {
            return false;
        }

        return true;
    }

    auto on_death_by_collision() -> void override {
        upgrade_picked* up = new (objects.alloc()) upgrade_picked{};
        up->x = x;
        up->y = y;
        up->dx = 50;
        up->ddx = -30;
    }
};
