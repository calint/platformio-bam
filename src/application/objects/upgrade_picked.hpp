#pragma once

#include "../../engine.hpp"
#include "../game_object.hpp"

class upgrade_picked final : public game_object {
    clk::time death_at_ms_{};

  public:
    upgrade_picked() : game_object{cls_upgrade_picked} {
        spr = sprites.alloc();
        spr->obj = this;
        spr->img = sprite_imgs[9];
        spr->layer = 2;
        spr->flip = sprite::flip_none;

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
