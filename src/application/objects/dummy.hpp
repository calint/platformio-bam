#pragma once

#include "../../engine.hpp"
#include "../game_object.hpp"

class dummy final : public game_object {
  public:
    dummy() : game_object{cls_dummy} {}

    auto update() -> bool override {
        if (!game_object::update()) {
            return false;
        }

        if (x >= display_width || x - sprite_width <= 0) {
            return false;
        }

        return true;
    }
};
