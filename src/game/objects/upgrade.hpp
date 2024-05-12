#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"
#include "upgrade_picked.hpp"

class upgrade final : public game_object {
public:
  upgrade() : game_object{upgrade_cls} {
    col_bits = cb_upgrade;
    col_mask = cb_enemy_bullet;

    spr = sprites.allocate_instance();
    spr->obj = this;
    spr->img = sprite_imgs[8];
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

  void on_death_by_collision() override {
    upgrade_picked *up = new (objects.allocate_instance()) upgrade_picked{};
    up->x = x;
    up->y = y;
    up->dx = 50;
    up->ddx = -30;
  }
};
