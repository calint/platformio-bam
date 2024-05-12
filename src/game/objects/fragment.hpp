#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class fragment final : public game_object {
public:
  clk::time die_at_ms = 0;

  fragment() : game_object{fragment_cls} {
    col_bits = cb_fragment;
    col_mask = cb_none;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[2];
    spr->layer = 1;
    spr->flip = 0;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    if (clk.ms >= die_at_ms) {
      return true;
    }
    return false;
  }
};
