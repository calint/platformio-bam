#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class upgrade_picked final : public game_object {
  clk::time death_at_ms = 0;

public:
  upgrade_picked() : game_object{upgrade_picked_cls} {
    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[9];
    spr->layer = 1;
    spr->flip = 0;

    death_at_ms = clk.ms + 5000;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    if (clk.ms >= death_at_ms) {
      return true;
    }
    return false;
  }
};
