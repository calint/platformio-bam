#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class ship1 final : public game_object {
public:
  ship1() : game_object{ship1_cls} {
    col_bits = cb_hero;
    col_mask = cb_enemy_bullet;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[5];
    spr->layer = 1;
    spr->flip = 0;
  }

  // returns true if object died
  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }

    if (y >= display_height) {
      return true;
    }

    return false;
  }
};
