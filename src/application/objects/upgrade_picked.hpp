#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
// then other

class upgrade_picked final : public game_object {
    clk::time death_at_ms_ = 0;

  public:
    upgrade_picked() : game_object{cls_upgrade_picked} {
        spr = sprites.alloc();
        spr->obj = this;
        spr->img = sprite_imgs[9];
        spr->layer = 1;
        spr->flip = 0;

        death_at_ms_ = clk.ms + 5000;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (clk.ms >= death_at_ms_) {
            return false;
        }

        return true;
    }
};
