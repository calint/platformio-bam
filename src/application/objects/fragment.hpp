#pragma once

#include "../../engine.hpp"
#include "../game_object.hpp"

class fragment final : public game_object {
  public:
    clk::time die_at_ms{};

    fragment() : game_object{cls_fragment} {
        col_bits = cb_fragment;
        col_mask = cb_none;

        spr = sprites.alloc();
        spr->obj = this;
        spr->img = sprite_imgs[2];
        spr->layer = 2;
        spr->flip = 0;
    }

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (clk.ms >= die_at_ms) {
            return false;
        }

        return true;
    }
};
